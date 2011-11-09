/***************************************************************************
 *   Copyright (c) 2008  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *             (c) 2008  Ian Monroe <imonroe@kde.org>                      *
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

#include "BookmarkModel.h"

#include "AmarokMimeData.h"
#include "AmarokUrl.h"
#include "BookmarkGroup.h"
#include "Debug.h"
#include "CollectionManager.h"
#include "SqlStorage.h"

#include <KIcon>

#include <QAbstractListModel>
#include <QListIterator>

#include <typeinfo>

static const int BOOKMARK_DB_VERSION = 4;
static const QString key("AMAROK_BOOKMARKS");

BookmarkModel * BookmarkModel::s_instance = 0;

BookmarkModel * BookmarkModel::instance()
{
    if ( s_instance == 0 )
        s_instance = new BookmarkModel();

    return s_instance;
}


BookmarkModel::BookmarkModel()
 : QAbstractItemModel()
{
    checkTables();

    m_root = BookmarkGroupPtr( new BookmarkGroup( "root", BookmarkGroupPtr() ) );
}


BookmarkModel::~BookmarkModel()
{
}

QVariant
BookmarkModel::data(const QModelIndex & index, int role) const
{
    
    if ( !index.isValid() )
        return QVariant();

    BookmarkViewItemPtr item =  m_viewItems.value( index.internalId() );

    if ( role == 0xf00d )
        return QVariant::fromValue( item );
    else if ( role == Qt::DisplayRole || role == Qt::EditRole )
        return item->name();
    else if (role == Qt::DecorationRole ) {

        if ( typeid( * item ) == typeid( BookmarkGroup ) )
            return QVariant( KIcon( "folder-amarok" ) );
        else if ( typeid( * item ) == typeid( AmarokUrl ) ) {
            AmarokUrl * url = static_cast<AmarokUrl *>( item.data() );
            if ( url->command() == "navigate" )
                return QVariant( KIcon( "flag-amarok" ) );
            else if ( url->command() == "play" )
                return QVariant( KIcon( "x-media-podcast-amarok" ) );
            else
                return QVariant( KIcon() );
        }
    }

    return QVariant();
}


QModelIndex
BookmarkModel::createIndex( int row, int column, BookmarkViewItemPtr item ) const
{
    quint32 index = qHash( item.data() );
    bool debugIt = false;
    if( m_viewItems.contains( index ) )
        debugIt = false;
    else
        m_viewItems[ index ] = item;
    QModelIndex ret = QAbstractItemModel::createIndex( row, column, index );
//    if( debugIt )
//        debug() << "created " << ret << " with " << ret.parent().internalId();
    return ret;
}

QModelIndex
BookmarkModel::index(int row, int column, const QModelIndex & parent) const
{
    //DEBUG_BLOCK

    //debug() << "row: " << row << ", column: " <<column;
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if ( !parent.isValid() )
    {

        int noOfGroups = m_root->childGroups().count();
        if ( row < noOfGroups )
        {
            return createIndex( row, column, BookmarkViewItemPtr::staticCast( m_root->childGroups().at( row ) ) );
        }
        else
        {
            //debug() << "Root playlist";
            return createIndex( row, column, BookmarkViewItemPtr::staticCast( m_root->childBookmarks().at( row - noOfGroups ) ) );
        }
    }
    else
    {
        BookmarkGroupPtr playlistGroup = BookmarkGroupPtr::staticCast( m_viewItems.value( parent.internalId() ) );
        int noOfGroups = playlistGroup->childGroups().count();

        if ( row < noOfGroups )
        {
            return createIndex( row, column, BookmarkViewItemPtr::staticCast( playlistGroup->childGroups().at(row) ) );
        }
        else
        {
            return createIndex( row, column, BookmarkViewItemPtr::staticCast( playlistGroup->childBookmarks().at(row - noOfGroups) ) );
        }
    }
}

QModelIndex
BookmarkModel::parent( const QModelIndex & index ) const
{
    //DEBUG_BLOCK

    if (!index.isValid())
        return QModelIndex();
    BookmarkViewItemPtr item = m_viewItems.value( index.internalId() );
    
    BookmarkGroupPtr parent = item->parent();

    //debug() << "parent: " << parent;

    if ( parent &&  parent->parent() )
    {
        int row = parent->parent()->childGroups().indexOf( parent );
        return createIndex( row , 0, BookmarkViewItemPtr::staticCast( parent ) );
    }
    else {
        return QModelIndex();
    }
}

int
BookmarkModel::rowCount( const QModelIndex & parent ) const
{
    //DEBUG_BLOCK

    if ( parent.column() > 0 ) {
      //  debug() << "baad collumn";
        return 0;
    }

    if (!parent.isValid()) {

        //debug() << "top level item has" << m_root->childCount();

        return m_root->childCount();

    }
    BookmarkViewItemPtr item = m_viewItems.value( parent.internalId() );
    //debug() << "row: " << parent.row();
    //debug() << "address: " << item;
    //debug() << "count: " << item->childCount();

    return item->childCount();
}

int
BookmarkModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 1;
}


Qt::ItemFlags
BookmarkModel::flags( const QModelIndex & index ) const
{
            
    if (!index.isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    BookmarkViewItemPtr item = BookmarkViewItemPtr::staticCast( m_viewItems.value( index.internalId() ) );

    if ( typeid( * item ) == typeid( BookmarkGroup ) )
    {
        debug() <<  "got a group with ItemIsDropEnabled";
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;
    }
    else
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
}

QVariant
BookmarkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch( section )
        {
            case 0: return i18n("Name");
            default: return QVariant();
        }
    }

    return QVariant();
}

bool BookmarkModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role != Qt::EditRole)
        return false;
    if ( index.column() != 0 )
        return false;

    BookmarkViewItemPtr item = m_viewItems.value( index.internalId() );

    item->rename( value.toString() );

    return true;

}

QStringList
BookmarkModel::mimeTypes() const
{
    DEBUG_BLOCK
    QStringList ret;
    ret << AmarokMimeData::BOOKMARKGROUP_MIME;
    ret << AmarokMimeData::AMAROKURL_MIME;
    return ret;
}

QMimeData*
BookmarkModel::mimeData( const QModelIndexList &indexes ) const
{
    DEBUG_BLOCK
    AmarokMimeData* mime = new AmarokMimeData();

    BookmarkGroupList groups;
    BookmarkList bookmarks;

    foreach( const QModelIndex &index, indexes ) {

        BookmarkViewItemPtr item = m_viewItems.value( index.internalId() );

        if ( typeid( * item ) == typeid( BookmarkGroup ) ) {
            BookmarkGroupPtr playlistGroup = BookmarkGroupPtr::staticCast( item );
            groups << playlistGroup;
        }
        else
        {
            AmarokUrlPtr bookmark = AmarokUrlPtr::dynamicCast( item );
            if( bookmark )
                bookmarks << bookmark;
        }
    }

    debug() << "adding " << groups.count() << " groups and " << bookmarks.count() << " bookmarks";

    mime->setBookmarkGroups( groups );
    mime->setBookmarks( bookmarks );

    return mime;
}


bool
BookmarkModel::dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent ) //reimplemented
{
    DEBUG_BLOCK
            
    Q_UNUSED( column ); 
    Q_UNUSED( row );
    if( action == Qt::IgnoreAction )
        return true;

    BookmarkGroupPtr parentGroup;
    if ( !parent.isValid() )
    {
        parentGroup = m_root;
    }
    else
    {
        parentGroup = BookmarkGroupPtr::staticCast( m_viewItems.value( parent.internalId() ) );
    }

    if( data->hasFormat( AmarokMimeData::BOOKMARKGROUP_MIME ) )
    {
        debug() << "Found playlist group mime type";

        const AmarokMimeData* bookmarkGroupDrag = dynamic_cast<const AmarokMimeData*>( data );
        if( bookmarkGroupDrag )
        {

            BookmarkGroupList groups = bookmarkGroupDrag->bookmarkGroups();

            foreach( BookmarkGroupPtr group, groups ) {
                group->reparent( parentGroup );
            }

            reloadFromDb();

            return true;
        }
    }
    else if( data->hasFormat( AmarokMimeData::AMAROKURL_MIME ) )
    {
        debug() << "Found amarokurl mime type";

        const AmarokMimeData* dragList = dynamic_cast<const AmarokMimeData*>( data );
        if( dragList )
        {
            BookmarkList bookmarks = dragList->bookmarks();

            foreach( AmarokUrlPtr bookmarkPtr, bookmarks ) {
                bookmarkPtr->reparent( parentGroup );
            }

            reloadFromDb();

            return true;
        }
    }

    return false;
}

void BookmarkModel::createTables()
{
    DEBUG_BLOCK

    SqlStorage *sqlStorage = CollectionManager::instance()->sqlStorage();
    sqlStorage->query( QString( "CREATE TABLE bookmark_groups ("
            " id " + sqlStorage->idType() +
            ", parent_id INTEGER"
            ", name " + sqlStorage->textColumnType() +
            ", description " + sqlStorage->textColumnType() +
            ", custom " + sqlStorage->textColumnType() + " );" ) );

    sqlStorage->query( QString( "CREATE TABLE bookmarks ("
            " id " + sqlStorage->idType() +
            ", parent_id INTEGER"
            ", name " + sqlStorage->textColumnType() +
            ", url " + sqlStorage->exactTextColumnType() +
            ", description " + sqlStorage->exactTextColumnType() +
            ", custom " + sqlStorage->textColumnType() + " );" ) );

}

void BookmarkModel::deleteTables()
{

    DEBUG_BLOCK
    
    SqlStorage *sqlStorage = CollectionManager::instance()->sqlStorage();

    sqlStorage->query( "DROP TABLE bookmark_groups;" );
    sqlStorage->query( "DROP TABLE bookmarks;" );

}

void BookmarkModel::checkTables()
{

    DEBUG_BLOCK
            
    SqlStorage *sqlStorage = CollectionManager::instance()->sqlStorage();
    QStringList values = sqlStorage->query( QString("SELECT version FROM admin WHERE component = '%1';").arg(sqlStorage->escape( key ) ) );
    if( values.isEmpty() )
    {
        //debug() << "creating Playlist Tables";
        createTables();
        sqlStorage->query( "INSERT INTO admin(component,version) "
                "VALUES('" + key + "'," + QString::number( BOOKMARK_DB_VERSION ) + ");" );
    }
    else if ( values.at( 0 ).toInt() < 4 )
    {
        upgradeTables( values.at( 0 ).toInt() );
        sqlStorage->query( "UPDATE admin SET version=" + QString::number( BOOKMARK_DB_VERSION ) + " WHERE component=" + key + ';' );
    }
}

void
BookmarkModel::reloadFromDb()
{
    DEBUG_BLOCK;
    reset();
    m_root->clear();
}

void
BookmarkModel::editBookmark( int id )
{

  //for now, assume that the newly added playlist is in the top level:
    int row = m_root->childGroups().count() - 1;
    foreach ( AmarokUrlPtr bookmark, m_root->childBookmarks() ) {
        row++;
        if ( bookmark->id() == id ) {
            emit editIndex( createIndex( row , 0, BookmarkViewItemPtr::staticCast( bookmark ) ) );
        }
    }
}

void
BookmarkModel::createNewGroup()
{
    DEBUG_BLOCK

    BookmarkGroup * group = new BookmarkGroup( i18n("New Group"), m_root );
    group->save();
    int id = group->id();
    delete group;

    reloadFromDb();

    int row = 0;
    foreach ( BookmarkGroupPtr childGroup, m_root->childGroups() ) {
        if ( childGroup->id() == id )
        {
            debug() << "emmiting edit for " << childGroup->name() << " id " << childGroup->id() << " in row " << row;
            emit editIndex( createIndex( row , 0, BookmarkViewItemPtr::staticCast( childGroup ) ) );
        }
        row++;
    }

}

void BookmarkModel::upgradeTables( int from )
{
    SqlStorage *sqlStorage = CollectionManager::instance()->sqlStorage();
    
    if ( from == 2 ) {
        sqlStorage->query( "ALTER TABLE bookmarks ADD custom " + sqlStorage->textColumnType() + ';' );
    }

    sqlStorage->query( "ALTER TABLE bookmark_groups ADD custom " + sqlStorage->textColumnType() + ';' );
} 





#include "BookmarkModel.moc"
