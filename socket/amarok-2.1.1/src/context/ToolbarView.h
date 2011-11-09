/**************************************************************************
* copyright            : (C) 2008 Leo Franchi <lfranchi@kde.org>          *
**************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef AMAROK_CONTEXT_TOOLBAR_VIEW
#define AMAROK_CONTEXT_TOOLBAR_VIEW

#include <QGraphicsView>

class QGraphicsScene;
class QWidget;

namespace Plasma
{
    class Applet;
    class Containment;
}

namespace Context
{

class AppletToolbar;
class AppletToolbarAppletItem;
class AppletItemOverlay;

class ToolbarView : public QGraphicsView
{
    Q_OBJECT
    public:
        explicit ToolbarView( Plasma::Containment* cont, QGraphicsScene* scene, QWidget* parent = 0 );
        ~ToolbarView();
        
        virtual QSize sizeHint() const;
        int heightForWidth ( int w ) const;
    protected:
        void resizeEvent( QResizeEvent * event );
        void dragEnterEvent(QDragEnterEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);
        void dragLeaveEvent(QDragLeaveEvent *event);
    
    private slots:
        void toggleConfigMode();
        void appletRemoved( Plasma::Applet* );
        void appletAdded( Plasma::Applet*, int);
        void refreshOverlays();
        void recreateOverlays();
        void installApplets();
    
    private:
        int m_height;
        AppletToolbar* m_toolbar;
        QList< AppletItemOverlay* > m_moveOverlays;
        Plasma::Containment* m_cont;
};    
    
}


#endif
