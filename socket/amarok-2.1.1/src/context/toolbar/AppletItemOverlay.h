/***************************************************************************
* copyright            : (C) 2008 Leo Franchi <lfranchi@kde.org>         *
****************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef AMAROK_APPLET_ITEM_OVERLAY_H
#define AMAROK_APPLET_ITEM_OVERLAY_H

#include <QWidget>

class QGraphicsLinearLayout;
class QGraphicsWidget;
class QToolButton;

// NOTE inspiration and code taken from kdebase/workspace/plasma/shells/desktop/panelappletoverlay.{h,cpp}

namespace Plasma
{
    class Applet;
}

namespace Context
{
    
class Applet;
class AppletToolbarAppletItem;
    
class AppletItemOverlay : public QWidget
{
    Q_OBJECT
    
public:
    AppletItemOverlay(AppletToolbarAppletItem *applet, QGraphicsLinearLayout* layout, QWidget *parent);
    ~AppletItemOverlay();
    
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    
    AppletToolbarAppletItem* applet();
protected:
    virtual void resizeEvent( QResizeEvent* );
    
signals:
    void moveApplet( Plasma::Applet*, int, int );
    void deleteApplet( Plasma::Applet* );
    
private slots:
    void deleteApplet();
    void delaySyncGeometry();
    void syncGeometry();

private:
    void swapWithPrevious();
    void swapWithNext();
    
    AppletToolbarAppletItem *m_applet;
    QGraphicsWidget *m_spacer;
    QGraphicsLinearLayout *m_layout;
    QRectF m_prevGeom;
    QRectF m_nextGeom;
    QPoint m_origin;
    QToolButton* m_deleteIcon;
    int m_offset;
    int m_index;
    bool m_clickDrag;
};

}

#endif
