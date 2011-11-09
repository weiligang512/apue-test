/***************************************************************************
 *   Copyright (c) 2009  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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


#ifndef AMAROK_MULTISOURCECAPABILITYIMPL_P_H
#define AMAROK_MULTISOURCECAPABILITYIMPL_P_H

#include "meta/capabilities/MultiSourceCapability.h"

#include "Debug.h"
#include "MultiTrack.h"


class MultiSourceCapabilityImpl : public Meta::MultiSourceCapability
{
    Q_OBJECT
public:
    MultiSourceCapabilityImpl( Meta::MultiTrack * track );


    virtual KUrl first() { return m_track->first(); }
    virtual KUrl next() { return m_track->next(); }
    virtual int current() { return m_track->current(); }
    virtual QStringList sources() { return m_track->sources(); }
    virtual void setSource( int source );

private:
    Meta::MultiTrack * m_track;

};

#endif


