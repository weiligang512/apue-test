/***************************************************************************
 *   Copyright (c) 2009  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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
 
#include "PlaylistLayoutEditDialog.h"

#include "Debug.h"

#include "playlist/layouts/LayoutManager.h"
#include "playlist/PlaylistDefines.h"

#include <KMessageBox>

#include <QInputDialog>

using namespace Playlist;

PlaylistLayoutEditDialog::PlaylistLayoutEditDialog( QWidget *parent )
    : QDialog( parent )
{
    setupUi( this );

    tokenPool->addToken( new Token( i18nc( "'Album' playlist column name and token for playlist layouts", columnNames[Album] ), iconNames[Album], Album ) );
    tokenPool->addToken( new Token( i18nc( "'Album artist' playlist column name and token for playlist layouts", columnNames[AlbumArtist] ), iconNames[AlbumArtist], AlbumArtist ) );
    tokenPool->addToken( new Token( i18nc( "'Artist' playlist column name and token for playlist layouts", columnNames[Artist] ), iconNames[Artist], Artist ) );
    tokenPool->addToken( new Token( i18nc( "'Bitrate' playlist column name and token for playlist layouts", columnNames[Bitrate] ), iconNames[Bitrate], Bitrate ) );
    tokenPool->addToken( new Token( i18nc( "'Comment' playlist column name and token for playlist layouts", columnNames[Comment] ), iconNames[Comment], Comment ) );
    tokenPool->addToken( new Token(  i18nc( "'Composer' playlist column name and token for playlist layouts", columnNames[Composer] ), iconNames[Composer], Composer ) );
    tokenPool->addToken( new Token( i18nc( "'Directory' playlist column name and token for playlist layouts", columnNames[Directory] ), iconNames[Directory], Directory ) );
    tokenPool->addToken( new Token( i18nc( "'Disc number' playlist column name and token for playlist layouts", columnNames[DiscNumber] ), iconNames[DiscNumber], DiscNumber ) );
    tokenPool->addToken( new Token( i18nc( "'Divider' token for playlist layouts representing a small visual divider", columnNames[Divider] ), iconNames[Divider], Divider ) );
    tokenPool->addToken( new Token( i18nc( "'File name' playlist column name and token for playlist layouts", columnNames[Filename] ), iconNames[Filename], Filename ) );
    tokenPool->addToken( new Token( i18nc( "'File size' playlist column name and token for playlist layouts", columnNames[Filesize] ), iconNames[Filesize], Filesize ) );
    tokenPool->addToken( new Token( i18nc( "'Genre' playlist column name and token for playlist layouts", columnNames[Genre] ), iconNames[Genre], Genre ) );
    tokenPool->addToken( new Token( i18nc( "'Group length' (total play time of group) playlist column name and token for playlist layouts", columnNames[GroupLength] ), iconNames[GroupLength], GroupLength ) );
    tokenPool->addToken( new Token( i18nc( "'Group tracks' (number of tracks in group) playlist column name and token for playlist layouts", columnNames[GroupTracks] ), iconNames[GroupTracks], GroupTracks ) );
    tokenPool->addToken( new Token( i18nc( "'Last played' (when was track last played) playlist column name and token for playlist layouts", columnNames[LastPlayed] ), iconNames[LastPlayed], LastPlayed ) );
    tokenPool->addToken( new Token( i18nc( "'Length' (track length) playlist column name and token for playlist layouts", columnNames[Length] ), iconNames[Length], Length ) );
    tokenPool->addToken( new Token( i18nc( "Empty placeholder token used for spacing in playlist layouts", columnNames[PlaceHolder] ), iconNames[PlaceHolder], PlaceHolder ) );
    tokenPool->addToken( new Token( i18nc( "'Play count' playlist column name and token for playlist layouts", columnNames[PlayCount] ), iconNames[PlayCount], PlayCount ) );
    tokenPool->addToken( new Token( i18nc( "'Rating' playlist column name and token for playlist layouts", columnNames[Rating] ), iconNames[Rating], Rating ) );
    tokenPool->addToken( new Token( i18nc( "'Sample rate' playlist column name and token for playlist layouts", columnNames[SampleRate] ), iconNames[SampleRate], SampleRate ) );
    tokenPool->addToken( new Token( i18nc( "'Score' playlist column name and token for playlist layouts", columnNames[Score] ), iconNames[Score], Score ) );
    tokenPool->addToken( new Token( i18nc( "'Source' (local collection, Magnatune.com, last.fm, ... ) playlist column name and token for playlist layouts", columnNames[Source] ), iconNames[Source], Source ) );
    tokenPool->addToken( new Token( i18nc( "'Title' (track name) playlist column name and token for playlist layouts", columnNames[Title] ), iconNames[Title], Title ) );
    tokenPool->addToken( new Token( i18nc( "'Title (with track number)' (track name prefixed with the track number) playlist column name and token for playlist layouts", columnNames[TitleWithTrackNum] ), iconNames[TitleWithTrackNum], TitleWithTrackNum ) );
    tokenPool->addToken( new Token( i18nc( "'Track number' playlist column name and token for playlist layouts", columnNames[TrackNumber] ), iconNames[TrackNumber], TrackNumber ) );
    tokenPool->addToken( new Token(i18nc( "'Type' (file format) playlist column name and token for playlist layouts",  columnNames[Type] ), iconNames[Type], Type ) );
    tokenPool->addToken( new Token( i18nc( "'Year' playlist column name and token for playlist layouts", columnNames[Year] ), iconNames[Year], Year ) );

    m_firstActiveLayout = LayoutManager::instance()->activeLayoutName();

    //add an editor to each tab
    m_headEdit = new Playlist::LayoutEditWidget( this );
    m_bodyEdit = new Playlist::LayoutEditWidget( this );
    m_singleEdit = new Playlist::LayoutEditWidget( this );
    m_layoutsMap = new QMap<QString, PlaylistLayout>();

    elementTabs->addTab( m_headEdit, i18n( "Head" ) );
    elementTabs->addTab( m_bodyEdit, i18n( "Body" ) );
    elementTabs->addTab( m_singleEdit, i18n( "Single" ) );

    elementTabs->removeTab( 0 );

    QStringList layoutNames = LayoutManager::instance()->layouts();
    foreach( QString layoutName, layoutNames )
    {
        PlaylistLayout layout = LayoutManager::instance()->layout( layoutName );
        layout.setDirty( false );
        m_layoutsMap->insert( layoutName, layout );
    }

    layoutListWidget->addItems( layoutNames );

    layoutListWidget->setCurrentRow( LayoutManager::instance()->layouts().indexOf( LayoutManager::instance()->activeLayoutName() ) );

    if ( layoutListWidget->currentItem() )
        setLayout( layoutListWidget->currentItem()->text() );

    connect( previewButton, SIGNAL( clicked() ), this, SLOT( preview() ) );
    connect( layoutListWidget, SIGNAL( currentTextChanged( const QString & ) ), this, SLOT( setLayout( const QString & ) ) );
    connect( layoutListWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT( toggleDeleteButton() ) );
    connect( layoutListWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT( toggleUpDownButtons() ) );

    connect( moveUpButton, SIGNAL( clicked() ), this, SLOT( moveUp() ) );
    connect( moveDownButton, SIGNAL( clicked() ), this, SLOT( moveDown() ) );

    buttonBox->button(QDialogButtonBox::Apply)->setIcon( KIcon( "dialog-ok-apply" ) );
    buttonBox->button(QDialogButtonBox::Ok)->setIcon( KIcon( "dialog-ok" ) );
    buttonBox->button(QDialogButtonBox::Cancel)->setIcon( KIcon( "dialog-cancel" ) );
    connect( buttonBox->button(QDialogButtonBox::Apply), SIGNAL( clicked() ), this, SLOT( apply() ) );

    const KIcon newIcon( "document-new" );
    newLayoutButton->setIcon( newIcon );
    newLayoutButton->setToolTip( i18n( "New playlist layout" ) );
    connect( newLayoutButton, SIGNAL( clicked() ), this, SLOT( newLayout() ) );
    
    const KIcon copyIcon( "edit-copy" );
    copyLayoutButton->setIcon( copyIcon );
    copyLayoutButton->setToolTip( i18n( "Copy playlist layout" ) );
    connect( copyLayoutButton, SIGNAL( clicked() ), this, SLOT( copyLayout() ) );
    
    const KIcon deleteIcon( "edit-delete" );
    deleteLayoutButton->setIcon( deleteIcon );
    deleteLayoutButton->setToolTip( i18n( "Delete playlist layout" ) );
    connect( deleteLayoutButton, SIGNAL( clicked() ), this, SLOT( deleteLayout() ) );
    toggleDeleteButton();

    const KIcon renameIcon( "edit-rename" );
    renameLayoutButton->setIcon( renameIcon );
    renameLayoutButton->setToolTip( i18n( "Rename playlist layout" ) );
    connect( renameLayoutButton, SIGNAL( clicked() ), this, SLOT( renameLayout() ) );

    toggleUpDownButtons();

    connect( m_headEdit, SIGNAL( changed() ), this, SLOT( setLayoutChanged() ) );
    connect( m_bodyEdit, SIGNAL( changed() ), this, SLOT( setLayoutChanged() ) );
    connect( m_singleEdit, SIGNAL( changed() ), this, SLOT( setLayoutChanged() ) );
}


PlaylistLayoutEditDialog::~PlaylistLayoutEditDialog()
{
}

void PlaylistLayoutEditDialog::newLayout()      //SLOT
{
    QString layoutName = QInputDialog::getText( this, i18n( "Choose a name for the new playlist layout" ),
                    i18n( "Please enter a name for the playlist layout you are about to define:" ) );
    if( layoutName.isEmpty() )
    {
        KMessageBox::sorry( this, i18n( "Cannot create a layout with no name." ), i18n( "Layout name error" ) );
        return;
    }
    if( m_layoutsMap->keys().contains( layoutName ) )
    {
        KMessageBox::sorry( this, i18n( "Cannot create a layout with the same name as an existing layout." ), i18n( "Layout name error" ) );
        return;
    }
    if( layoutName.contains( '/' ) )
    {
        KMessageBox::sorry( this, i18n( "Cannot create a layout containing '/'." ), i18n( "Layout name error" ) );
        return;
    }

    debug() << "Creating new layout " << layoutName;
    PlaylistLayout layout;
    layout.setEditable( true );      //Should I use true, TRUE or 1?
    layout.setDirty( true );

    layoutListWidget->addItem( layoutName );
    layoutListWidget->setCurrentItem( (layoutListWidget->findItems( layoutName, Qt::MatchExactly ) ).first() );
    m_headEdit->clear();
    m_bodyEdit->clear();
    m_singleEdit->clear();
    layout.setHead( m_headEdit->config() );
    layout.setBody( m_bodyEdit->config() );
    layout.setSingle( m_singleEdit->config() );
    m_layoutsMap->insert( layoutName, layout );

    LayoutManager::instance()->addUserLayout( layoutName, layout );

    setLayout( layoutName );
}

void PlaylistLayoutEditDialog::copyLayout()
{
    LayoutItemConfig headConfig = m_headEdit->config();
    LayoutItemConfig bodyConfig = m_bodyEdit->config();
    LayoutItemConfig singleConfig = m_singleEdit->config();
    
    QString layoutName = QInputDialog::getText( this, i18n( "Choose a name for the new playlist layout" ),
                    i18n( "Please enter a name for the playlist layout you are about to define as copy of the layout '%1':",
                    layoutListWidget->currentItem()->text() ) );
    if( layoutName.isEmpty() )
    {
        KMessageBox::sorry( this, i18n( "Cannot create a layout with no name." ), i18n( "Layout name error" ) );
        return;
    }
    if( m_layoutsMap->keys().contains( layoutName ) )
    {
        KMessageBox::sorry( this, i18n( "Cannot create a layout with the same name as an existing layout." ), i18n( "Layout name error" ) );
        return;
    }
    debug() << "Copying layout " << layoutName;
    //layoutListWidget->addItem( layoutName );
    PlaylistLayout layout;
    layout.setEditable( true );      //Should I use true, TRUE or 1?
    layout.setDirty( true );

    headConfig.setActiveIndicatorRow( -1 );
    layout.setHead( headConfig );
    layout.setBody( bodyConfig );
    layout.setSingle( singleConfig );

    LayoutManager::instance()->addUserLayout( layoutName, layout );

    //reload from manager:
    layoutListWidget->clear();
    layoutListWidget->addItems( LayoutManager::instance()->layouts() );

    m_layoutsMap->insert( layoutName, layout );
    layoutListWidget->setCurrentItem( ( layoutListWidget->findItems( layoutName, Qt::MatchExactly ) ).first() );
    setLayout( layoutName );
}

void PlaylistLayoutEditDialog::deleteLayout()   //SLOT
{
    m_layoutsMap->remove( layoutListWidget->currentItem()->text() );
    if( LayoutManager::instance()->layouts().contains( layoutListWidget->currentItem()->text() ) )  //if the layout is already saved in the LayoutManager
        LayoutManager::instance()->deleteLayout( layoutListWidget->currentItem()->text() );         //delete it
    delete layoutListWidget->currentItem();
}

void PlaylistLayoutEditDialog::renameLayout()
{
    PlaylistLayout layout = m_layoutsMap->value( layoutListWidget->currentItem()->text() );
    
    QString layoutName;
    while( layoutName.isEmpty() || m_layoutsMap->keys().contains( layoutName ) )
    {
        bool ok;
        layoutName = QInputDialog::getText( this, i18n( "Choose a new name for the playlist layout" ),
                    i18n( "Please enter a new name for the playlist layout you are about to rename:" ),
                    QLineEdit::Normal, layoutListWidget->currentItem()->text(), &ok);
        if ( !ok )
        {
            //Cancelled so just return
            return;
        }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
        if( LayoutManager::instance()->isDefaultLayout( layoutName ) )
        {
            KMessageBox::sorry( this, i18n( "Cannot rename one of the default layouts." ), i18n( "Layout name error" ) );
            return;
        }
        if( layoutName.isEmpty() )
            KMessageBox::sorry( this, i18n( "Cannot rename a layout to have no name." ), i18n( "Layout name error" ) );
        if( m_layoutsMap->keys().contains( layoutName ) )
            KMessageBox::sorry( this, i18n( "Cannot rename a layout to have the same name as an existing layout." ), i18n( "Layout name error" ) );
    }
    debug() << "Renaming layout " << layoutName;
    m_layoutsMap->remove( layoutListWidget->currentItem()->text() );
    if( LayoutManager::instance()->layouts().contains( layoutListWidget->currentItem()->text() ) )  //if the layout is already saved in the LayoutManager
        LayoutManager::instance()->deleteLayout( layoutListWidget->currentItem()->text() );         //delete it
    LayoutManager::instance()->addUserLayout( layoutName, layout );
    m_layoutsMap->insert( layoutName, layout );
    layoutListWidget->currentItem()->setText( layoutName );

    setLayout( layoutName );
}

/**
 * Loads the configuration of the layout layoutName from the m_layoutsMap to the LayoutItemConfig area.
 * @param layoutName the name of the PlaylistLayout to be loaded for configuration
 */
