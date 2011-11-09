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
 
#include "MagnatuneDatabaseWorker.h"

#include "collection/CollectionManager.h"
#include "collection/SqlStorage.h"

MagnatuneDatabaseWorker::MagnatuneDatabaseWorker()
    : ThreadWeaver::Job()
    , m_registry( 0 )
{
    connect( this, SIGNAL( done( ThreadWeaver::Job* ) ), SLOT( completeJob() ) );
}


MagnatuneDatabaseWorker::~MagnatuneDatabaseWorker()
{
}


void
MagnatuneDatabaseWorker::run()
{
    switch ( m_task ) {
        case FETCH_MODS:
            doFetchMoodMap();
            break;
        case FETCH_MOODY_TRACKS:
            doFetchTrackswithMood();
            break;
        default:
            break;
    }
}

void MagnatuneDatabaseWorker::completeJob()
{
    switch ( m_task ) {
        case FETCH_MODS:
            emit( gotMoodMap( m_moodMap ) );
            break;
        case FETCH_MOODY_TRACKS:
            emit( gotMoodyTracks( m_moodyTracks ) );
            break;
        default:
            break;
    }
    deleteLater();
}




void MagnatuneDatabaseWorker::fetchMoodMap()
{
    m_task = FETCH_MODS;
    m_moodMap.clear();
}

void MagnatuneDatabaseWorker::fetchTrackswithMood( const QString &mood, int noOfTracks, ServiceSqlRegistry * registry )
{
    m_task = FETCH_MOODY_TRACKS;
    m_mood = mood;
    m_noOfTracks = noOfTracks;

    m_registry = registry;

    m_moodyTracks.clear();
}


void MagnatuneDatabaseWorker::doFetchMoodMap()
{
    SqlStorage *sqlDb = CollectionManager::instance()->sqlStorage();
    QString queryString = "select count( mood ), mood from magnatune_moods GROUP BY mood;";
    debug() << "Querying for moods: " << queryString;
    QStringList result = sqlDb->query( queryString );
    debug() << "result: " << result;

    while ( !result.isEmpty() ) {
        int count = result.takeFirst().toInt();
        QString string =  result.takeFirst();
        m_moodMap.insert( string, count );
    }

}

void MagnatuneDatabaseWorker::doFetchTrackswithMood()
{
    SqlStorage *sqlDb = CollectionManager::instance()->sqlStorage();



    //ok, a huge joing turned out to be _really_ slow, so lets chop up the query a bit...

    QString queryString = "SELECT DISTINCT track_id FROM magnatune_moods WHERE mood =\"" + m_mood + "\"  ORDER BY RANDOM() LIMIT " + QString::number( m_noOfTracks, 10 ) + ';';

    QStringList result = sqlDb->query( queryString );

    int rowCount = ( m_registry->factory()->getTrackSqlRowCount() +
            m_registry->factory()->getAlbumSqlRowCount() +
            m_registry->factory()->getArtistSqlRowCount() +
            m_registry->factory()->getGenreSqlRowCount() );

    foreach( const QString &idString, result ) {

        QString queryString = "SELECT DISTINCT ";
        
                
        queryString += m_registry->factory()->getTrackSqlRows() + ',' +
                    m_registry->factory()->getAlbumSqlRows() + ',' +
                    m_registry->factory()->getArtistSqlRows() + ',' +
                    m_registry->factory()->getGenreSqlRows();

        queryString += " FROM magnatune_tracks LEFT JOIN magnatune_albums ON magnatune_tracks.album_id = magnatune_albums.id LEFT JOIN magnatune_artists ON magnatune_albums.artist_id = magnatune_artists.id LEFT JOIN magnatune_genre ON magnatune_genre.album_id = magnatune_albums.id";

        queryString += " WHERE magnatune_tracks.id = " + idString;
        queryString += " GROUP BY  magnatune_tracks.id";

        //debug() << "Querying for moody tracks: " << queryString;

        QStringList result = sqlDb->query( queryString );
        //debug() << "result: " << result;



        int resultRows = result.count() / rowCount;

        for( int i = 0; i < resultRows; i++ )
        {
            QStringList row = result.mid( i*rowCount, rowCount );

            Meta::TrackPtr trackptr =  m_registry->getTrack( row );

            m_moodyTracks.append( trackptr );
        }
    }

}

#include "MagnatuneDatabaseWorker.moc"

