/******************************************************************************
 * Copyright (c) 2007 Alexandre Pereira de Oliveira <aleprj@gmail.com>        *
 *           (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>  *
 *           (c) 2007 Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>          *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#define DEBUG_PREFIX "CollectionTreeItemModelBase"

#include "CollectionTreeItemModelBase.h"

#include "Amarok.h"
#include "AmarokMimeData.h"
#include "Collection.h"
#include "CollectionManager.h"
#include "CollectionTreeItem.h"
#include "CollectionTreeView.h"
#include "Debug.h"
#include "Expression.h"
#include "MetaConstants.h"
#include "QueryMaker.h"
#include "amarokconfig.h"
#include "meta/capabilities/EditCapability.h"

#include <KIcon>
#include <KIconLoader>
#include <KLocale>
#include <KStandardDirs>
#include <QPixmap>
#include <QTimeLine>

using namespace Meta;

inline uint qHash( const Meta::DataPtr &data )
{
    return qHash( data.data() );
}


CollectionTreeItemModelBase::CollectionTreeItemModelBase( )
    : QAbstractItemModel()
    , m_rootItem( 0 )
    , d( new Private )
    , m_animFrame( 0 )
    , m_loading1( QPixmap( KStandardDirs::locate("data", "amarok/images/loading1.png" ) ) )
    , m_loading2( QPixmap( KStandardDirs::locate("data", "amarok/images/loading2.png" ) ) )
    , m_currentAnimPixmap( m_loading1 )
{
    m_timeLine = new QTimeLine( 10000, this );
    m_timeLine->setFrameRange( 0, 20 );
    m_timeLine->setLoopCount ( 0 );
    connect( m_timeLine, SIGNAL( frameChanged( int ) ), this, SLOT( loadingAnimationTick() ) );
}

CollectionTreeItemModelBase::~CollectionTreeItemModelBase()
{
    delete m_rootItem;
    delete d;
}

Qt::ItemFlags CollectionTreeItemModelBase::flags(const QModelIndex & index) const
{
    if ( !index.isValid() )
        return Qt::ItemIsEnabled;

    Qt::ItemFlags flags;
    flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
    return flags;

}

bool
CollectionTreeItemModelBase::setData( const QModelIndex &index, const QVariant &value, int role )
{
    Q_UNUSED( role )

    if( !index.isValid() )
        return false;
    CollectionTreeItem *item = static_cast<CollectionTreeItem*>( index.internalPointer() );

    Meta::DataPtr data = item->data();

    if( Meta::TrackPtr track = Meta::TrackPtr::dynamicCast( data ) )
    {
        if( !track->hasCapabilityInterface( Meta::Capability::Editable ) )
            return false;
        Meta::EditCapability *ec = track->create<Meta::EditCapability>();
        if( ec )
        {
            ec->setTitle( value.toString() );
            emit dataChanged( index, index );
            delete ec;
            return true;
        }
    }
    /*else
    {
        QModelIndex in = index;
        while( in.model()->hasChildren( in ) )
        {
            int rows = in.model()->rowCount( in );
            for( int i = 0; i < rows; i++ )
            {
                setData( in.child( i, 1 ), value, role );
                in = in.sibling( in.row() + 1, in.column() );
            }
        }
    }*/
    else if( Meta::AlbumPtr album = Meta::AlbumPtr::dynamicCast( data ) )
    {
        Meta::TrackList tracks = album->tracks();
        if( tracks.size() > 0 )
        {
            foreach( Meta::TrackPtr track, tracks )
            {
                Meta::EditCapability *ec = track->create<Meta::EditCapability>();
                if( ec )
                    ec->setAlbum( value.toString() );
                delete ec;
            }
            emit dataChanged( index, index );
            return true;
        }
    }
    else if( Meta::ArtistPtr artist = Meta::ArtistPtr::dynamicCast( data ) )
    {
        Meta::TrackList tracks = artist->tracks();
        if( tracks.size() > 0 )
        {
            foreach( Meta::TrackPtr track, tracks )
            {
                Meta::EditCapability *ec = track->create<Meta::EditCapability>();
                if( ec )
                    ec->setArtist( value.toString() );
                delete ec;
            }
            emit dataChanged( index, index );
            return true;
        }
    }
    else if( Meta::GenrePtr genre = Meta::GenrePtr::dynamicCast( data ) )
    {
        Meta::TrackList tracks = genre->tracks();
        if( tracks.size() > 0 )
        {
            foreach( Meta::TrackPtr track, tracks )
            {
                Meta::EditCapability *ec = track->create<Meta::EditCapability>();
                if( ec )
                    ec->setGenre( value.toString() );
                delete ec;
            }
            emit dataChanged( index, index );
            return true;
        }
    }
    else if( Meta::YearPtr year = Meta::YearPtr::dynamicCast( data ) )
    {
        Meta::TrackList tracks = year->tracks();
        if( tracks.size() > 0 )
        {
            foreach( Meta::TrackPtr track, tracks )
            {
                Meta::EditCapability *ec = track->create<Meta::EditCapability>();
                if( ec )
                    ec->setYear( value.toString() );
                delete ec;
            }
            emit dataChanged( index, index );
            return true;
        }
    }
    else if( Meta::ComposerPtr composer = Meta::ComposerPtr::dynamicCast( data ) )
    {
        Meta::TrackList tracks = composer->tracks();
        if( tracks.size() > 0 )
        {
            foreach( Meta::TrackPtr track, tracks )
            {
                Meta::EditCapability *ec = track->create<Meta::EditCapability>();
                if( ec )
                    ec->setComposer( value.toString() );
                delete ec;
            }
            emit dataChanged( index, index );
            return true;
        }
    }
    return false;
}

