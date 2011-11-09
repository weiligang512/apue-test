/* This file is part of the KDE project
   Copyright (C) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>

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

#ifndef SQLREGISTRY_H
#define SQLREGISTRY_H

#include "SqlMeta.h"

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QPair>
#include <QTimer>
#include <QList>

class SqlCollection;

typedef QPair<int, QString> TrackId;

class SqlRegistry : public QObject
{
    Q_OBJECT

    public:
        SqlRegistry(SqlCollection *collection);
        virtual ~SqlRegistry();

        Meta::TrackPtr getTrack( const QString &url );
        Meta::TrackPtr getTrack( const QStringList &rowData );
        Meta::TrackPtr getTrackFromUid( const QString &uid );
        bool checkUidExists( const QString &uid );

        Meta::ArtistPtr getArtist( const QString &name, int id = -1 );
        Meta::GenrePtr getGenre( const QString &name, int id = -1 );
        Meta::ComposerPtr getComposer( const QString &name, int id = -1 );
        Meta::YearPtr getYear( const QString &year, int id = -1 );
        Meta::AlbumPtr getAlbum( const QString &album, int id = -1, int artist = -1 ); //TODO fix this


    private slots:
        void emptyCache();

    private:

        //we don't care about the ordering so use the faster QHash
        QHash<TrackId, Meta::TrackPtr > m_trackMap;
        QHash<QString, Meta::TrackPtr > m_uidMap;
        QHash<int, Meta::ArtistPtr > m_artistMap;
        QHash<int, Meta::ComposerPtr > m_composerMap;
        QHash<int, Meta::GenrePtr > m_genreMap;
        QHash<int, Meta::YearPtr > m_yearMap;
        QHash<int, Meta::AlbumPtr > m_albumMap;        //TODO: needs improvement

        QMutex m_trackMutex;
        QMutex m_artistMutex;
        QMutex m_composerMutex;
        QMutex m_genreMutex;
        QMutex m_yearMutex;
        QMutex m_albumMutex;
        QMutex m_uidMutex;

        QTimer *m_timer;

        SqlCollection *m_collection;
};

#endif /* SQLREGISTRY_H */
