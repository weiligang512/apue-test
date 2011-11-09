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

#ifndef AMAROKURLHANDLER_H
#define AMAROKURLHANDLER_H

#include "amarok_export.h"
#include "AmarokUrlRunnerBase.h"
#include "Meta.h"

#include <QMap>

class TimecodeObserver;
class AmarokUrlHandler;
class NavigationUrlRunner;
class PlayUrlRunner;

namespace The {
    AMAROK_EXPORT AmarokUrlHandler* amarokUrlHandler();
}

/**
A singleton class for handling and delegating all amarok:// urls

	@author Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>
*/
class AmarokUrlHandler : public QObject
{
    Q_OBJECT
    friend AmarokUrlHandler* The::amarokUrlHandler();

public:

    void registerRunner( AmarokUrlRunnerBase * runner, const QString & command );
    void unRegisterRunner( AmarokUrlRunnerBase * runner );

    bool run( AmarokUrl url );

public slots:
    void bookmarkAlbum( Meta::AlbumPtr album );
    void bookmarkArtist( Meta::ArtistPtr artist );

private:

    AmarokUrlHandler();
    ~AmarokUrlHandler();

    QMap<QString, AmarokUrlRunnerBase *> m_registeredRunners;

    NavigationUrlRunner * m_navigationRunner;
    PlayUrlRunner * m_playRunner;
    TimecodeObserver * m_timecodeObserver;

};

#endif
