/* This file is part of the KDE project
   Copyright (C) 2008 Alejandro Wainzinger <aikawarazuni@gmail.com>

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

/*

CollectionCapabilityHelper holds onto either a tracklist or a querymaker that gets
a tracklist, to pass along to appropriate slot when a particular action is triggered.

CollectionCapability returns a QList of PopupDropperAction* for e.g. creation of
a context menu.

*/

#ifndef AMAROK_COLLECTIONCAPABILITY_H
#define AMAROK_COLLECTIONCAPABILITY_H

#include "amarok_export.h"
#include "meta/Capability.h"
#include "Meta.h"

#include "context/popupdropper/libpud/PopupDropperAction.h"

#include <QList>
#include <QObject>

namespace Meta
{
    class AMAROK_EXPORT CollectionCapabilityHelper : public QObject
    {
        Q_OBJECT

        public:
            CollectionCapabilityHelper( QueryMaker *qm );

            ~CollectionCapabilityHelper();

            void setAction( PopupDropperAction *action, const QObject *receiver, const char *method );

        signals:
            void tracklistReady( Meta::TrackList tracklist);

        public slots:
            void newResultReady( QString collId, Meta::TrackList tracklist );
            void runQuery(); // runs querymaker
            void tracklistReadySlot();

        private:
            TrackList  m_tracklist;
            QueryMaker *m_querymaker;
    };

    class AMAROK_EXPORT CollectionCapability : public Meta::Capability
    {
        Q_OBJECT
        public:
            virtual ~CollectionCapability();

            static Type capabilityInterfaceType() { return Meta::Capability::Collection; }
            // if qm passed in, Helper made, action's triggered goes to helper's run query
            virtual QList<PopupDropperAction *>  collectionActions( QueryMaker *qm ) = 0;
            // if tracklist passed in, Helper made, action's triggered calls helper's triggered calls slot
            virtual QList<PopupDropperAction *>  collectionActions( const TrackList tracklist ) = 0;
    };
}

#endif
