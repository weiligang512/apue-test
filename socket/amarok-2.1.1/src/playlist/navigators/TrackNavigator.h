/***************************************************************************
 * copyright            : (C) 2007 Ian Monroe <ian@monroe.nu>
 *                      : (C) 2008 Soren Harward <stharward@gmail.com>
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
#ifndef TRACKNAVIGATOR_H
#define TRACKNAVIGATOR_H

#include <QObject>
#include <QQueue>

namespace Playlist
{

    /** 
     * An abstract class which defines what should be done after a track
     * finishes playing.  The Playlist::Model will have an object of the
     * currently active strategy.  It is the "strategy" pattern from the Design
     * Patterns book. In Amarok 1.x, the Playlist became very confusing due to
     * random mode and dynamic playlists requiring complicated nested if
     * statements. This should prevent that.
     */

    typedef QList<quint64> ItemList; // A convenient typedef!

    class TrackNavigator : public QObject
    {
        Q_OBJECT

        public:
            TrackNavigator();
            virtual ~TrackNavigator() { }

            /**
             * The engine will finish the current track in a couple of seconds,
             * and would like to know what the next track should be.
             */
            virtual quint64 requestNextTrack() = 0;

            /**
             * The user triggers the next-track action.
             */
            virtual quint64 requestUserNextTrack() = 0;

            /**
             * The user triggers the previous-track action.
             */
            virtual quint64 requestLastTrack() = 0;

            /**
             * Find the position of the id in the queue
             * @return the position, or -1 if non in queue
             */
            int queuePosition( const quint64 id ) const;

        public slots:
            /**
             * Queues the specified id and schedules it to be played.
             */
            virtual bool queueIds( const QList<quint64> &ids );
            virtual bool queueId( const quint64 id );

            /**
             * Dequeue the specified id from the queue list
             */
            virtual bool dequeueIds( const QList<quint64> &ids );
            virtual bool dequeueId( const quint64 id );

        protected:
            // repeat the entire playlist when we've reached the end
            bool m_repeatPlaylist;

            // Holds the list of tracks to be played next. General 
            // workflow should dictate that the TrackAdvancer should
            // respect the queue list as an override to what the Advancer
            // implementation would normally return as the next track.
            // TODO: a history queue to allow requestLastTrack() to work 
            // properly?
            // Static queue so that all navigators share the same queue
            QQueue<quint64> m_queue;
    };
}

#endif
