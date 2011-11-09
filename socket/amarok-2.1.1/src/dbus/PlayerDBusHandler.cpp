/******************************************************************************
 * Copyright (C) 2008 Ian Monroe <ian@monroe.nu>                              *
 *           (C) 2008 Peter ZHOU <peterzhoulei@gmail.com>                     *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#include "PlayerDBusHandler.h"

#include "amarokconfig.h"
#include "App.h"
#include "Debug.h"
#include "EngineController.h"
#include "meta/Meta.h"
#include "PlayerAdaptor.h"
#include "playlist/PlaylistActions.h"
#include "playlist/PlaylistModel.h"
#include "ActionClasses.h"

// Marshall the DBusStatus data into a D-BUS argument
QDBusArgument &operator<<(QDBusArgument &argument, const DBusStatus &status)
{
    argument.beginStructure();
    argument << status.Play;
    argument << status.Random;
    argument << status.Repeat;
    argument << status.RepeatPlaylist;
    argument.endStructure();
    return argument;
}

// Retrieve the DBusStatus data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusStatus &status)
{
    argument.beginStructure();
    argument >> status.Play;
    argument >> status.Random;
    argument >> status.Repeat;
    argument >> status.RepeatPlaylist;
    argument.endStructure();
    return argument;
}


namespace Amarok
{

    PlayerDBusHandler *PlayerDBusHandler::s_instance = 0;

    PlayerDBusHandler::PlayerDBusHandler()
        : QObject(kapp)
    {
        qDBusRegisterMetaType<DBusStatus>();

        s_instance = this;
        setObjectName("PlayerDBusHandler");

        new PlayerAdaptor( this );
        QDBusConnection::sessionBus().registerObject("/Player", this);

        connect( The::engineController(), SIGNAL( trackChanged( Meta::TrackPtr ) ), this, SLOT( slotTrackChange() ) );
        connect( The::engineController(), SIGNAL( trackChanged( Meta::TrackPtr ) ), this, SLOT( slotStatusChange() ) );
        connect( The::engineController(), SIGNAL( trackFinished() ), this, SLOT( slotStatusChange() ) );
        connect( The::engineController(), SIGNAL( trackPlayPause( int ) ), this, SLOT( slotStatusChange() ) );
        connect( this, SIGNAL( StatusChange( DBusStatus ) ), this, SLOT( slotCapsChange() ) );

        SelectAction* repeatAction = qobject_cast<SelectAction*>( Amarok::actionCollection()->action( "repeat" ) );
        Q_ASSERT( repeatAction );
        connect( repeatAction, SIGNAL( triggered( int ) ), this, SLOT( slotStatusChange() ) );
    }

    DBusStatus PlayerDBusHandler::GetStatus()
    {
        DBusStatus status;
        switch( The::engineController()->state() )
        {
            case Phonon::PlayingState:
            case Phonon::BufferingState:
                status.Play = 0; //Playing
                break;
            case Phonon::PausedState:
                status.Play = 1; //Paused
                break;
            case Phonon::LoadingState:
            case Phonon::StoppedState:
            case Phonon::ErrorState:
                status.Play = 2; //Stopped
        };
        if ( AmarokConfig::randomMode() )
            status.Random = 1;
        else
            status.Random = 0;
        if ( Amarok::repeatTrack() )
            status.Repeat = 1;
        else
            status.Repeat = 0;
        if ( Amarok::repeatPlaylist() || Amarok::repeatAlbum() || AmarokConfig::randomMode() )
            status.RepeatPlaylist = 1;
        else
            status.RepeatPlaylist = 0; //the music will not end if we play random
        return status;
    }

    void PlayerDBusHandler::Pause()
    {
        The::engineController()->playPause();
    }

    void PlayerDBusHandler::Play()
    {
        The::engineController()->play();
    }

    void PlayerDBusHandler::Next()
    {
        The::playlistActions()->next();
    }

    void PlayerDBusHandler::Prev()
    {
        The::playlistActions()->back();
    }

    void PlayerDBusHandler::Repeat( bool on )
    {
        debug() << (on ? "Turning repeat on" : "Turning repeat off");
        if ( on == Amarok::repeatTrack() ) {
            // Don't turn off repeatAlbum or repeatPlaylist because
            // we were asked to turn off repeatTrack
            return;
        }

        SelectAction* repeatAction = qobject_cast<SelectAction*>( Amarok::actionCollection()->action( "repeat" ) );
        Q_ASSERT(repeatAction);

        if (repeatAction)
            repeatAction->setCurrentItem( on ? int(AmarokConfig::EnumRepeat::Track)
                                             : int(AmarokConfig::EnumRepeat::Off) );
    }

    //position is specified in milliseconds
    int PlayerDBusHandler::PositionGet()
    {
        return The::engineController()->trackPosition() * 1000;
    }

    void PlayerDBusHandler::PositionSet( int time )
    {
        if ( time > 0 && The::engineController()->state() != Phonon::StoppedState )
            The::engineController()->seek( time );
    }

    void PlayerDBusHandler::Stop()
    {
        The::engineController()->stop();
    }

    int PlayerDBusHandler::VolumeGet()
    {
        return The::engineController()->volume();
    }

    void PlayerDBusHandler::VolumeSet( int vol )
    {
        The::engineController()->setVolume(vol);
    }

    void PlayerDBusHandler::VolumeUp( int step ) const
    {
        The::engineController()->increaseVolume( step );
    }

    void PlayerDBusHandler::VolumeDown( int step ) const
    {
        The::engineController()->decreaseVolume( step );
    }

    void PlayerDBusHandler::Mute() const
    {
        The::engineController()->toggleMute();
    }

    QVariantMap PlayerDBusHandler::GetMetadata()
    {
        return GetTrackMetadata( The::engineController()->currentTrack() );
    }

    int PlayerDBusHandler::GetCaps()
    {
        int caps = NONE;
        Meta::TrackPtr track = The::engineController()->currentTrack();
        caps |= CAN_HAS_TRACKLIST;
        if ( track ) caps |= CAN_PROVIDE_METADATA;
        if ( GetStatus().Play == 0 /*playing*/ ) caps |= CAN_PAUSE;
        if ( ( GetStatus().Play == 1 /*paused*/ ) || ( GetStatus().Play == 2 /*stoped*/ ) ) caps |= CAN_PLAY;
        if ( ( GetStatus().Play == 0 /*playing*/ ) || ( GetStatus().Play == 1 /*paused*/ ) ) caps |= CAN_SEEK;
        if ( ( The::playlistModel()->activeRow() >= 0 ) && ( The::playlistModel()->activeRow() <= The::playlistModel()->rowCount() ) )
        {
            caps |= CAN_GO_NEXT;
            caps |= CAN_GO_PREV;
        }
        return caps;
    }

    void PlayerDBusHandler::slotCapsChange()
    {
        emit CapsChange( GetCaps() );
    }

    void PlayerDBusHandler::slotTrackChange()
    {
        emit TrackChange( GetMetadata() );
    }

    void PlayerDBusHandler::slotStatusChange()
    {
        DBusStatus status = GetStatus();
        emit StatusChange( status );
    }

    QVariantMap PlayerDBusHandler::GetTrackMetadata( Meta::TrackPtr track )
    {
        QVariantMap map;
        if( track )
        {
            // MANDATORY:
            map["location"] = track->playableUrl().url();

            // INFORMATIONAL:
            map["title"] = track->prettyName();
            
            if( track->artist() )
                map["artist"] = track->artist()->name();
            
            if( track->album() )
                map["album"] = track->album()->name();
            
            map["tracknumber"] = track->trackNumber();
            map["time"] = track->length();
            map["mtime"] = track->length() * 1000;
            
            if( track->genre() )
                map["genre"] = track->genre()->name();
            
            map["comment"] = track->comment();
            map["rating"] = track->rating()/2;  //out of 5, not 10.
            
            if( track->year() )
                map["year"] = track->year()->name();

            if( track->album() )
                map["arturl"] = track->album()->imageLocation().url();

            //TODO: external service meta info

            // TECHNICAL:
            map["audio-bitrate"] = track->bitrate();
            map["audio-samplerate"] = track->sampleRate();
            //amarok has no video-bitrate
        }
        return map;
    }
} // namespace Amarok

namespace The {
    Amarok::PlayerDBusHandler* playerDBusHandler() { return Amarok::PlayerDBusHandler::s_instance; }
}

#include "PlayerDBusHandler.moc"
