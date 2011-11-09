/***************************************************************************
 *   Copyright (c) 2008  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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

#ifndef MAGNATUNEDATABASEWORKER_H
#define MAGNATUNEDATABASEWORKER_H

#include "MagnatuneMeta.h"

#include "../ServiceSqlRegistry.h"

#include <threadweaver/Job.h>

/**
A small helper class to do some simple asynchroneous database queries

	@author Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>   
*/

class MagnatuneDatabaseWorker : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    MagnatuneDatabaseWorker();

    ~MagnatuneDatabaseWorker();

    void run();

    void fetchMoodMap();
    void fetchTrackswithMood( const QString &mood, int noOfTracks, ServiceSqlRegistry * registry );

signals:

    void gotMoodMap( QMap<QString, int> map );
    void gotMoodyTracks( Meta::TrackList tracks );

private slots:
    void completeJob();

private:

    void doFetchMoodMap();
    void doFetchTrackswithMood();
    
    enum taskType { FETCH_MODS, FETCH_MOODY_TRACKS };

    int m_task;

    QMap<QString, int> m_moodMap;
    Meta::TrackList m_moodyTracks;

    QString m_mood;
    int m_noOfTracks;

    ServiceSqlRegistry * m_registry;
    

};

#endif