void PlaylistLayoutEditDialog::setLayout( const QString &layoutName )   //SLOT
{
    m_layoutName = layoutName;
    debug()<< "Trying to load layout for configuration " << layoutName;

    if( m_layoutsMap->keys().contains( layoutName ) )   //is the layout exists in the list of loaded layouts
    {
        PlaylistLayout layout = m_layoutsMap->value( layoutName );
        m_headEdit->readLayout( layout.head() );
        m_bodyEdit->readLayout( layout.body() );
        m_singleEdit->readLayout( layout.single() );
    }
    else
    {
        debug() << "Empty layout, clearing config view";
        m_headEdit->clear();
        m_bodyEdit->clear();
        m_singleEdit->clear();
    }
}

void PlaylistLayoutEditDialog::preview()
{
    PlaylistLayout layout;

    LayoutItemConfig headConfig = m_headEdit->config() ;
    headConfig.setActiveIndicatorRow( -1 );
    
    layout.setHead( headConfig );
    layout.setBody( m_bodyEdit->config() );
    layout.setSingle( m_singleEdit->config() );

    LayoutManager::instance()->setPreviewLayout( layout );
}

void PlaylistLayoutEditDialog::toggleDeleteButton() //SLOT
{
    if ( !layoutListWidget->currentItem() )
        deleteLayoutButton->setEnabled( 0 );
    else if( LayoutManager::instance()->isDefaultLayout( layoutListWidget->currentItem()->text() ) )
        deleteLayoutButton->setEnabled( 0 );
    else
        deleteLayoutButton->setEnabled( 1 );
}

