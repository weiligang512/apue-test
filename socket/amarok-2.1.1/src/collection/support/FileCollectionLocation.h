/*
 *  Copyright (c) 2008 Casey Link <unnamedrambler@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AMAROK_FILECOLLECTIONLOCATION_H
#define AMAROK_FILECOLLECTIONLOCATION_H

#include "CollectionLocation.h"

#include <QSet>
#include <QMap>
#include <QString>

class SqlCollection;
class KJob;

class FileCollectionLocation : public CollectionLocation
{
    Q_OBJECT
    public:
        FileCollectionLocation();
        virtual ~FileCollectionLocation();

        virtual bool isWritable() const;
        virtual bool isOrganizable() const;
        virtual bool remove( const Meta::TrackPtr &track );
};

#endif
