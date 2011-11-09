/***************************************************************************
 *   Copyright (c) 2008  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *   Copyright (c) 2008  Bart Cerneels <bart.cerneels@kde.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef USERPLAYLISTPROVIDER_H
#define USERPLAYLISTPROVIDER_H

#include "meta/Playlist.h"
#include "playlistmanager/PlaylistManager.h"

class PopupDropperAction;

/**
    @author Bart Cerneels <bart.cerneels@kde.org>
*/
class AMAROK_EXPORT UserPlaylistProvider : public PlaylistProvider
{
    public:
        virtual ~UserPlaylistProvider() {};

        /* PlaylistProvider functions */
        virtual int category() const { return PlaylistManager::UserPlaylist; };
        virtual Meta::PlaylistList playlists() = 0;

        /* UserPlaylistProvider functions */
        /**
            @returns true if this provider supports - and is currently able to - save playlists
        **/
        virtual bool canSavePlaylists() = 0;

        /**
            Save a list of tracks as a playlist in the database.
            @returns a non-null Meta::PlaylistPtr if successful
        **/
        virtual Meta::PlaylistPtr save( const Meta::TrackList &tracks ) = 0;
        
        virtual Meta::PlaylistPtr save( const Meta::TrackList &tracks, const QString& name ) = 0;

        virtual bool supportsEmptyGroups() { return false; }

        virtual QList<PopupDropperAction *> playlistActions( Meta::PlaylistList list ) = 0;
};

#endif
