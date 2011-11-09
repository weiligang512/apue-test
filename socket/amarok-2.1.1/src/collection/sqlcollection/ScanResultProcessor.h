/*
 *  Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>
 *  Copyright (c) 2008 Seb Ruiz <ruiz@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AMAROK_SQL_SCANRESULTPROCESSOR_H
#define AMAROK_SQL_SCANRESULTPROCESSOR_H

#include "CollectionManager.h"

#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVariant>


class SqlCollection;

class ScanResultProcessor : public QObject
{
    Q_OBJECT

    public:
        enum ScanType
        {
            FullScan = 0,
            IncrementalScan = 1
        };

        ScanResultProcessor( SqlCollection *collection );
        ~ScanResultProcessor();

        void addDirectory( const QString &dir, uint mtime );
        void addImage( const QString &path, const QList< QPair<QString, QString> > );
        void setScanType( ScanType type );
        void processDirectory( const QList<QVariantMap > &data );
        void commit();
        void rollback();

    signals:
        void changedTrackUrls( TrackUrls );

    private:
        void addTrack( const QVariantMap &trackData, int albumArtistId );

        int artistId( const QString &artist );
        int genreId( const QString &genre );
        int composerId( const QString &composer );
        int yearId( const QString &year );
        int imageId( const QString &image, int albumId );
        int albumId( const QString &album, int artistId );
        int urlId( const QString &url, const QString &uid );
        int directoryId( const QString &dir );

        void updateAftPermanentTablesUrlId( int urlId, const QString &uid );
        void updateAftPermanentTablesUidId( int urlId, const QString &uid );
        void updateAftPermanentTablesUrlString( const QString &url, const QString &uid );
        void updateAftPermanentTablesUidString( const QString &url, const QString &uid );

        int checkExistingAlbums( const QString &album );

        QString findAlbumArtist( const QSet<QString> &artists, int trackCount ) const;
        void setupDatabase();

    private:
        SqlCollection *m_collection;
        bool m_setupComplete;

        QMap<QString, int> m_artists;
        QMap<QString, int> m_genre;
        QMap<QString, int> m_year;
        QMap<QString, int> m_composer;
        QMap<QPair<QString, int>, int> m_albums;
        QMap<QPair<QString, int>, int> m_images;
        QMap<QString, int> m_directories;

        QHash<QString, uint> m_filesInDirs;

        TrackUrls m_changedUrls;

        ScanType m_type;

        QStringList m_aftPermanentTablesUrlString;
};

#endif
