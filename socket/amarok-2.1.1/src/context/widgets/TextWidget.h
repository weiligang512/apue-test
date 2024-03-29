/***************************************************************************
* copyright            : (C) 2007 Leo Franchi <lfranchi@gmail.com>        *
**************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef AMAROK_TEXT_WIDGET_H
#define AMAROK_TEXT_WIDGET_H

#include "amarok_export.h"

#include <plasma/layouts/layoutitem.h>

#include <QGraphicsRectItem>

namespace Context
{

class AMAROK_EXPORT TextWidget : public QGraphicsTextItem,
                                 public Plasma::LayoutItem
{
        
public:
    explicit TextWidget( QGraphicsItem* parent = 0, QGraphicsScene* scene = 0 );
    
    void setText( const QString text );
    
    // layout stuff
    Qt::Orientations expandingDirections() const;
    
    QSizeF minimumSize() const;
    QSizeF maximumSize() const;
    
    bool hasHeightForWidth() const;
    qreal heightForWidth( qreal w ) const;
    
    bool hasWidthForHeight() const;
    qreal widthForHeight(qreal h) const;
    
    QRectF geometry() const;
    void setGeometry( const QRectF& geometry );
    
    QSizeF sizeHint() const;
    
private:
    
    QTextDocument* shortenHeight( qreal height );
};

} // Context namespace

#endif
