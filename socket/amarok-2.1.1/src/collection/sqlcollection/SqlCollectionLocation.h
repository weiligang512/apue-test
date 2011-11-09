/*
 *  Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>
 *  Copyright (c) 2008 Jason A. Donenfeld <Jason@zx2c4.com>
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

#ifndef AMAROK_SQLCOLLECTIONLOCATION_H
#define AMAROK_SQLCOLLECTIONLOCATION_H

#include "CollectionLocation.h"

#include <QSet>
#include <QMap>
#include <QString>

class SqlCollection;
class KJob;

class SqlCollectionLocation : public CollectionLocation
{
    Q_OBJECT
    public:
        SqlCollectionLocation( SqlCollection const *collection );
        virtual ~SqlCollectionLocation();

        virtual QString prettyLocation() const;
        virtual QStringList actualLocation() const;
        virtual bool isWritable() const;
        virtual bool isOrganizable() const;
        virtual bool remove( const Meta::TrackPtr &track );
        virtual void insertTracks( const QMap<Meta::TrackPtr, QString> &trackMap );
        virtual void insertStatistics( const QMap<Meta::TrackPtr, QString> &trackMap );

    protected:
        virtual void showDestinationDialog( const Meta::TrackList &tracks, bool removeSources );
        virtual void copyUrlsToCollection( const QMap<Meta::TrackPtr, KUrl> &sources );

    private slots:
        void slotDialogAccepted();
        void slotDialogRejected();
        void slotJobFinished( KJob *job );

    private:
        QMap<QString, uint> updatedMtime( const QStringList &urls );

        SqlCollection *m_collection;
        QMap<Meta::TrackPtr, QString> m_destinations;
        bool m_overwriteFiles;
        QMap<KJob*, Meta::TrackPtr> m_jobs;
};

#endif
