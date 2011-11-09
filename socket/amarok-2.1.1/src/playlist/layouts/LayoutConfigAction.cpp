/***************************************************************************
 *   Copyright (c) 2008  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *             (c) 2009  Teo Mrnjavac <teo.mrnjavac@gmail.com>             *
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
 
#include "LayoutConfigAction.h"

#include "Debug.h"
#include "LayoutManager.h"
#include "dialogs/PlaylistLayoutEditDialog.h"
#include "widgets/EditDeleteDelegate.h"
#include "widgets/EditDeleteComboBoxView.h"
#include "MainWindow.h"

#include <QLabel>
#include <QComboBox>

namespace Playlist
{

LayoutConfigAction::LayoutConfigAction( QWidget * parent )
    : KAction( parent )
{
    KIcon actionIcon( "configure" );    //TEMPORARY ICON
    setIcon( actionIcon );
    m_layoutMenu = new KMenu( parent );
    setMenu( m_layoutMenu );
    setText( i18n( "Playlist layouts" ) );
    m_configAction = new KAction( m_layoutMenu );
    
    m_layoutMenu->addAction( m_configAction );
    m_layoutMenu->addSeparator();
    m_layoutActions = new QActionGroup( m_layoutMenu );
    m_layoutActions->setExclusive( true );

    QStringList layoutsList( LayoutManager::instance()->layouts() );
    foreach( QString iterator, layoutsList )
    {
        m_layoutActions->addAction( iterator )->setCheckable( true );
    }
    m_layoutMenu->addActions( m_layoutActions->actions() );
    int index = LayoutManager::instance()->layouts().indexOf( LayoutManager::instance()->activeLayoutName() );
    debug() << "About to check layout at index " << index;
    if( index > -1 )    //needed to avoid crash when created a layout which is moved by the LayoutManager when sorting alphabetically.
                        //this should be fixed by itself when layouts ordering will be supported in the LayoutManager
    m_layoutActions->actions()[ index ]->setChecked( true );

    connect( m_layoutActions, SIGNAL( triggered( QAction * ) ), this, SLOT( setActiveLayout( QAction * ) ) );

    connect( LayoutManager::instance(), SIGNAL( layoutListChanged() ), this, SLOT( layoutListChanged() ) );
    connect( LayoutManager::instance(), SIGNAL( activeLayoutChanged() ), this, SLOT( onActiveLayoutChanged() ) );

    const KIcon configIcon( "configure" );
    m_configAction->setIcon( configIcon );
    m_configAction->setText( i18n( "Configure playlist layouts..." ) );

    connect( m_configAction, SIGNAL( triggered() ), this, SLOT( configureLayouts() ) );
}


LayoutConfigAction::~LayoutConfigAction()
{}

void LayoutConfigAction::setActiveLayout( QAction *layoutAction )
{
    QString layoutName( layoutAction->text() );
    layoutName = layoutName.remove( QChar( '&' ) );        //need to remove the & from the string, used for the shortcut key underscore
    LayoutManager::instance()->setActiveLayout( layoutName );
}

void LayoutConfigAction::configureLayouts()
{
    PlaylistLayoutEditDialog layoutEditor( The::mainWindow() );
    layoutEditor.exec();
    layoutListChanged();
}

void Playlist::LayoutConfigAction::layoutListChanged()
{
    m_layoutMenu->removeAction( m_configAction );
    m_layoutMenu->clear();
    m_layoutMenu->addAction( m_configAction );
    m_layoutMenu->addSeparator();
    foreach( QAction * action, m_layoutActions->actions() )
    {
        delete action;
    }
    QStringList layoutsList( LayoutManager::instance()->layouts() );
    debug() << "Layouts are " << layoutsList;
    foreach( QString iterator, layoutsList )
    {
        m_layoutActions->addAction( iterator )->setCheckable( true );
    }
    m_layoutMenu->addActions( m_layoutActions->actions() );
    int index = LayoutManager::instance()->layouts().indexOf( LayoutManager::instance()->activeLayoutName() );
    debug() << "About to check layout at index " << index;
    if( index > -1 )    //needed to avoid crash when created a layout which is moved by the LayoutManager when sorting alphabetically.
                        //this should be fixed by itself when layouts ordering will be supported in the LayoutManager
        m_layoutActions->actions()[ index ]->setChecked( true );
}

void LayoutConfigAction::onActiveLayoutChanged()
{
    QString layoutName( LayoutManager::instance()->activeLayoutName() );
    layoutName = layoutName.remove( QChar( '&' ) );        //need to remove the & from the string, used for the shortcut key underscore
    if( layoutName != QString( "%%PREVIEW%%" ) )           //if it's not just a preview
    {
        int index = LayoutManager::instance()->layouts().indexOf( layoutName );
        debug() << "Index in the LayoutManager of currently active layout, called " << LayoutManager::instance()->activeLayoutName() << ", is: " << index;
        if( index != -1 && m_layoutActions->actions()[ index ] != m_layoutActions->checkedAction() )
            m_layoutActions->actions()[ index ]->setChecked( true );
    }
}

}

#include "LayoutConfigAction.moc"
