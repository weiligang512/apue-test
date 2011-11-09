/***************************************************************************
 * copyright            : (C) 2008 Daniel Jones <danielcjones@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "DynamicTrackNavigator.h"

#include "Debug.h"
#include "DynamicModel.h"
#include "DynamicPlaylist.h"
#include "Meta.h"
#include "playlist/PlaylistModel.h"
#include "playlist/PlaylistController.h"

#include <QMutexLocker>


Playlist::DynamicTrackNavigator::DynamicTrackNavigator( Dynamic::DynamicPlaylistPtr p )
    : m_playlist( p )
{
    connect( m_playlist.data(), SIGNAL( tracksReady( Meta::TrackList ) ), SLOT( receiveTracks( Meta::TrackList ) ) );
    connect( Model::instance(), SIGNAL( activeTrackChanged( quint64 ) ), SLOT( trackChanged() ) );
    connect( Model::instance(), SIGNAL( modelReset() ), SLOT( repopulate() ) );
    connect( PlaylistBrowserNS::DynamicModel::instance(), SIGNAL( activeChanged() ), SLOT( activePlaylistChanged() ) );
}

Playlist::DynamicTrackNavigator::~DynamicTrackNavigator()
{
    m_playlist->requestAbort();
}

void
Playlist::DynamicTrackNavigator::receiveTracks( Meta::TrackList tracks )
{
    DEBUG_BLOCK

    Controller::instance()->insertOptioned( tracks, Append );
}

void
Playlist::DynamicTrackNavigator::appendUpcoming()
{
    DEBUG_BLOCK

    int updateRow = Model::instance()->activeRow() + 1;
    int rowCount = Model::instance()->rowCount();
    int upcomingCountLag = m_playlist->upcomingCount() - ( rowCount - updateRow );

    if ( upcomingCountLag > 0 )
        m_playlist->requestTracks( upcomingCountLag );
}

void
Playlist::DynamicTrackNavigator::removePlayed()
{
    int activeRow = Model::instance()->activeRow();
    if ( activeRow > m_playlist->previousCount() )
    {
        Controller::instance()->removeRows( 0, activeRow - m_playlist->previousCount() );
    }
}

void
Playlist::DynamicTrackNavigator::activePlaylistChanged()
{
    DEBUG_BLOCK

    Dynamic::DynamicPlaylistPtr newPlaylist =
        PlaylistBrowserNS::DynamicModel::instance()->activePlaylist();

    if ( newPlaylist == m_playlist )
        return;

    m_playlist->requestAbort();
    QMutexLocker locker( &m_mutex );

    m_playlist = newPlaylist;

    connect( m_playlist.data(), SIGNAL( tracksReady( Meta::TrackList ) ), SLOT( receiveTracks( Meta::TrackList ) ) );
}

void
Playlist::DynamicTrackNavigator::trackChanged()
{
    appendUpcoming();
    removePlayed();
}

void
Playlist::DynamicTrackNavigator::repopulate()
{
    if ( !m_mutex.tryLock() )
        return;

    int row = Model::instance()->activeRow() + 1;
    if ( row < 0 )
        row = 0;

    // Don't remove queued tracks
    QList<int> rows;

    do {
        if( !(Model::instance()->stateOfRow( row ) & Item::Queued) )
            rows << row;
        row++;
    }
    while( row < Model::instance()->rowCount() );

    if( !rows.isEmpty() )
        Controller::instance()->removeRows( rows );

    m_playlist->recalculate();
    appendUpcoming();

    m_mutex.unlock();
}