QVariant
CollectionTreeItemModelBase::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if (section == 0)
            return m_headerText;
    }
    return QVariant();
}

QModelIndex
CollectionTreeItemModelBase::index(int row, int column, const QModelIndex & parent) const
{
    CollectionTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<CollectionTreeItem*>(parent.internalPointer());

   //if ( parentItem->childrenLoaded() )
   //{
        CollectionTreeItem *childItem = parentItem->child(row);
        if( childItem )
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    //}
    //else
    //   return QModelIndex();
}

QModelIndex
CollectionTreeItemModelBase::parent(const QModelIndex & index) const
{
     if( !index.isValid() )
         return QModelIndex();

     CollectionTreeItem *childItem = static_cast<CollectionTreeItem*>(index.internalPointer());
     CollectionTreeItem *parentItem = childItem->parent();

     if ( (parentItem == m_rootItem) || !parentItem )
         return QModelIndex();

     return createIndex(parentItem->row(), 0, parentItem);
}

int
CollectionTreeItemModelBase::rowCount(const QModelIndex & parent) const
{
    CollectionTreeItem *parentItem;

    if( !parent.isValid() )
        parentItem = m_rootItem;
    else
        parentItem = static_cast<CollectionTreeItem*>(parent.internalPointer());

    if( parentItem->childrenLoaded() )
        return parentItem->childCount();
    return 0;
}

int CollectionTreeItemModelBase::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED( parent )
    return 1;
}

QStringList
CollectionTreeItemModelBase::mimeTypes() const
{
    QStringList types;
    types << AmarokMimeData::TRACK_MIME;
    return types;
}

QMimeData*
CollectionTreeItemModelBase::mimeData(const QModelIndexList & indices) const
{
    if ( indices.isEmpty() )
        return 0;

    QList<CollectionTreeItem*> items;

    foreach( const QModelIndex &index, indices )
    {
        if( index.isValid() )
            items << static_cast<CollectionTreeItem*>( index.internalPointer() );
    }

    return mimeData( items );
}

