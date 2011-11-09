/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>
  Copyright (c) 2007-2008 Mark Kretschmann <kretschmann@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef AMAROK_SIDEBARWIDGET_H
#define AMAROK_SIDEBARWIDGET_H

#include "SmartPointerList.h"
#include "SvgHandler.h"

#include <QAction>
#include <QAbstractButton>

#include <kvbox.h>

class QTimer;

class SideBarButton: public QAbstractButton
{
    Q_OBJECT

    typedef QAbstractButton super;

    public:
        SideBarButton( const QIcon &icon, const QString &text, QWidget *parent );

        virtual QSize sizeHint() const;

    protected:
        virtual void paintEvent( QPaintEvent *e );
        virtual void enterEvent( QEvent* );
        virtual void leaveEvent( QEvent* );
        virtual void paletteChange( const QPalette & oldPalette );

    private slots:
        virtual void slotAnimTimer();

    private:
        static QColor blendColors( const QColor& color1, const QColor& color2, int percent );

        virtual void dragEnterEvent( QDragEnterEvent* );
        virtual void dragLeaveEvent( QDragLeaveEvent* );

        int widthHint() const;
        int heightHint() const;

        static const int ANIM_INTERVAL = 18;
        static const int ANIM_MAX = 20;
        static const int AUTO_OPEN_TIME = 1000;

        bool m_animEnter;
        int m_animCount;
        QTimer* m_animTimer;
        QTimer* m_autoOpenTimer;
};


class SideBarWidget: public KVBox
{
    typedef KVBox super;

    Q_OBJECT

    public:
        SideBarWidget( QWidget *parent );
        virtual ~SideBarWidget();

        /* Restores visible/invisible and active/inactive state of browsers from last session */
        void restoreSession();

        int addSideBar( const QIcon &icon, const QString &text );
        int count() const;
        QString text( int index ) const;
        QIcon icon( int index ) const;

    public slots:
        void open( int index );

    signals:
        void opened( int index );
        void closed();

    protected:
        virtual void wheelEvent( QWheelEvent *e );

    private slots:
        void slotClicked( bool checked );
        void slotSetVisible( bool visible );

    private:
        void updateShortcuts();

        SmartPointerList<SideBarButton> m_buttons;
        SmartPointerList<QAction> m_actions;
        SmartPointerList<QAction> m_shortcuts;

        QAction* m_closeShortcut;
        QList<int> m_visible;
};


#endif
