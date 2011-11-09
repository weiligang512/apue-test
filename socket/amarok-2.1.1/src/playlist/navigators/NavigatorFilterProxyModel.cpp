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

#include "NavigatorFilterProxyModel.h"

#include "Debug.h"
#include "playlist/PlaylistModel.h"

namespace Playlist {

NavigatorFilterProxyModel* NavigatorFilterProxyModel::s_instance = 0;

NavigatorFilterProxyModel* NavigatorFilterProxyModel::instance()
{
    if ( s_instance == 0 )
        s_instance = new NavigatorFilterProxyModel();
    return s_instance;
}

NavigatorFilterProxyModel::NavigatorFilterProxyModel()
    : QSortFilterProxyModel( Model::instance() )
{
    setSourceModel( Model::instance() );

    connect( Model::instance(), SIGNAL( insertedIds( const QList<quint64>& ) ), this, SLOT( slotInsertedIds( const QList<quint64>& ) ) );
    connect( Model::instance(), SIGNAL( removedIds( const QList<quint64>& ) ), this, SLOT( slotRemovedIds( const QList<quint64>& ) ) );

    KConfigGroup config = Amarok::config("Playlist Search");
    m_passThrough = !config.readEntry( "ShowOnlyMatches", true );

    //setDynamicSortFilter( true );
}

NavigatorFilterProxyModel::~NavigatorFilterProxyModel()
{
}

bool NavigatorFilterProxyModel::filterAcceptsRow( int row, const QModelIndex & source_parent ) const
{
    Q_UNUSED( source_parent );

    if ( m_passThrough )
        return true;

    const bool match = Model::instance()->matchesCurrentSearchTerm( row );
    return match;
}

int NavigatorFilterProxyModel::activeRow() const
{
    // we map the active row form the source to this model. if the active row is not in the items
    // exposed by this proxy, just point to our first item.
    Model * model = Model::instance();
    return rowFromSource( model->activeRow() );
}

quint64 NavigatorFilterProxyModel::idAt( const int row ) const
{
    QModelIndex index = this->index( row, 0 );
    QModelIndex sourceIndex = mapToSource( index );
    return Model::instance()->idAt( sourceIndex.row() );
}

void NavigatorFilterProxyModel::filterUpdated()
{
    if ( !m_passThrough )
    {
        invalidateFilter();
        emit( filterChanged() );
        emit( layoutChanged() );
    }
}

int NavigatorFilterProxyModel::firstMatchAfterActive()
{
    Model * model = Model::instance();
    int activeSourceRow = model->activeRow();

    if ( m_passThrough )
        return activeSourceRow + 1;

    int matchRow = -1;
    int nextRow = activeSourceRow + 1;
    while ( model->rowExists( nextRow ) )
    {
        if ( model->matchesCurrentSearchTerm( nextRow ) ) {
            matchRow = nextRow;
            break;
        }

        nextRow++;
    }

    if ( matchRow == -1 )
        return -1;

    return rowFromSource( matchRow );
}

int NavigatorFilterProxyModel::firstMatchBeforeActive()
{
    Model * model = Model::instance();
    int activeSourceRow = model->activeRow();

    if ( m_passThrough )
        return activeSourceRow - 1;

    int matchRow = -1;
    int previousRow = activeSourceRow - 1;
    while ( model->rowExists( previousRow ) )
    {
        if ( model->matchesCurrentSearchTerm( previousRow ) ) {
            matchRow = previousRow;
            break;
        }

        previousRow--;
    }

    if ( matchRow == -1 )
        return -1;

    return rowFromSource( matchRow );
}

void NavigatorFilterProxyModel::slotInsertedIds( const QList< quint64 > &ids )
{
    Model * model = Model::instance();

    QList< quint64 > proxyIds;
    foreach( quint64 id, ids )
    {
        if ( model->matchesCurrentSearchTerm( model->rowForId( id ) ) )
            proxyIds << id;
    }

    if ( proxyIds.size() > 0 )
        emit( insertedIds( proxyIds ) );
}

void NavigatorFilterProxyModel::slotRemovedIds( const QList< quint64 > &ids )
{
    Model *model = Model::instance();

    QList<quint64> proxyIds;
    foreach( quint64 id, ids ) {
        const int row = model->rowForId( id );
        if ( row == -1 || model->matchesCurrentSearchTerm( row ) ) {
            proxyIds << id;
        }
    }

    if ( proxyIds.size() > 0 )
        emit removedIds( proxyIds );
}

Item::State NavigatorFilterProxyModel::stateOfRow( int row ) const
{
    return Model::instance()->stateOfRow( rowToSource( row ) );
}

Item::State NavigatorFilterProxyModel::stateOfId( quint64 id ) const
{
    return Model::instance()->stateOfId( id );
}

void NavigatorFilterProxyModel::setPassThrough( bool passThrough )
{
    m_passThrough = passThrough;

    //make sure to update model when mode changes ( as we might have ignored and
    //number of changes to the search term )
    invalidateFilter();
    emit( filterChanged() );
    emit( layoutChanged() );
}

int NavigatorFilterProxyModel::rowToSource( int row ) const
{
    QModelIndex index = this->index( row, 0 );
    QModelIndex sourceIndex = mapToSource( index );

    if ( !sourceIndex.isValid() )
        return -1;
    return sourceIndex.row();
}

int NavigatorFilterProxyModel::rowFromSource( int row ) const
{
    Model * model = Model::instance();
    QModelIndex sourceIndex = model->index( row, 0 );
    QModelIndex index = mapFromSource( sourceIndex );

    if ( !index.isValid() )
        return -1;
    return index.row();
}

bool NavigatorFilterProxyModel::rowExists( int row ) const
{

    QModelIndex index = this->index( row, 0 );
    return index.isValid();
}

void NavigatorFilterProxyModel::setActiveRow( int row )
{
    Model::instance()->setActiveRow( rowToSource( row ) );
}

Meta::TrackPtr NavigatorFilterProxyModel::trackAt(int row) const
{
    return Model::instance()->trackAt( rowToSource( row ) );
}

int NavigatorFilterProxyModel::find( const QString &searchTerm, int searchFields )
{
    return rowFromSource( Model::instance()->find( searchTerm, searchFields ) );
}

int NavigatorFilterProxyModel::findNext( const QString & searchTerm, int selectedRow, int searchFields )
{
    return rowFromSource( Model::instance()->findNext( searchTerm, selectedRow, searchFields ) );
}

int NavigatorFilterProxyModel::findPrevious( const QString & searchTerm, int selectedRow, int searchFields )
{
    return rowFromSource( Model::instance()->findPrevious( searchTerm, selectedRow, searchFields ) );
}

int NavigatorFilterProxyModel::totalLength() const
{
    return Model::instance()->totalLength();
}

void NavigatorFilterProxyModel::clearSearchTerm()
{
    Model::instance()->clearSearchTerm();
    
    if ( !m_passThrough )
    {
        invalidateFilter();
        emit( filterChanged() );
        emit( layoutChanged() );
    }
}

QString NavigatorFilterProxyModel::currentSearchTerm()
{
    return Model::instance()->currentSearchTerm();
}

int NavigatorFilterProxyModel::currentSearchFields()
{
    return Model::instance()->currentSearchFields();
}

QVariant NavigatorFilterProxyModel::data( const QModelIndex & index, int role ) const
{
     //HACK around incomplete index causing a crash...
    QModelIndex newIndex = this->index( index.row(), index.column() );

    QModelIndex sourceIndex = mapToSource( newIndex );
    return Model::instance()->data( sourceIndex, role );
}


Qt::DropActions NavigatorFilterProxyModel::supportedDropActions() const
{
    return Model::instance()->supportedDropActions();
}

Qt::ItemFlags NavigatorFilterProxyModel::flags( const QModelIndex &index ) const
{
    return Model::instance()->flags( index );
}

QStringList NavigatorFilterProxyModel::mimeTypes() const
{
    return Model::instance()->mimeTypes();
}

QMimeData * NavigatorFilterProxyModel::mimeData( const QModelIndexList &index ) const
{
    return Model::instance()->mimeData( index );
}

bool NavigatorFilterProxyModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
    return Model::instance()->dropMimeData( data, action, row, column, parent );
}

void NavigatorFilterProxyModel::setRowQueued( int row )
{
    Model::instance()->setRowQueued( rowToSource( row ) );
}

void NavigatorFilterProxyModel::setRowDequeued( int row )
{
    Model::instance()->setRowDequeued( rowToSource( row ) );
}

}

#include "NavigatorFilterProxyModel.moc"