QMimeData*
CollectionTreeItemModelBase::mimeData(const QList<CollectionTreeItem*> & items) const
{
    if ( items.isEmpty() )
        return 0;

    Meta::TrackList tracks;
    QList<QueryMaker*> queries;

    foreach( CollectionTreeItem *item, items )
    {
        if( item->allDescendentTracksLoaded() ) {
            tracks << item->descendentTracks();
        }
        else
        {
            QueryMaker *qm = item->queryMaker();
            CollectionTreeItem *tmpItem = item;
            while( tmpItem->isDataItem() )
            {
                if( tmpItem->data() )
                    qm->addMatch( tmpItem->data() );
                else
                    qm->setAlbumQueryMode( QueryMaker::OnlyCompilations );
                tmpItem = tmpItem->parent();
            }
            addFilters( qm );
            queries.append( qm );
        }
    }

    qStableSort( tracks.begin(), tracks.end(), Meta::Track::lessThan );

    AmarokMimeData *mimeData = new AmarokMimeData();
    mimeData->setTracks( tracks );
    mimeData->setQueryMakers( queries );
    mimeData->startQueries();
    return mimeData;
}

QPixmap
CollectionTreeItemModelBase::iconForLevel(int level) const
{
    QString icon;
    switch( m_levelType[level] )
    {
        case CategoryId::Album :
//             icon = "view-media-album-amarok"; // Doesn't exist..
            icon = "media-optical-amarok";
            break;
        case CategoryId::Artist :
            icon = "view-media-artist-amarok";
            break;
        case CategoryId::Composer :
            icon = "view-media-artist-amarok";
            break;
        case CategoryId::Genre :
            icon = "favorite-genres-amarok";
            break;
        case CategoryId::Year :
            icon = "clock";
            break;
    }
    return KIconLoader::global()->loadIcon( icon, KIconLoader::Toolbar, KIconLoader::SizeSmall );
}

void CollectionTreeItemModelBase::listForLevel(int level, QueryMaker * qm, CollectionTreeItem * parent) const
{
    //DEBUG_BLOCK
    if ( qm && parent )
    {
        //this check should not hurt anyone... needs to check if single... needs it
        for( QMapIterator<QueryMaker*, CollectionTreeItem*> iter( d->m_childQueries ); iter.hasNext(); )
        {
            if( iter.next().value() == parent )
                return;             //we are already querying for children of parent
        }
        if ( level > m_levelType.count() )
            return;

        if ( level == m_levelType.count() )
            qm->setQueryType( QueryMaker::Track );
        else
        {
            switch( m_levelType[level] )
            {
                case CategoryId::Album :
                    qm->setQueryType( QueryMaker::Album );
                    //restrict query to normal albums if the previous level
                    //was the artist category. in that case we handle compilations below
                    if( level > 0 && m_levelType[level-1] == CategoryId::Artist )
                    {
                        qm->setAlbumQueryMode( QueryMaker::OnlyNormalAlbums );
                    }
                    break;
                case CategoryId::Artist :
                    qm->setQueryType( QueryMaker::Artist );
                    //handle compilations only if the next level ist CategoryId::Album
                    if( level + 1 < m_levelType.count() && m_levelType[level+1] == CategoryId::Album )
                    {
                        handleCompilations( parent );
                        qm->setAlbumQueryMode( QueryMaker::OnlyNormalAlbums );
                    }
                    break;
                case CategoryId::Composer:
                    qm->setQueryType( QueryMaker::Composer );
                    break;
                case CategoryId::Genre:
                    qm->setQueryType( QueryMaker::Genre );
                    break;
                case CategoryId::Year:
                    qm->setQueryType( QueryMaker::Year );
                    break;
                default : //TODO handle error condition. return tracks?
                    break;
            }
        }
        CollectionTreeItem *tmpItem = parent;
        while( tmpItem->isDataItem() )
        {
            //ignore Various artists node (whichh will not have a data pointer
            if( tmpItem->data() )
                qm->addMatch( tmpItem->data() );

            tmpItem = tmpItem->parent();
        }
        addFilters( qm );
        qm->setReturnResultAsDataPtrs( true );
        connect( qm, SIGNAL( newResultReady( QString, Meta::DataList ) ), SLOT( newResultReady( QString, Meta::DataList ) ), Qt::QueuedConnection );
        connect( qm, SIGNAL( queryDone() ), SLOT( queryDone() ), Qt::QueuedConnection );
        d->m_childQueries.insert( qm, parent );
        qm->run();

        //start animation
        if( ( m_timeLine->state() != QTimeLine::Running ) && ( parent != m_rootItem ) )
            m_timeLine->start();
    }
}

