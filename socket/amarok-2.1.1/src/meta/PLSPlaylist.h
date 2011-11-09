/* This file is part of the KDE project
   Copyright (C) 2007 Bart Cerneels <bart.cerneels@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef METAPLSPLAYLIST_H
#define METAPLSPLAYLIST_H

#include <Playlist.h>

class QTextStream;
class QFile;

namespace Meta {

class PLSPlaylist;

typedef KSharedPtr<PLSPlaylist> PLSPlaylistPtr;
typedef QList<PLSPlaylistPtr> PLSPlaylistList;

/**
	@author Bart Cerneels <bart.cerneels@kde.org>
*/
class PLSPlaylist : public Playlist
{
    public:
        PLSPlaylist();
        PLSPlaylist( TrackList tracks );
        PLSPlaylist( const KUrl &url );

        ~PLSPlaylist();

        bool save( const QString &location, bool relative );

        /* Playlist virtual functions */
        virtual QString name() const { return prettyName(); };
        virtual QString prettyName() const { return m_url.fileName(); };

        /** returns all tracks in this playlist */
        TrackList tracks() { return m_tracks; };

        bool hasCapabilityInterface( Meta::Capability::Type type ) const { Q_UNUSED( type ); return false; };

        Capability* createCapabilityInterface( Capability::Type type ) { Q_UNUSED( type ); return 0; };

        KUrl retrievableUrl() { return m_url; };

        bool load( QTextStream &stream ) { return loadPls( stream ); };

    private:
        bool loadPls( QTextStream &stream );
        unsigned int loadPls_extractIndex( const QString &str ) const;

        Meta::TrackList m_tracks;
        KUrl m_url;

};

}

Q_DECLARE_METATYPE( Meta::PLSPlaylistPtr )
Q_DECLARE_METATYPE( Meta::PLSPlaylistList )

#endif
