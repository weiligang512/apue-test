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
 
#ifndef SERVICEBOOKMARKTHISCAPABILITY_H
#define SERVICEBOOKMARKTHISCAPABILITY_H

#include "amarok_export.h"
#include "meta/capabilities/BookmarkThisCapability.h"

class BookmarkThisProvider;

/**
A service specific implementation of the BookmarkThisCapability

	@author Nikolaj Hald Nielsen <nhnFreespirit@gmail.com> 
*/
class AMAROK_EXPORT ServiceBookmarkThisCapability : public Meta::BookmarkThisCapability {
public:
    ServiceBookmarkThisCapability( BookmarkThisProvider * provider );

    ~ServiceBookmarkThisCapability();

    virtual bool isBookmarkable();
    virtual QString browserName();
    virtual QString collectionName();
    virtual bool simpleFiltering();
    virtual PopupDropperAction * bookmarkAction();

private:

    BookmarkThisProvider * m_provider;
};

#endif