void
CollectionTreeItemModelBase::addFilters( QueryMaker * qm ) const
{
    int validFilters = qm->validFilterMask();

    ParsedExpression parsed = ExpressionParser::parse ( m_currentFilter );
    foreach( const or_list &orList, parsed )
    {
        qm->beginOr();

        foreach ( const expression_element &elem, orList )
        {
#define ADD_OR_EXCLUDE_FILTER( VALUE, FILTER, MATCHBEGIN, MATCHEND ) \
            if( elem.negate ) \
                qm->excludeFilter( VALUE, FILTER, MATCHBEGIN, MATCHEND ); \
            else \
                qm->addFilter( VALUE, FILTER, MATCHBEGIN, MATCHEND );
#define ADD_OR_EXCLUDE_NUMBER_FILTER( VALUE, FILTER, COMPARE ) \
            if( elem.negate ) \
                qm->excludeNumberFilter( VALUE, FILTER, COMPARE ); \
            else \
                qm->addNumberFilter( VALUE, FILTER, COMPARE );
            if( elem.negate )
                qm->beginAnd();
            else
                qm->beginOr();
            debug() << "field:" << elem.field << "negate?:" << elem.negate << "text:" << elem.text;
            if ( elem.field.isEmpty() )
            {
                foreach ( int level, m_levelType )
                {
                    qint64 value;
                    switch ( level )
                    {
                        case CategoryId::Album:
                            if ( ( validFilters & QueryMaker::AlbumFilter ) == 0 ) continue;
                            value = Meta::valAlbum;
                            break;
                        case CategoryId::Artist:
                            if ( ( validFilters & QueryMaker::ArtistFilter ) == 0 ) continue;
                            value = Meta::valArtist;
                            break;
                        case CategoryId::Composer:
                            if ( ( validFilters & QueryMaker::ComposerFilter ) == 0 ) continue;
                            value = Meta::valComposer;
                            break;
                        case CategoryId::Genre:
                            if ( ( validFilters & QueryMaker::GenreFilter ) == 0 ) continue;
                            value = Meta::valGenre;
                            break;
                        case CategoryId::Year:
                            if ( ( validFilters & QueryMaker::YearFilter ) == 0 ) continue;
                            value = Meta::valYear;
                            break;
                        default:
                            value = -1;
                            break;
                    }
                    qm->beginOr();
                    ADD_OR_EXCLUDE_FILTER( value, elem.text, false, false );
                    qm->endAndOr();
                }

                //always add track filter ( if supported..)
                if ( ( validFilters & QueryMaker::TitleFilter ) != 0 ) {
                    qm->beginOr();
                    ADD_OR_EXCLUDE_FILTER( Meta::valTitle, elem.text, false, false ); //always filter for track title too
                    qm->endAndOr();
                }
            }
            else
            {
                //get field values based on name
                QString lcField = elem.field.toLower();
                QueryMaker::NumberComparison compare = QueryMaker::Equals;
                switch( elem.match )
                {
                    case expression_element::More:
                        compare = QueryMaker::GreaterThan;
                        break;
                    case expression_element::Less:
                        compare = QueryMaker::LessThan;
                        break;
                    case expression_element::Contains:
                        compare = QueryMaker::Equals;
                        break;
                }

                if ( lcField.compare( "album", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "album" ), Qt::CaseInsensitive ) == 0 )
                {
                    if ( ( validFilters & QueryMaker::AlbumFilter ) == 0 ) continue;
                    ADD_OR_EXCLUDE_FILTER( Meta::valAlbum, elem.text, false, false );
                }
                else if ( lcField.compare( "artist", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "artist" ), Qt::CaseInsensitive ) == 0 )
                {
                    if ( ( validFilters & QueryMaker::ArtistFilter ) == 0 ) continue;
                    ADD_OR_EXCLUDE_FILTER( Meta::valArtist, elem.text, false, false );
                }
                else if ( lcField.compare( "genre", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "genre" ), Qt::CaseInsensitive ) == 0)
                {
                    if ( ( validFilters & QueryMaker::GenreFilter ) == 0 ) continue;
                    ADD_OR_EXCLUDE_FILTER( Meta::valGenre, elem.text, false, false );
                }
                else if ( lcField.compare( "composer", Qt::CaseInsensitive ) == 0|| lcField.compare( i18n( "composer" ), Qt::CaseInsensitive ) == 0 )
                {
                    if ( ( validFilters & QueryMaker::ComposerFilter ) == 0 ) continue;
                    ADD_OR_EXCLUDE_FILTER( Meta::valComposer, elem.text, false, false );
                }
                else if ( lcField.compare( "year", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "year" ), Qt::CaseInsensitive ) == 0)
                {
                    if ( ( validFilters & QueryMaker::YearFilter ) == 0 ) continue;
                    ADD_OR_EXCLUDE_FILTER( Meta::valYear, elem.text, false, false );
                }
                else if( lcField.compare( "comment", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "comment" ), Qt::CaseInsensitive ) == 0 )
                {
                    ADD_OR_EXCLUDE_FILTER( Meta::valYear, elem.text, false, false );
                }
                else if( lcField.compare( "rating", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "rating" ), Qt::CaseInsensitive ) == 0 )
                {
                    ADD_OR_EXCLUDE_NUMBER_FILTER( Meta::valRating, elem.text.toInt(), compare );
                }
                else if( lcField.compare( "score", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "score" ), Qt::CaseInsensitive ) == 0 )
                {
                    ADD_OR_EXCLUDE_NUMBER_FILTER( Meta::valScore, elem.text.toInt(), compare );
                }
                else if( lcField.compare( "playcount", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "playcount" ), Qt::CaseInsensitive ) == 0 )
                {
                    ADD_OR_EXCLUDE_NUMBER_FILTER( Meta::valPlaycount, elem.text.toInt(), compare );
                }
                else if( lcField.compare( "length", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "length" ), Qt::CaseInsensitive ) == 0 )
                {
                    ADD_OR_EXCLUDE_NUMBER_FILTER( Meta::valLength, elem.text.toInt(), compare );
                }
                else if( lcField.compare( "discnumber", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "discnumber" ), Qt::CaseInsensitive ) == 0 )
                {
                    ADD_OR_EXCLUDE_NUMBER_FILTER( Meta::valDiscNr, elem.text.toInt(), compare );
                }
                else if( lcField.compare( "tracknumber", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "tracknumber" ), Qt::CaseInsensitive ) == 0 )
                {
                    ADD_OR_EXCLUDE_NUMBER_FILTER( Meta::valTrackNr, elem.text.toInt(), compare );
                }
                else if( lcField.compare( "added", Qt::CaseInsensitive ) == 0 || lcField.compare( i18n( "added" ), Qt::CaseInsensitive ) == 0 )
                {
                    if( compare == QueryMaker::Equals ) // just do some basic string matching
                    {
                        QDateTime curTime = QDateTime::currentDateTime();
                        uint dateCutOff = 0;
                        if( ( elem.text.compare( "today", Qt::CaseInsensitive ) == 0 ) || ( elem.text.compare( i18n( "today" ), Qt::CaseInsensitive ) == 0 ) )
                            dateCutOff = curTime.addDays( -1 ).toTime_t();
                        else if( ( elem.text.compare( "last week", Qt::CaseInsensitive ) == 0 ) || ( elem.text.compare( i18n( "last week" ), Qt::CaseInsensitive ) == 0 ) )
                            dateCutOff = curTime.addDays( -7 ).toTime_t();
                        else if( ( elem.text.compare( "last month", Qt::CaseInsensitive ) == 0 ) || ( elem.text.compare( i18n( "last month" ), Qt::CaseInsensitive ) == 0 ) )
                            dateCutOff = curTime.addMonths( -1 ).toTime_t();
                        else if( ( elem.text.compare( "two months ago", Qt::CaseInsensitive ) == 0 ) || ( elem.text.compare( i18n( "two months ago" ), Qt::CaseInsensitive ) == 0 ) )
                            dateCutOff = curTime.addMonths( -2 ).toTime_t();
                        else if( ( elem.text.compare( "three months ago", Qt::CaseInsensitive ) == 0 ) || ( elem.text.compare( i18n( "three months ago" ), Qt::CaseInsensitive ) == 0 ) )
                            dateCutOff = curTime.addMonths( -3 ).toTime_t();

                        if( dateCutOff > 0 )
                        {
                            ADD_OR_EXCLUDE_NUMBER_FILTER( Meta::valCreateDate, dateCutOff, QueryMaker::GreaterThan );
                        }
                    }
                    else if( compare == QueryMaker::LessThan ) // parse a "#m#d" (discoverability == 0, but without a GUI, how to do it?)
                    {
                        int months = 0, weeks = 0, days = 0;
                        QString tmp;
                        for( int i = 0; i < elem.text.length(); i++ )
                        {
                            QChar c = elem.text.at( i );
                            if( c.isNumber() )
                                tmp += c;
                            else if( c == 'm' )
                            {
                                months = 0 - QString( tmp ).toInt();
                                tmp = "";
                            } else if( c == 'w' )
                            {
                                weeks = 0 - 7 * QString( tmp ).toInt();
                                tmp = "";
                            } else if( c == 'd' )
                            {
                                days = 0 - QString( tmp ).toInt();
                                break;
                            }
                        }
                        ADD_OR_EXCLUDE_NUMBER_FILTER( Meta::valCreateDate, QDateTime::currentDateTime().addMonths( months ).addDays( weeks ).addDays( days ).toTime_t(), QueryMaker::GreaterThan );
                    }
                }
            }
            qm->endAndOr();
