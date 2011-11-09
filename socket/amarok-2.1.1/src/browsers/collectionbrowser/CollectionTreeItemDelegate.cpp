/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *   Copyright (c) 2008  Mark Kretschmann <kretschmann@kde.org>            *
 *   Copyright (c) 2009  Seb Ruiz <ruiz@kde.org>                           *
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

#include "CollectionTreeItemDelegate.h"
#include "CollectionTreeItem.h"

#include "App.h"
#include "Debug.h"

#include <QApplication>
#include <QFontMetrics>
#include <QIcon>
#include <QPainter>


CollectionTreeItemDelegate::CollectionTreeItemDelegate( QTreeView *view )
    : QStyledItemDelegate()
    , m_view( view )
{
    DEBUG_BLOCK

    m_bigFont.setBold( true );
    m_smallFont.setPointSize( m_smallFont.pointSize() - 1 );
}

CollectionTreeItemDelegate::~CollectionTreeItemDelegate()
{}

void
CollectionTreeItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    if( index.parent().isValid() ) // not a root item
    {
        QStyledItemDelegate::paint( painter, option, index );
        return;
    }
   
    const QPoint topLeft = option.rect.topLeft();
    const int width = m_view->viewport()->size().width() - 4;
    const int height = sizeHint( option, index ).height();
    const int iconWidth = 32;
    const int iconHeight = 32;
    const int iconPadX = 4;
    const int iconPadY = 4;

    painter->save();

    QApplication::style()->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter );

    if ( option.state & QStyle::State_Selected )
        painter->setPen( App::instance()->palette().highlightedText().color() );
    else
        painter->setPen( App::instance()->palette().text().color() );
    
    painter->setRenderHint( QPainter::Antialiasing );

    const int iconYPadding = ( height - iconHeight ) / 2;
    QPoint iconPos( topLeft + QPoint( iconPadX, iconYPadding ) );
    if( QApplication::isRightToLeft() )
        iconPos = QPoint( width - iconWidth - iconPadX, iconYPadding );

    painter->drawPixmap( iconPos,
                         index.data( Qt::DecorationRole ).value<QIcon>().pixmap( iconWidth, iconHeight ) );

    const int iconRight = topLeft.x() + iconWidth + iconPadX * 2;
    QRectF titleRect;
    titleRect.setLeft( QApplication::isRightToLeft() ? 0 : iconRight );
    titleRect.setTop( option.rect.top() + iconYPadding );
    titleRect.setWidth( width - iconRight );
    titleRect.setHeight( height );

    const QString collectionName = index.data( Qt::DisplayRole ).toString();

    painter->setFont( m_bigFont );
    painter->drawText( titleRect, Qt::AlignLeft, collectionName );

    QFontMetrics bigFm( m_bigFont );

    QRectF textRect;
    textRect.setLeft( QApplication::isRightToLeft() ? 0 : iconRight );
    textRect.setTop( option.rect.top() + iconYPadding + bigFm.boundingRect( collectionName ).height() );
    textRect.setWidth( width - iconPadX * 2 );
    textRect.setHeight( height - ( iconHeight + iconPadY ) );

    painter->setFont( m_smallFont );
    painter->drawText( textRect, Qt::TextWordWrap, index.data( CustomRoles::ByLineRole ).toString() );

    painter->restore();
}

QSize
CollectionTreeItemDelegate::sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    if( index.parent().isValid() )
        return QStyledItemDelegate::sizeHint( option, index );

    int width = m_view->viewport()->size().width() - 4;
    int height;

    QFontMetrics bigFm( m_bigFont );
    QFontMetrics smallFm( m_smallFont );
    
    height = bigFm.boundingRect( 0, 0, width, 50, Qt::AlignLeft, index.data( Qt::DisplayRole ).toString() ).height()
           + smallFm.boundingRect( 0, 0, width, 50, Qt::AlignLeft, index.data( CustomRoles::ByLineRole ).toString() ).height()
           + 20;

    return QSize( width, height );
}


