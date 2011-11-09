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

#include "Applet.h"

#include "PaletteHandler.h"
#include <plasma/animator.h>

#include <QGraphicsLayout>
#include <QGraphicsScene>
#include <QFontMetrics>
#include <QPainter>

namespace Context
{

} // Context namespace

Context::Applet::Applet( QObject * parent, const QVariantList& args )
    : Plasma::Applet( parent, args )
    , m_transient( 0 )
    , m_standardPadding( 6.0 )
{}

QFont
Context::Applet::shrinkTextSizeToFit( const QString& text, const QRectF& bounds )
{
    Q_UNUSED( text );

    int size = 13; // start here, shrink if needed
    QFont font( QString(), size, QFont::Light );
    font.setStyleHint( QFont::SansSerif );
    font.setStyleStrategy( QFont::PreferAntialias );
    
    QFontMetrics fm( font );
    while( fm.height() > bounds.height() + 4 )
    {
        if( size < 0 )
        {
            size = 5;
            break;
        }
        size--;
        fm = QFontMetrics( QFont( QString(), size ) );
    }
    
    // for aesthetics, we make it one smaller
    size--;

    QFont returnFont( QString(), size, QFont::Light );
    font.setStyleHint( QFont::SansSerif );
    font.setStyleStrategy( QFont::PreferAntialias );
    
    return QFont( returnFont );
}

QString
Context::Applet::truncateTextToFit( QString text, const QFont& font, const QRectF& bounds )
{
    QFontMetrics fm( font );
    return fm.elidedText ( text, Qt::ElideRight, (int)bounds.width() );
    
}

void
Context::Applet::drawRoundedRectAroundText( QPainter* p, QGraphicsSimpleTextItem* t )
{

   p->setRenderHint( QPainter::Antialiasing );
   QColor col = PaletteHandler::highlightColor().lighter( 150 );
   p->save();
   QRectF rect = t->boundingRect();
   rect.moveTopLeft( t->pos() );
   QPainterPath path;
   path.addRoundedRect( rect.adjusted( -5, -2, 5, 2 ), 3, 3 );

   p->fillPath( path, col );
   p->restore();
   // draw outline around textbox
   p->save();
   col = PaletteHandler::highlightColor( 0.3, .5 );
   p->setPen( col );
   rect = t->boundingRect();
   rect.moveTopLeft( t->pos() );
   p->drawRoundedRect( rect.adjusted( -5, -2, 5, 2 ), 3, 3 );
   p->restore(); 
}

void
Context::Applet::addGradientToAppletBackground( QPainter* p )
{
        // tint the whole applet
    // draw non-gradient backround. going for elegance and style
    p->save();
    QPainterPath path;
    path.addRoundedRect( boundingRect().adjusted( 0, 1, -1, -1 ), 3, 3 );
    //p->fillPath( path, gradient );
    QColor highlight = PaletteHandler::highlightColor( 0.4, 1.05 );
    highlight.setAlpha( 120 );
    p->fillPath( path, highlight );
    p->restore();

    p->save();
    QColor col = PaletteHandler::highlightColor( 0.3, .7 );
    p->setPen( col );
    p->drawRoundedRect( boundingRect().adjusted( 2, 2, -2, -2 ), 3, 3 );
    p->restore(); 
}

qreal
Context::Applet::standardPadding()
{
    return  m_standardPadding;
}


void
Context::Applet::destroy()
{
    if ( Plasma::Applet::immutability() != Plasma::Mutable || m_transient ) {
        return; //don't double delete
    }
    m_transient = true;
    cleanUpAndDelete();
}

void
Context::Applet::cleanUpAndDelete()
{
    QGraphicsWidget *parent = dynamic_cast<QGraphicsWidget *>( parentItem() );
    //it probably won't matter, but right now if there are applethandles, *they* are the parent.
    //not the containment.

    //is the applet in a containment and is the containment have a layout? if yes, we remove the applet in the layout
    if ( parent && parent->layout() )
    {
        QGraphicsLayout *l = parent->layout();
        for ( int i = 0; i < l->count(); ++i )
        {
            if ( this == l->itemAt( i ) )
            {
                l->removeAt( i );
                break;
            }
        }
    }

    if ( Plasma::Applet::configScheme() ) {
        Plasma::Applet::configScheme()->setDefaults();
    }

    Plasma::Applet::scene()->removeItem( this );
    Plasma::Applet::deleteLater();
}

