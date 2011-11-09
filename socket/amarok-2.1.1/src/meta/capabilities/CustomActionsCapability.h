/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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

#ifndef AMAROK_CUSTOMACTIONSCAPABILITY_H
#define AMAROK_CUSTOMACTIONSCAPABILITY_H

#include "amarok_export.h"
#include "meta/Capability.h"

#include "context/popupdropper/libpud/PopupDropperAction.h"
#include <QList>

namespace Meta
{
    /**
     * This capability allows different meta types to display custom actions in the right click menu in the tree view
     * or anywhere else where the actions are shown. This is useful for purchasing from stores, downloading from services
     * banning a genre or whatever we can think of in the future
     *
     *         @author Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>
     */

    class AMAROK_EXPORT CustomActionsCapability : public Meta::Capability
    {
        Q_OBJECT
        public:
            
            /**
             * Constructor
             */
            CustomActionsCapability();
            
            /**
             * Constructor
             * @param actions A list of actions to use.
             */
            CustomActionsCapability( const QList< PopupDropperAction* > &actions );
            /**
             * Destructor
             */
            virtual ~CustomActionsCapability();

            /**
             * Get the custom actions for this capablility
             * @return The list of custom actions
             */
            virtual QList<PopupDropperAction *> customActions() const;

            /**
             * Get the capabilityInterfaceType of this capability
             * @return The capabilityInterfaceType ( always Meta::Capability::CustomActions; )
             */
            static Type capabilityInterfaceType() { return Meta::Capability::CustomActions; }

        protected:
            QList< PopupDropperAction* > m_actions;
    };
}

#endif
