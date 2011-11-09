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

#ifndef SERVICEINFOOBSERVER_H
#define SERVICEINFOOBSERVER_H

#include "amarok_export.h"

#include <QVariantMap>

/**
An abstract base class for observers that wants to be notified when here is new contex information available about an active service

    @author Nikolaj Hald Nielsen <nhnFreespirit@gmail.com> 
*/
class AMAROK_EXPORT ServiceInfoObserver{
public:
    virtual void serviceInfoChanged( QVariantMap infoMap ) = 0;
    virtual ~ServiceInfoObserver() {};
};

#endif