void PlaylistLayoutEditDialog::toggleUpDownButtons()
{
    if ( !layoutListWidget->currentItem() )
    {
        moveUpButton->setEnabled( 0 );
        moveDownButton->setEnabled( 0 );
    }
    else if ( layoutListWidget->currentRow() == 0 )
    {
        moveUpButton->setEnabled( 0 );
        if ( layoutListWidget->currentRow() >= m_layoutsMap->size() -1 )
            moveDownButton->setEnabled( 0 );
        else
            moveDownButton->setEnabled( 1 );
    }
    else if ( layoutListWidget->currentRow() >= m_layoutsMap->size() -1 )
    {
        moveDownButton->setEnabled( 0 );
        moveUpButton->setEnabled( 1 ); //we already cheked that this is not row 0
    }
    else
    {
        moveDownButton->setEnabled( 1 );
        moveUpButton->setEnabled( 1 );
    }
        
    
}

void PlaylistLayoutEditDialog::apply()  //SLOT
{
    QMap<QString, PlaylistLayout>::Iterator i = m_layoutsMap->begin();
    while( i != m_layoutsMap->end() )
    {
        debug() << "I'm on layout " << i.key();
        if( i.value().isDirty() )
        {
            debug() << "Layout " << i.key() << " has been modified and will be saved.";
            if ( LayoutManager::instance()->isDefaultLayout( i.key() ) )
            {
                const QString msg = i18n( "The layout '%1' you modified is one of the default layouts and cannot be overwritten. \
                Please select a different name to save a copy.", i.key() );
                KMessageBox::sorry( this, msg, i18n( "Reserved Layout Name" ) );
                //TODO: handle this on layout switch maybe? this is not the right time to tell users they needed to make a copy in the first place
                i.value().setHead( LayoutManager::instance()->layout( i.key() ).head() );
                i.value().setBody( LayoutManager::instance()->layout( i.key() ).body() );
                i.value().setSingle( LayoutManager::instance()->layout( i.key() ).single() );
                i.value().setDirty( false );
                if ( m_layoutName == i.key() )
                    setLayout( i.key() );
                return;
            }
            i.value().setDirty( false );
            LayoutManager::instance()->addUserLayout( i.key(), i.value() );
            debug() << "Layout " << i.key() << " saved to LayoutManager";
        }
        i++;
    }
    LayoutManager::instance()->setActiveLayout( layoutListWidget->currentItem()->text() );  //important to override the previewed layout if preview is used
}

