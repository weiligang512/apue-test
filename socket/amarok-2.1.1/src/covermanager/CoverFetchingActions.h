/***************************************************************************
 *   Copyright (c) 2008  Seb Ruiz <ruiz@kde.org>                           *
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
#ifndef COVERFETCHINGACTIONS_H
#define COVERFETCHINGACTIONS_H

#include "context/popupdropper/libpud/PopupDropperAction.h"

#include "Meta.h"

/**
 * This collection of actions handles fetching, displaying and removing of album art
 *
 * @author Seb Ruiz
 */

class AMAROK_EXPORT BaseCoverAction : public PopupDropperAction
{
    Q_OBJECT
    public:
        BaseCoverAction( QObject *parent, Meta::AlbumPtr album )
            : PopupDropperAction( parent )
        {
            m_albums.append( album );
            connect( this, SIGNAL( triggered( bool ) ), SLOT( slotTriggered() ) );
        }
        BaseCoverAction( QObject *parent, Meta::AlbumList albums )
            : PopupDropperAction( parent )
        {
            m_albums = albums;
            connect( this, SIGNAL( triggered( bool ) ), SLOT( slotTriggered() ) );
        }

    protected slots:
        virtual void slotTriggered() = 0;
    protected:
        Meta::AlbumList m_albums;
};

class AMAROK_EXPORT FetchCoverAction : public BaseCoverAction
{
    Q_OBJECT
    public:
        FetchCoverAction( QObject *parent, Meta::AlbumPtr album )
            : BaseCoverAction( parent, album ) { init(); }
        FetchCoverAction( QObject *parent, Meta::AlbumList albums )
            : BaseCoverAction( parent, albums ) { init(); }

    protected slots:
        virtual void slotTriggered();
    protected:
        virtual void init();
};

class AMAROK_EXPORT DisplayCoverAction : public BaseCoverAction
{
    Q_OBJECT
    public:
        DisplayCoverAction( QObject *parent, Meta::AlbumPtr album )
            : BaseCoverAction( parent, album ) { init(); }
        DisplayCoverAction( QObject *parent, Meta::AlbumList albums )
            : BaseCoverAction( parent, albums ) { init(); }

    protected slots:
        virtual void slotTriggered();
    protected:
        virtual void init();
};

class AMAROK_EXPORT UnsetCoverAction : public BaseCoverAction
{
    Q_OBJECT
    public:
        UnsetCoverAction( QObject *parent, Meta::AlbumPtr album )
            : BaseCoverAction( parent, album ) { init(); }
        UnsetCoverAction( QObject *parent, Meta::AlbumList albums )
            : BaseCoverAction( parent, albums ) { init(); }

    protected slots:
        virtual void slotTriggered();
    protected:
        virtual void init();
};

class AMAROK_EXPORT SetCustomCoverAction : public BaseCoverAction
{
    Q_OBJECT
    public:
        SetCustomCoverAction( QObject *parent, Meta::AlbumPtr album )
            : BaseCoverAction( parent, album ) { init(); }
        SetCustomCoverAction( QObject *parent, Meta::AlbumList albums )
            : BaseCoverAction( parent, albums ) { init(); }

    protected slots:
        virtual void slotTriggered();
    protected:
        virtual void init();
};

#endif