#undef ADD_OR_EXCLUDE_FILTER
#undef ADD_OR_EXCLUDE_NUMBER_FILTER
        }
        qm->endAndOr();
    }
}

void
CollectionTreeItemModelBase::queryDone()
{
    QueryMaker *qm = qobject_cast<QueryMaker*>( sender() );
    if( !qm )
        return;

    CollectionTreeItem* item = d->m_childQueries.contains( qm ) ? d->m_childQueries.take( qm ) : d->m_compilationQueries.take( qm );

    //reset icon for this item
    if( item && item != m_rootItem )
        emit ( dataChanged ( createIndex(item->row(), 0, item), createIndex(item->row(), 0, item) ) );

    //stop timer if there are no more animations active
    if( d->m_childQueries.count() == 0 /*&& d->m_compilationQueries.count() == 0 */ )
        m_timeLine->stop();
    qm->deleteLater();
}

void
CollectionTreeItemModelBase::newResultReady(const QString & collectionId, Meta::DataList data)
{
    Q_UNUSED( collectionId )

    if ( data.count() == 0 )
        return;

    //if we are expanding an item, we'll find the sender in m_childQueries
    //otherwise we are filtering all collections
    QueryMaker *qm = qobject_cast<QueryMaker*>( sender() );
    if( !qm )
        return;

    if( d->m_childQueries.contains( qm ) )
    {
        CollectionTreeItem *parent = d->m_childQueries.value( qm );
        QModelIndex parentIndex;
        if( parent ) {
            if( parent == m_rootItem ) // will never happen in CollectionTreeItemModel
                parentIndex = QModelIndex();
            else
                parentIndex = createIndex( parent->row(), 0, parent );

            //add new rows after existing ones here (which means all artists nodes
            //will be inserted after the "Various Artists" node
            beginInsertRows( parentIndex, parent->childCount(), parent->childCount() + data.count()-1 );
            populateChildren( data, parent );
            endInsertRows();

            for( int count = parent->childCount(), i = 0; i < count; ++i )
            {
                CollectionTreeItem *item = parent->child( i );
                if ( m_expandedItems.contains( item->data() ) ) //item will always be a data item
                {
                    listForLevel( item->level(), item->queryMaker(), item );
                }
            }

            if ( parent->isDataItem() )
            {
                if ( m_expandedItems.contains( parent->data() ) )
                    emit expandIndex( parentIndex );
                else
                    //simply insert the item, nothing will change if it is already in the set
                    m_expandedItems.insert( parent->data() );
            }
            else
            {
                m_expandedCollections.insert( parent->parentCollection() );
            }
        }
    }
    else if( d->m_compilationQueries.contains( qm ) )
    {
        CollectionTreeItem *parent = d->m_compilationQueries.value( qm );
        QModelIndex parentIndex;
        if( parent ) {
            if (parent == m_rootItem ) // will never happen in CollectionTreeItemModel
            {
                parentIndex = QModelIndex();
            }
            else
            {
                parentIndex = createIndex( parent->row(), 0, parent );
            }
            //we only insert the "Various Artists" node
            beginInsertRows( parentIndex, 0, 0 );
            CollectionTreeItem *vaItem = new CollectionTreeItem( data, parent );
            endInsertRows();

            CollectionTreeItem *tmp = parent;
            while( tmp->isDataItem() )
                tmp = tmp->parent();

            if( m_expandedVariousArtistsNodes.contains( tmp->parentCollection() ) )
            {
                debug() << "Expand vanode for collection " << collectionId;
                QModelIndex vanode = createIndex( 0, 0, vaItem ); //we've just inserted the vaItem at row 0
                emit expandIndex( vanode );
            }

        }
    }
}