void PlaylistLayoutEditDialog::accept()     //SLOT
{
    apply();
    QDialog::accept();
}

void PlaylistLayoutEditDialog::reject()     //SLOT
{
    DEBUG_BLOCK

    debug() << "Applying initial layout: " << m_firstActiveLayout;
    LayoutManager::instance()->setActiveLayout( m_firstActiveLayout );

    QDialog::reject();
}

void PlaylistLayoutEditDialog::moveUp()
{
    int newRow = LayoutManager::instance()->moveUp( m_layoutName );

    layoutListWidget->clear();
    layoutListWidget->addItems( LayoutManager::instance()->layouts() );

    layoutListWidget->setCurrentRow( newRow );
}

void PlaylistLayoutEditDialog::moveDown()
{
    int newRow = LayoutManager::instance()->moveDown( m_layoutName );

    layoutListWidget->clear();
    layoutListWidget->addItems( LayoutManager::instance()->layouts() );

    layoutListWidget->setCurrentRow( newRow );
}

void PlaylistLayoutEditDialog::setLayoutChanged()
{
    (*m_layoutsMap)[m_layoutName].setHead( m_headEdit->config() );
    (*m_layoutsMap)[m_layoutName].setBody( m_bodyEdit->config() );
    (*m_layoutsMap)[m_layoutName].setSingle( m_singleEdit->config() );
    (*m_layoutsMap)[m_layoutName].setDirty( true );
}


#include "PlaylistLayoutEditDialog.moc"

