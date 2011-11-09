/***************************************************************************
* copyright            : (C) 2007 Shane King <kde@dontletsstart.com>      *
* copyright            : (C) 2008 Leo Franchi <lfranchi@kde.org>          *
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define DEBUG_PREFIX "lastfm"

#include "ScrobblerAdapter.h"
#include "Amarok.h"
#include "amarokconfig.h"
#include "Debug.h"
#include "EngineController.h"
#include "MainWindow.h"
#include "MetaConstants.h"
#include "meta/LastFmMeta.h"

ScrobblerAdapter::ScrobblerAdapter( QObject *parent, const QString &clientId )
    : QObject( parent ),
      EngineObserver( The::engineController() ),
      m_scrobbler( new Scrobbler( clientId ) ),
      m_clientId( clientId ),
      m_lastSaved( 0 )
{
    resetVariables();

    connect( m_scrobbler, SIGNAL( status( int, QVariant ) ), this, SLOT( statusChanged( int, QVariant ) ) );
    connect( The::mainWindow(), SIGNAL( loveTrack(Meta::TrackPtr) ), SLOT( loveTrack(Meta::TrackPtr) ) );
}


ScrobblerAdapter::~ScrobblerAdapter()
{}


void
ScrobblerAdapter::engineNewTrackPlaying()
{
    DEBUG_BLOCK

    Meta::TrackPtr track = The::engineController()->currentTrack();
    if( track )
    {
        m_lastSaved = m_lastPosition; // HACK engineController is broken :(
    
        debug() << "track type:" << track->type();
        const bool isRadio = ( track->type() == "stream/lastfm" );
        
        checkScrobble();

        m_current.stamp();
        
        m_current.setTitle( track->name() );
        m_current.setDuration( track->length() );
        if( track->artist() )
            m_current.setArtist( track->artist()->name() );
        if( track->album() )
            m_current.setAlbum( track->album()->name() );

        // TODO: need to get music brainz id from somewhere
        // m_current.setMbId( );

        // TODO also set fingerprint... whatever that is :)
        // m_current.setFingerprintId( qstring );
        
        m_current.setSource( isRadio ? Track::LastFmRadio : Track::Player );
        

        if( !m_current.isNull() )
        {
            debug() << "nowPlaying: " << m_current.artist() << " - " << m_current.album() << " - " << m_current.title();
            m_scrobbler->nowPlaying( m_current );

            // When playing Last.fm Radio, we need to submit twice, once in Radio mode and once in Player mode
            // TODO check with mxcl if this is still required
            if( isRadio ) {
                m_current.setSource( Track::Player );
                m_scrobbler->nowPlaying( m_current );
            }
        }
    }
}


void 
ScrobblerAdapter::engineNewMetaData( const QHash<qint64, QString> &newMetaData, bool trackChanged )
{
    Q_UNUSED( newMetaData )
    Q_UNUSED( trackChanged )
    DEBUG_BLOCK

    // if we are listening to a stream, take the new metadata as a "new track" and, if we have enough info, save it for scrobbling
    Meta::TrackPtr track = The::engineController()->currentTrack();
    if( track &&
        ( track->type() == "stream" && ( !track->name().isEmpty() 
          && track->artist() ) ) ) // got a stream, and it has enough info to be a new track
    {
        // don't use checkScrobble as we don't need to check timestamps, it is a stream
        debug() << "scrobble: " << m_current.artist() << " - " << m_current.album() << " - " << m_current.title();
        m_current.setDuration( QDateTime::currentDateTime().toTime_t() - m_current.timestamp().toTime_t() );
        m_scrobbler->cache( m_current );
        m_scrobbler->submit();
        resetVariables();
                    
        m_current.setTitle( track->name() );
        m_current.setArtist( track->artist()->name() );
        m_current.stamp();
        
        m_current.setSource( Track::NonPersonalisedBroadcast );

        if( !m_current.isNull() )
        {
            debug() << "nowPlaying: " << m_current.artist() << " - " << m_current.album() << " - " << m_current.title();
            m_scrobbler->nowPlaying( m_current );
        }
    }
}

void
ScrobblerAdapter::enginePlaybackEnded( int finalPosition, int /*trackLength*/, PlaybackEndedReason /*reason*/ )
{
    DEBUG_BLOCK
    engineTrackPositionChanged( finalPosition, false );
    checkScrobble();
    resetVariables();
}


void
ScrobblerAdapter::engineTrackPositionChanged( long position, bool userSeek )
{
    // HACK enginecontroller is fscked. it sends engineTrackPositionChanged messages
    // with info for the last track even after engineNewTrackPlaying. this means that
    // we think we've played the whole new track even though we really haven't. so, temporary
    // workaround for 2.1.0 until i can rewrite this class properly to not need to do it
    // this way.
    //debug() << "m_lastPosition:" << m_lastPosition << "position:" << position << "m_lastSaved:" << m_lastSaved;
    if( m_lastPosition == 0 && m_lastSaved != 0 && position > m_lastSaved ) // this is probably when the fucked up info came through, ignore
        return;
    m_lastSaved = 0;
    
    // note: in the 1.2 protocol, it's OK to submit if the user seeks
    // so long as they meet the half file played requirement.
    //debug() << "userSeek" << userSeek << "position:" << position << "m_lastPosition" << m_lastPosition << "m_totalPlayed" << m_totalPlayed;
    if( !userSeek && position > m_lastPosition )
        m_totalPlayed += position - m_lastPosition;
    m_lastPosition = position;
    //debug() << "userSeek" << userSeek << "position:" << position << "m_lastPosition" << m_lastPosition << "m_totalPlayed" << m_totalPlayed;
}


void
ScrobblerAdapter::skip()
{
    DEBUG_BLOCK

    // NOTE doesn't exist in 1.2.1 lib... find replacement
    //m_current.setRatingFlag( Track::Skipped );
}


void
ScrobblerAdapter::love()
{
    DEBUG_BLOCK

    m_current.love();
    
}

void
ScrobblerAdapter::loveTrack( Meta::TrackPtr track )
{
    DEBUG_BLOCK

    if( track )
    {
        MutableTrack trackInfo;
        trackInfo.setTitle( track->name() );
        if( track->artist() )
            trackInfo.setArtist( track->artist()->name() );
        if( track->album() )
            trackInfo.setAlbum( track->album()->name() );

        trackInfo.love();
    }
}


void
ScrobblerAdapter::ban()
{
    DEBUG_BLOCK

    m_current.ban();
}


void
ScrobblerAdapter::statusChanged( int statusCode, QVariant /*data*/ )
{
    debug() << "statusChanged: statusCode=" << statusCode;
}


void
ScrobblerAdapter::resetVariables()
{
    m_current = MutableTrack();
    m_totalPlayed = m_lastPosition = 0;
}


void
ScrobblerAdapter::checkScrobble()
{
    DEBUG_BLOCK
    // note: in the 1.2 protocol submits are always done at end of file
    debug() << "total played" << m_totalPlayed << "duration" << m_current.duration() * 1000 / 2 << "isNull" << m_current.isNull() << "submit?" << AmarokConfig::submitPlayedSongs();
    if( ( m_totalPlayed >= m_current.duration() * 1000 / 2 ) && !m_current.isNull() && AmarokConfig::submitPlayedSongs() )
    {
        debug() << "scrobble: " << m_current.artist() << " - " << m_current.album() << " - " << m_current.title();
        m_scrobbler->cache( m_current );
        m_scrobbler->submit();
    }
    resetVariables();
}
