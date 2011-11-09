/**************************************************************************
*   Copyright (c) 2009  Casey Link <unnamedrambler@gmail.com>             *
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

#ifndef TIMECODEWRITECAPABILITY_H
#define TIMECODEWRITECAPABILITY_H

#include "amarok_export.h"
#include "meta/Meta.h"
#include "meta/Capability.h"

namespace Meta
{
/**
 * This capability determines whether a track can have a timecode
 * written to it.
 * @author Casey Link
 */
class AMAROK_EXPORT TimecodeWriteCapability : public Capability
{
    Q_OBJECT
public:

    virtual ~TimecodeWriteCapability();

    /**
     * Stores a timecode for the track
     * @param seconds the position in seconds at which the timecide should be stored.
     * @return  true if the write was successful, false if not.
     */
    virtual bool writeTimecode ( int seconds ) = 0;

    /**
     * Stores an auto timecode for the track and deletes any previously added auto timecodes
     * @param seconds the position in seconds at which the timecide should be stored.
     * @return  true if the write was successful, false if not.
     */
    virtual bool writeAutoTimecode ( int seconds ) = 0;

    /**
    * Get the capabilityInterfaceType of this capability
    * @return The capabilityInterfaceType ( always Meta::Capability::WriteTimecode; )
    */
    static Type capabilityInterfaceType()
    {
        return Meta::Capability::WriteTimecode;
    }

protected:

    bool writeTimecode( int seconds, Meta::TrackPtr track );
    bool writeAutoTimecode( int seconds, Meta::TrackPtr track );
};

#endif // TIMECODEWRITECAPABILITY_H
}