void
CollectionTreeItemModelBase::populateChildren(const DataList & dataList, CollectionTreeItem * parent) const
{
    foreach( Meta::DataPtr data, dataList )
        new CollectionTreeItem( data, parent );
    parent->setChildrenLoaded( true );
}

void
CollectionTreeItemModelBase::updateHeaderText()
{
    m_headerText.clear();
    for( int i=0; i< m_levelType.count(); ++i )
        m_headerText += nameForLevel( i ) + " / ";

    m_headerText.chop( 3 );
}

QString
CollectionTreeItemModelBase::nameForLevel(int level) const
{
    switch( m_levelType[level] )
    {
        case CategoryId::Album      : return AmarokConfig::showYears() ? i18n( "Year - Album" ) : i18n( "Album" );
        case CategoryId::Artist     : return i18n( "Artist" );
        case CategoryId::Composer   : return i18n( "Composer" );
        case CategoryId::Genre      : return i18n( "Genre" );
        case CategoryId::Year       : return i18n( "Year" );

        default: return QString();
    }
}

void
CollectionTreeItemModelBase::handleCompilations( CollectionTreeItem *parent ) const
{
    //this method will be called when we retrieve a list of artists from the database.
    //we have to query for all compilations, and then add a "Various Artists" node if at least
    //one compilation exists
    QueryMaker *qm = parent->queryMaker();
    qm->setAlbumQueryMode( QueryMaker::OnlyCompilations );
    qm->setQueryType( QueryMaker::Album );
    CollectionTreeItem *tmpItem = parent;
    while( tmpItem->isDataItem()  )
    {
        //ignore Various artists node (which will not have a data pointer)
        if( tmpItem->data() )
            qm->addMatch( tmpItem->data() );
        tmpItem = tmpItem->parent();
    }
    addFilters( qm );
    qm->setReturnResultAsDataPtrs( true );
    connect( qm, SIGNAL( newResultReady( QString, Meta::DataList ) ), SLOT( newResultReady( QString, Meta::DataList ) ), Qt::QueuedConnection );
    connect( qm, SIGNAL( queryDone() ), SLOT( queryDone() ), Qt::QueuedConnection );
    d->m_compilationQueries.insert( qm, parent );
    qm->run();
}

