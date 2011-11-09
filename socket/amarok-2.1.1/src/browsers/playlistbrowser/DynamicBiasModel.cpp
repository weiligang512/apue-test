/***************************************************************************
 * copyright         : (C) 2008 Daniel Caleb Jones <danielcjones@gmail.com>
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

#include "DynamicBiasModel.h"

#include "Debug.h"
#include "DynamicBiasWidgets.h"
#include "BiasedPlaylist.h"

#include <QVariant>

#include <KLocale>

#include <typeinfo>

PlaylistBrowserNS::DynamicBiasModel::DynamicBiasModel( QListView* listView )
    : m_listView( listView )
{
}

PlaylistBrowserNS::DynamicBiasModel::~DynamicBiasModel()
{
    clearWidgets();
}

void
PlaylistBrowserNS::DynamicBiasModel::setPlaylist( Dynamic::DynamicPlaylistPtr playlist )
{
    DEBUG_BLOCK

    if( playlist.data() == m_playlist.data() )
        return;

    clearWidgets();

    Dynamic::BiasedPlaylist* bp = 
        dynamic_cast<Dynamic::BiasedPlaylist*>( playlist.data() );

    if( bp )
    {
        beginInsertRows( QModelIndex(), 0, bp->biases().size() );
        m_playlist = bp;
        foreach( Dynamic::Bias* b, bp->biases() )
        {
            debug() << "BIAS ADDED";
            PlaylistBrowserNS::BiasWidget* widget = b->widget( m_listView->viewport() );

            connect( widget, SIGNAL(widgetChanged(QWidget*)),
                    SLOT(widgetChanged(QWidget*)) );
            connect( widget, SIGNAL(biasRemoved(Dynamic::Bias*)),
                    SLOT(removeBias(Dynamic::Bias*)) );
            connect( widget, SIGNAL(biasChanged(Dynamic::Bias*)),
                    SLOT(biasChanged(Dynamic::Bias*)) );

            if( !m_widgets.isEmpty() )
                widget->setAlternate( !m_widgets.back()->alternate() );

            m_widgets.append( widget );
        }

        // add the bias adding widgets

        PlaylistBrowserNS::BiasAddWidget* globalAdder =
            new PlaylistBrowserNS::BiasAddWidget(
                    i18n( "Proportional Bias" ),
                    i18n( "Match a certain portion of the playlist to a value." ),
                    m_listView->viewport() );
        if( !m_widgets.isEmpty() )
            globalAdder->setAlternate( !m_widgets.back()->alternate() );


        connect( globalAdder, SIGNAL(addBias()),
                SLOT(appendGlobalBias()) );

        m_widgets.append( globalAdder );
        connect( m_widgets.back(), SIGNAL(widgetChanged(QWidget*)),
                this, SLOT(widgetChanged(QWidget*)) );


        PlaylistBrowserNS::BiasAddWidget* normalAdder =
            new PlaylistBrowserNS::BiasAddWidget(
                    i18n( "Fuzzy Bias" ),
                    i18n( "Loosely match the playlist to an approximate value." ),
                    m_listView->viewport() );

        if( !m_widgets.isEmpty() )
            normalAdder->setAlternate( !m_widgets.back()->alternate() );

        connect( normalAdder, SIGNAL(addBias()),
                SLOT(appendNormalBias()) );

        m_widgets.append( normalAdder );
        connect( m_widgets.back(), SIGNAL(widgetChanged(QWidget*)),
                this, SLOT(widgetChanged(QWidget*)) );

        endInsertRows();
    }
    // TODO: else add the "Unknown Playlist Type" box.
}

void
PlaylistBrowserNS::DynamicBiasModel::appendGlobalBias()
{
    Dynamic::GlobalBias* gb = 
        new Dynamic::GlobalBias( 0.0, XmlQueryReader::Filter() );
    gb->setActive( false );
    appendBias( gb );
}

void
PlaylistBrowserNS::DynamicBiasModel::appendNormalBias()
{
    appendBias( new Dynamic::NormalBias() );
}

void
PlaylistBrowserNS::DynamicBiasModel::appendBias( Dynamic::Bias* b )
{
    if( !m_playlist )
        return;

    beginInsertRows( QModelIndex(), rowCount()-2, rowCount()-2 );
    m_playlist->biases().append( b );

    PlaylistBrowserNS::BiasWidget* widget = b->widget( m_listView->viewport() );

    connect( widget, SIGNAL(widgetChanged(QWidget*)),
            SLOT(widgetChanged(QWidget*)) );
    connect( widget, SIGNAL(biasRemoved(Dynamic::Bias*)),
            SLOT(removeBias(Dynamic::Bias*)) );
    connect( widget, SIGNAL(biasChanged(Dynamic::Bias*)),
            SLOT(biasChanged(Dynamic::Bias*)) );

    if( m_widgets.size() > 2 )
        widget->setAlternate( !m_widgets.at( m_widgets.size() - 3 )->alternate() );

    // toggle the 'add bias' dialog background
    for( int i = rowCount()-2; i < rowCount(); ++i )
        m_widgets[i]->toggleAlternate();

    m_widgets.insert( rowCount()-2, widget );
    endInsertRows();

    // fix a render bug
    m_widgets.back()->hide();


    emit playlistModified( m_playlist );
}

void
PlaylistBrowserNS::DynamicBiasModel::removeBias( Dynamic::Bias* b )
{
    int index = m_playlist->biases().indexOf( b );
    if( index == -1 )
        return;

    beginRemoveRows( QModelIndex(), index, index );
    delete m_widgets[index];
    m_widgets.removeAt( index );
    m_playlist->biases().removeAt( index );
    endRemoveRows();

    for( int i = index; i < m_widgets.size(); ++i )
        m_widgets[i]->toggleAlternate();

    emit playlistModified( m_playlist );
}

void
PlaylistBrowserNS::DynamicBiasModel::biasChanged( Dynamic::Bias* b )
{
    Q_UNUSED(b);
    emit playlistModified( m_playlist );
}


void
PlaylistBrowserNS::DynamicBiasModel::clearWidgets()
{
    if( rowCount() <= 0 ) return;

    beginRemoveRows( QModelIndex(), 0, rowCount() - 2 );

    foreach( PlaylistBrowserNS::BiasBoxWidget* b, m_widgets )
    {
        delete b;
    }
    m_widgets.clear();

    endRemoveRows();
}


QVariant
PlaylistBrowserNS::DynamicBiasModel::data( const QModelIndex& index, int role ) const
{
    if( m_playlist && role == WidgetRole && index.row() >= 0 && index.row() < m_widgets.size() )
        return QVariant::fromValue( m_widgets.at( index.row() ) );
    else
        return QVariant();
}


QModelIndex
PlaylistBrowserNS::DynamicBiasModel::index( int row, int column,
        const QModelIndex& parent ) const
{
    Q_UNUSED(parent)
    if( rowCount() <= row ) return QModelIndex();

    return createIndex( row, column, 0 );
}

QModelIndex
PlaylistBrowserNS::DynamicBiasModel::indexOf( PlaylistBrowserNS::BiasBoxWidget* widget )
{
    return index( m_widgets.indexOf( widget ), 0 );
}

QModelIndex
PlaylistBrowserNS::DynamicBiasModel::parent( const QModelIndex& index ) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int
PlaylistBrowserNS::DynamicBiasModel::rowCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent)
    return m_playlist ? m_widgets.size() : 0;
}

int
PlaylistBrowserNS::DynamicBiasModel::columnCount( const QModelIndex& parent ) const 
{
    Q_UNUSED(parent)
    return 1;
}

void
PlaylistBrowserNS::DynamicBiasModel::widgetChanged( QWidget* w )
{
    Q_UNUSED(w)

    // more or less a hack to get the delegate to redraw the list correctly when
    // the size of one of the widgets changes.

    int i;
    if( w )
    {
        for( i = 0; i < m_widgets.size(); ++i )
        {
            if( m_widgets[i] == w )
                break;
        }
    }
    else i = 0;

    for( int j = 0; j < m_widgets.size(); ++j )
        m_widgets[i]->hide();

    beginInsertRows( QModelIndex(), i, i );
    endInsertRows();
}

