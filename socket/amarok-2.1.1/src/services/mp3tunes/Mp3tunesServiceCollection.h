/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *   Copyright (c) 2008  Casey Link <unnamedrambler@gmail.com>             *
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
#ifndef MP3TUNESSERVICECOLLECTION_H
#define MP3TUNESSERVICECOLLECTION_H

#include <ServiceCollectionLocation.h>
#include "Mp3tunesLockerMeta.h"
#include "Mp3tunesMeta.h"
#include "Mp3tunesLocker.h"

#include <QMap>

class Mp3tunesServiceCollection : public ServiceCollection
{
    Q_OBJECT
    
public:
    Mp3tunesServiceCollection( ServiceBase * service, const QString &sessionId,
            Mp3tunesLocker * locker );

    virtual ~Mp3tunesServiceCollection();

    virtual QueryMaker* queryMaker();

    virtual QString collectionId() const;
    virtual QString prettyName() const;
    virtual CollectionLocation* location() const;
    Mp3tunesLocker* locker() const;

    virtual Meta::TrackPtr trackForUrl( const KUrl &url );
    virtual bool possiblyContainsTrack( const KUrl &url ) const;

private slots:
    /**
     * Handles trackForUrl complete.
     */
    void trackForUrlComplete( Mp3tunesLockerTrack &track );

private:
    QString m_sessionId;
    Mp3tunesLocker * m_locker;
    QMap<QString, Meta::Mp3TunesTrack*> m_tracksFetching; // a list of tracks that are being fetched via trackForUrl and their associated threadweaver jobs
};

#endif