void CollectionTreeItemModelBase::loadingAnimationTick()
{
    if ( m_animFrame == 0 )
        m_currentAnimPixmap = m_loading2;
    else
        m_currentAnimPixmap = m_loading1;

    m_animFrame = 1 - m_animFrame;

    //trigger an update of all items being populated at the moment;
    QList<CollectionTreeItem* > items = d->m_childQueries.values();

    foreach ( CollectionTreeItem* item, items )
        emit ( dataChanged ( createIndex(item->row(), 0, item), createIndex(item->row(), 0, item) ) );
}

void
CollectionTreeItemModelBase::setCurrentFilter( const QString &filter )
{
    m_currentFilter = filter;
}

void
CollectionTreeItemModelBase::slotFilter()
{
    if ( isQuerying() )
        return; // we are already busy, do not try to change filters in the middle of everything as that will cause crashes

    filterChildren();
    reset();
    if ( !m_expandedCollections.isEmpty() )
    {
        foreach( Amarok::Collection *expanded, m_expandedCollections )
        {
            CollectionTreeItem *expandedItem = d->m_collections.value( expanded->collectionId() ).second;
            if( expandedItem )
                emit expandIndex( createIndex( expandedItem->row(), 0, expandedItem ) );
        }
    }
}

void
CollectionTreeItemModelBase::slotCollapsed( const QModelIndex &index )
{
    if ( index.isValid() )      //probably unnecessary, but let's be safe
    {
        CollectionTreeItem *item = static_cast<CollectionTreeItem*>( index.internalPointer() );
        if ( item->isDataItem() && item->data() )
        {
            m_expandedItems.remove( item->data() );
        }
        else if( item->isDataItem() && !item->data() ) //Various artists nodes
        {
            CollectionTreeItem *tmp = item->parent();
            while( tmp->isDataItem() )
                tmp = tmp->parent();

            m_expandedVariousArtistsNodes.remove( tmp->parentCollection() );
        }
        else
        {
            m_expandedCollections.remove( item->parentCollection() );
        }
    }
}

void
CollectionTreeItemModelBase::slotExpanded( const QModelIndex &index )
{
    if( index.isValid() )
    {
        CollectionTreeItem *item = static_cast<CollectionTreeItem*>( index.internalPointer() );
        //we are really only interested in the various artists nodes here.
        //we have to remember whether the user expanded a various artists node or not.
        //otherwise we won't be able to automatically expand the various artists node after filtering again
        //there is exactly one various artists node per collection, so use the collection to store that information
        if( item->isDataItem() && !item->data() )
        {
            CollectionTreeItem *tmp = item->parent();
            while( tmp->isDataItem() )
                tmp = tmp->parent();

            if( tmp->parentCollection() )
                debug() << "VA node for collection " << tmp->parentCollection()->collectionId() << " expanded";
            m_expandedVariousArtistsNodes.insert( tmp->parentCollection() );
        }
    }
}

void CollectionTreeItemModelBase::update()
{
    reset();
}

bool CollectionTreeItemModelBase::isQuerying() const
{
    return !( d->m_childQueries.isEmpty() && d->m_compilationQueries.isEmpty() );
}


#include "CollectionTreeItemModelBase.moc"

