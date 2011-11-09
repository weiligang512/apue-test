/***************************************************************************
 * copyright            : (C) 2007 Ian Monroe <ian@monroe.nu>
 *                      : (C) 2008 - 2009 Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>
 *                      : (C) 2008 Soren Harward <stharward@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#define DEBUG_PREFIX "Playlist::PrettyItemDelegate"

#include "PrettyItemDelegate.h"

#include "App.h"
#include "Debug.h"
#include "SvgHandler.h"
#include "SvgTinter.h"
#include "meta/Meta.h"
#include "meta/capabilities/SourceInfoCapability.h"
#include "playlist/GroupingProxy.h"
#include "playlist/PlaylistModel.h"
#include "playlist/layouts/LayoutManager.h"

#include "kratingpainter.h"

#include <QFontMetricsF>
#include <QPainter>

using namespace Playlist;

const qreal PrettyItemDelegate::ALBUM_WIDTH = 50.0;
const qreal PrettyItemDelegate::SINGLE_TRACK_ALBUM_WIDTH = 40.0;
const qreal PrettyItemDelegate::MARGIN = 2.0;
const qreal PrettyItemDelegate::MARGINH = 6.0;
const qreal PrettyItemDelegate::MARGINBODY = 1.0;
const qreal PrettyItemDelegate::PADDING = 1.0;

int Playlist::PrettyItemDelegate::s_fontHeight = 0;


Playlist::PrettyItemDelegate::PrettyItemDelegate( QObject* parent )
    : QStyledItemDelegate( parent )
{
    DEBUG_BLOCK

    LayoutManager::instance();
}

PrettyItemDelegate::~PrettyItemDelegate() { }

QSize
PrettyItemDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    int height = 0;

    QFontMetricsF nfm( option.font );
    QFont boldfont( option.font );
    boldfont.setBold( true );
    QFontMetricsF bfm( boldfont );

    s_fontHeight = bfm.height();

    PlaylistLayout layout = LayoutManager::instance()->activeLayout();

    const int groupMode = index.data( GroupRole ).toInt();
    int rowCount = 1;

    switch ( groupMode )
    {
        case Head:
            rowCount = layout.head().rows() + layout.body().rows();
            break;

        case Body:
            rowCount = layout.body().rows();
            break;

        case Tail:
            rowCount = layout.body().rows();
            break;

        case None:
        default:
            rowCount = layout.single().rows();
            break;
    }

    height = MARGIN * 2 + rowCount * s_fontHeight + ( rowCount - 1 ) * PADDING;
    return QSize( 120, height );
}

void
PrettyItemDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    PlaylistLayout layout = LayoutManager::instance()->activeLayout();
    
    painter->save();
    QApplication::style()->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter );
    painter->translate( option.rect.topLeft() );

    painter->drawPixmap( 0, 0, The::svgHandler()->renderSvgWithDividers( "track", ( int )option.rect.width(), ( int )option.rect.height(), "track" ) );

    if ( option.state & QStyle::State_Selected )
        painter->setPen( App::instance()->palette().highlightedText().color() );
    else
        painter->setPen( App::instance()->palette().text().color() );

    // call paint method based on type
    const int groupMode = index.data( GroupRole ).toInt();

    if ( groupMode == None )
        paintItem( layout.single(), painter, option, index );
    else if ( groupMode == Head )
    {
        //we need to split up the options for the actual header and the included first track

        QFont boldfont( option.font );
        boldfont.setBold( true );
        QFontMetricsF bfm( boldfont );

        QStyleOptionViewItem headOption( option );
        QStyleOptionViewItem trackOption( option );

        int headRows = layout.head().rows();
        int headHeight ;

        if ( headRows > 0 )
        {
            headHeight = MARGIN * 2 + headRows * s_fontHeight + ( headRows - 1 ) * PADDING;
            headOption.rect = QRect( 0, 0, option.rect.width(), headHeight );
            paintItem( layout.head(), painter, headOption, index, true );
            painter->translate( 0, headHeight - 3 );
        } 

        int trackRows = layout.body().rows();
        int trackHeight = MARGIN * 2 + trackRows * s_fontHeight + ( trackRows - 1 ) * PADDING;
        trackOption.rect = QRect( 0, 0, option.rect.width(), trackHeight );
        paintItem( layout.body(), painter, trackOption, index );
        
    } else if ( groupMode == Body )
        paintItem( layout.body(), painter, option, index );
    else if ( groupMode == Tail )
        paintItem( layout.body(), painter, option, index );
    else
        QStyledItemDelegate::paint( painter, option, index );

    painter->restore();
}

bool
PrettyItemDelegate::insideItemHeader( const QPoint& pt, const QRect& rect )
{
    int headRows = LayoutManager::instance()->activeLayout().head().rows();

    if ( headRows < 1 )
        return false;

    QRect headerBounds = rect.adjusted( ( int )MARGINH,
                                        ( int )MARGIN,
                                        ( int )( -MARGINH ),
                                                    0 );
    
    headerBounds.setHeight( static_cast<int>( 2 * MARGIN + headRows * s_fontHeight ) );

    return headerBounds.contains( pt );
}

QPointF
PrettyItemDelegate::centerImage( const QPixmap& pixmap, const QRectF& rect ) const
{
    qreal pixmapRatio = ( qreal )pixmap.width() / ( qreal )pixmap.height();

    qreal moveByX = 0.0;
    qreal moveByY = 0.0;

    if ( pixmapRatio >= 1 )
        moveByY = ( rect.height() - ( rect.width() / pixmapRatio ) ) / 2.0;
    else
        moveByX = ( rect.width() - ( rect.height() * pixmapRatio ) ) / 2.0;

    return QPointF( moveByX, moveByY );
}


void Playlist::PrettyItemDelegate::paintItem( LayoutItemConfig config, QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index, bool ignoreQueueMarker ) const
{
    int rowCount = config.rows();

    if ( rowCount == 0 )
        return;
    
    int rowHeight = option.rect.height() / rowCount;

    int rowOffsetX = MARGINH;
    int rowOffsetY = 0;

    int imageSize = option.rect.height() - MARGIN * 2;
    QRectF nominalImageRect( MARGINH, MARGIN, imageSize, imageSize );
    
    if ( config.showCover() )
    {
        QModelIndex coverIndex = index.model()->index( index.row(), CoverImage );
        QPixmap albumPixmap = coverIndex.data( Qt::DisplayRole ).value<QPixmap>();

        //offset cover if non square
        QPointF offset = centerImage( albumPixmap, nominalImageRect );
        QRectF imageRect( nominalImageRect.x() + offset.x(),
                          nominalImageRect.y() + offset.y(),
                          nominalImageRect.width() - offset.x() * 2,
                          nominalImageRect.height() - offset.y() * 2 );

        if ( !albumPixmap.isNull() )
            painter->drawPixmap( imageRect, albumPixmap, QRectF( albumPixmap.rect() ) );

        QModelIndex emblemIndex = index.model()->index( index.row(), SourceEmblem );
        QPixmap emblemPixmap = emblemIndex.data( Qt::DisplayRole ).value<QPixmap>();

        if ( !albumPixmap.isNull() )
            painter->drawPixmap( QRectF( nominalImageRect.x(), nominalImageRect.y() , 16, 16 ), emblemPixmap, QRectF( 0, 0 , 16, 16 ) );

        rowOffsetX = imageSize + MARGINH + PADDING * 2;
    }

    int markerOffsetX = nominalImageRect.x();
    
    if( index.data( StateRole ).toInt() & Item::Queued && !ignoreQueueMarker )
    {
        const int queuePosition = index.data( QueuePositionRole ).toInt();
        const int w = 16, h = 16;
        const int x = markerOffsetX;
        const int y = nominalImageRect.y() + ( imageSize - h );
        const QRect rect( x, y, w, h );
        painter->drawPixmap( x, y, The::svgHandler()->renderSvg( "queue_marker", w, h, "queue_marker" ) );
        painter->drawText( rect, Qt::AlignCenter, QString::number( queuePosition ) );

        markerOffsetX += ( 16 + PADDING );

        if ( !config.showCover() )
            rowOffsetX = markerOffsetX;
    }

    if( index.data( MultiSourceRole ).toBool() && !ignoreQueueMarker )
    {
        const int w = 16, h = 16;
        const int x = markerOffsetX;
        const int y = nominalImageRect.y() + ( imageSize - h );
        const QRect rect( x, y, w, h );
        painter->drawPixmap( x, y, The::svgHandler()->renderSvg( "multi_marker", w, h, "multi_marker" ) );

        markerOffsetX += ( 16 + PADDING );

        if ( !config.showCover() )
            rowOffsetX += ( 16 + PADDING );
    }

    if( index.data( StopAfterTrackRole ).toBool() )
    {
        const int w = 16, h = 16;
        const int x = markerOffsetX;
        const int y = nominalImageRect.y() + ( imageSize - h );
        const QRect rect( x, y, w, h );
        painter->drawPixmap( x, y, The::svgHandler()->renderSvg( "stop_button", w, h, "stop_button" ) );

        markerOffsetX += ( 16 + PADDING );

        if ( !config.showCover() )
            rowOffsetX += ( 16 + PADDING );
    }

    for ( int i = 0; i < rowCount; i++ )
    {
        LayoutItemConfigRow row = config.row( i );
        qreal itemOffsetX = rowOffsetX;

        const int elementCount = row.count();

        qreal rowWidth = option.rect.width() - ( rowOffsetX + MARGINH );

        if ( i == config.activeIndicatorRow() && index.data( ActiveTrackRole ).toBool() )
        {
            painter->drawPixmap( rowOffsetX - 1, rowOffsetY + 1,
                                  The::svgHandler()->renderSvg(
                                  "active_overlay",
                                  rowWidth + 2, rowHeight - 2,
                                  "active_overlay" ) );
        }

        QRectF rowBox( itemOffsetX, rowOffsetY, rowWidth, rowHeight );
        int currentItemX = itemOffsetX;
        
        //we need to do a quick pass to figure out how much space is left for auto sizing elements
        qreal spareSpace = 1.0;
        int autoSizeElemCount = 0;
        for ( int k = 0; k < elementCount; ++k )
        {
            spareSpace -= row.element( k ).size();
            if ( row.element( k ).size() < 0.001 )
                autoSizeElemCount++;
        }

        qreal spacePerAutoSizeElem = spareSpace / (qreal) autoSizeElemCount;

        for ( int j = 0; j < elementCount; ++j )
        {
            LayoutItemConfigRowElement element = row.element( j );

            int value = element.value();

            QModelIndex textIndex = index.model()->index( index.row(), value );
            QString text = textIndex.data( Qt::DisplayRole ).toString();

            qreal itemWidth = 0.0;

            text = element.prefix() + text + element.suffix();

            bool bold = element.bold();
            bool italic = element.italic();
            int alignment = element.alignment();

            QFont font = option.font;
            font.setBold( bold );
            font.setItalic( italic );
            painter->setFont( font );

            QRectF elementBox;

            qreal size;
            if ( element.size() > 0.0001 )
                size = element.size();
            else 
                size = spacePerAutoSizeElem;

            if ( size > 0.0001 )
            {
                itemWidth = rowWidth * size;

                //special case for painting the rating...
                if ( value == Rating )
                {
                    if( textIndex.data( InCollectionRole ).toBool() )
                    {
                        int rating = textIndex.data( Qt::DisplayRole ).toInt();

                        Qt::Alignment ratingAlignment;
                        if ( alignment & Qt::AlignLeft )
                            ratingAlignment = Qt::AlignLeft;
                        else if ( alignment & Qt::AlignRight )
                            ratingAlignment = Qt::AlignRight;
                        else
                            ratingAlignment = Qt::AlignCenter;

                        KRatingPainter::paintRating( painter, QRect( currentItemX, rowOffsetY + 1, itemWidth, rowHeight - 2 ), ratingAlignment, rating, rating );
                    }

                } else if ( value == Divider )
                {
                    QPixmap left = The::svgHandler()->renderSvg(
                            "divider_left",
                            1, rowHeight ,
                            "divider_left" );

                    QPixmap right = The::svgHandler()->renderSvg(
                            "divider_right",
                            1, rowHeight,
                            "divider_right" );

                    if ( alignment & Qt::AlignLeft )
                    {
                        painter->drawPixmap( currentItemX, rowOffsetY, left );
                        painter->drawPixmap( currentItemX + 1, rowOffsetY, right );
                    }
                    else if ( alignment & Qt::AlignRight )
                    {
                        painter->drawPixmap( currentItemX + itemWidth - 1, rowOffsetY, left );
                        painter->drawPixmap( currentItemX + itemWidth, rowOffsetY, right );
                    }
                    else
                    {
                        int center = currentItemX + ( itemWidth / 2 );
                        painter->drawPixmap( center, rowOffsetY, left );
                        painter->drawPixmap( center + 1, rowOffsetY, right );
                    }
                }
                else
                {
                    text = QFontMetricsF( font ).elidedText( text, Qt::ElideRight, itemWidth );
                    painter->drawText( currentItemX, rowOffsetY, itemWidth, rowHeight, alignment, text );
                }

                currentItemX += itemWidth;
            }

        }
        rowOffsetY += rowHeight;
    }
}


