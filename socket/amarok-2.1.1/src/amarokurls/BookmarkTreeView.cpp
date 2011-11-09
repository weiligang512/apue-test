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
 
#include "BookmarkTreeView.h"

#include "BookmarkModel.h"
#include "context/popupdropper/libpud/PopupDropperAction.h"
#include "PaletteHandler.h"
#include "AmarokUrl.h"
#include "BookmarkGroup.h"
#include "SvgHandler.h"
#include "statusbar/StatusBar.h"

#include <KAction>
#include <KMenu>

#include <QFrame>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QModelIndex>
#include <QPoint>

#include <typeinfo>

BookmarkTreeView::BookmarkTreeView( QWidget *parent )
    : Amarok::PrettyTreeView( parent )
    , m_loadAction( 0 )
    , m_deleteAction( 0 )
    , m_renameAction( 0 )
    , m_addGroupAction( 0 )
{

    setEditTriggers( QAbstractItemView::NoEditTriggers );

    setSelectionMode( QAbstractItemView::ExtendedSelection );
    The::paletteHandler()->updateItemView( this );

    header()->hide();
    setFrameShape( QFrame::NoFrame );

    //Give line edits a solid background color as any edit delegates will otherwise inherit the transparent base color,
    //which is bad as the line edit is drawn on top of the original name, leading to double text while editing....
    QPalette p = The::paletteHandler()->palette();
    QColor c = p.color( QPalette::Base );
    setStyleSheet("QLineEdit { background-color: " + c.name() + " }");

    setDragEnabled( true );
    setAcceptDrops( true );
    setDropIndicatorShown( true );
}


BookmarkTreeView::~BookmarkTreeView()
{
}

void BookmarkTreeView::mouseDoubleClickEvent( QMouseEvent * event )
{
    QModelIndex index = indexAt( event->pos() );

    if( index.isValid() )
    {
        BookmarkViewItemPtr item = BookmarkModel::instance()->data( index, 0xf00d ).value<BookmarkViewItemPtr>();

        if ( typeid( *item ) == typeid( AmarokUrl ) ) {
            AmarokUrl * bookmark = static_cast< AmarokUrl* >( item.data() );
            bookmark->run();
        }
    }
}


void
BookmarkTreeView::keyPressEvent( QKeyEvent *event )
{
    switch( event->key() )
    {
        case Qt::Key_Delete:
            slotDelete();
            return;

        case Qt::Key_F2:
            slotRename();
            return;
    }
    QTreeView::keyPressEvent( event );
}

QList<KAction *>
BookmarkTreeView::createCommonActions( QModelIndexList indices )
{

    QList< KAction * > actions;
    
    if ( m_loadAction == 0 )
    {
        m_loadAction = new KAction( KIcon( "folder-open" ), i18nc( "Load the view represented by this bookmark", "&Load" ), this );
        connect( m_loadAction, SIGNAL( triggered() ), this, SLOT( slotLoad() ) );
    }

    if ( m_deleteAction == 0 )
    {
        m_deleteAction = new KAction( KIcon( "media-track-remove-amarok" ), i18n( "&Delete" ), this );
        connect( m_deleteAction, SIGNAL( triggered() ), this, SLOT( slotDelete() ) );
    }

    if ( m_renameAction == 0 )
    {
        m_renameAction = new KAction( KIcon( "media-track-edit-amarok" ), i18n( "&Rename" ), this );
        connect( m_renameAction, SIGNAL( triggered() ), this, SLOT( slotRename() ) );
    }
    
    if ( indices.count() > 0 )
    {
        actions << m_loadAction;
    }

    if ( indices.count() > 0 )
        actions << m_deleteAction;

    if ( indices.count() == 1 )
        actions << m_renameAction;


    return actions;
}

void BookmarkTreeView::slotLoad()
{
    DEBUG_BLOCK
    foreach( BookmarkViewItemPtr item, selectedItems() )
    {
        if( typeid( * item ) == typeid( AmarokUrl ) )
        {
            AmarokUrlPtr bookmark = AmarokUrlPtr::staticCast( item );
            bookmark->run();
        }
    }
}

void BookmarkTreeView::slotDelete()
{
    DEBUG_BLOCK

    //TODO FIXME Confirmation of delete

    foreach( BookmarkViewItemPtr item, selectedItems() )
    {
        debug() << "deleting " << item->name();
        item->removeFromDb();
        item->parent()->deleteChild( item );
    }
    BookmarkModel::instance()->reloadFromDb();
}

void BookmarkTreeView::slotRename()
{
    DEBUG_BLOCK
    if ( selectionModel()->hasSelection() )
        edit( selectionModel()->selectedIndexes().first() );
}

void BookmarkTreeView::contextMenuEvent( QContextMenuEvent * event )
{
    DEBUG_BLOCK

    QModelIndexList indices = selectionModel()->selectedIndexes();

    KMenu* menu = new KMenu( this );

    QList<KAction *> actions = createCommonActions( indices );

    foreach( KAction * action, actions )
        menu->addAction( action );

    if( indices.count() == 0 )
        menu->addAction( m_addGroupAction );

    debug() << "showing menu at pos:" << event->pos() << "and globalpos:" << event->globalPos();
    emit showMenu( menu, event->globalPos() );
}

QSet<BookmarkViewItemPtr>
BookmarkTreeView::selectedItems() const
{
    QSet<BookmarkViewItemPtr> selected;
    foreach( const QModelIndex &index, selectionModel()->selectedIndexes() )
    {
        if( index.isValid() && index.internalPointer() )
            selected.insert( BookmarkModel::instance()->data( index, 0xf00d ).value<BookmarkViewItemPtr>() );
    } 
    return selected;
}

void BookmarkTreeView::setNewGroupAction( KAction * action )
{
    m_addGroupAction = action;
}

void BookmarkTreeView::selectionChanged( const QItemSelection & selected, const QItemSelection & deselected )
{
    DEBUG_BLOCK
    Q_UNUSED( deselected )
    QModelIndexList indexes = selected.indexes();
    if ( indexes.size() == 1 ) {
        QModelIndex index = indexes.at( 0 );
        BookmarkViewItemPtr item = BookmarkModel::instance()->data( index, 0xf00d ).value<BookmarkViewItemPtr>();

        if ( typeid( * item ) == typeid( AmarokUrl ) ) {
            debug() << "a url was selected...";
            AmarokUrl bookmark = *static_cast< AmarokUrl* >( item.data() );
            emit( bookmarkSelected( bookmark ) );
        }
    }
    
}

KMenu* BookmarkTreeView::contextMenu( const QPoint& point )
{
    DEBUG_BLOCK
    KMenu* menu = new KMenu( 0 );

    debug() << "getting menu for point:" << point;
    QModelIndex index = indexAt( point );
    if( index.isValid() )
    {

        debug() << "got valid index";
        
        QModelIndexList indices = selectionModel()->selectedIndexes();

        QList<KAction *> actions = createCommonActions( indices );

        foreach( KAction * action, actions )
            menu->addAction( action );

        if( indices.count() == 0 )
            menu->addAction( m_addGroupAction );

    }
    
    return menu;
}

#include "BookmarkTreeView.moc"



