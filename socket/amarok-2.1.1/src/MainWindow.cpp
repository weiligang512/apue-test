/***************************************************************************
  begin                : Fre Nov 15 2002
  copyright            : (C) Mark Kretschmann <markey@web.de>
                       : (C) Max Howell <max.howell@methylblue.com>
                       : (C) G??bor Lehel <illissius@gmail.com>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define DEBUG_PREFIX "MainWindow"

#include "MainWindow.h"

#include <config-amarok.h>    //HAVE_LIBVISUAL definition

#include "ActionClasses.h"
#include "Amarok.h"
#include "Debug.h"
#include "EngineController.h" //for actions in ctor
#include "MainToolbar.h"
#include "Osd.h"
#include "PaletteHandler.h"
#include "ScriptManager.h"
#include "SearchWidget.h"
#include "Sidebar.moc"
#include "amarokconfig.h"
#include "amarokurls/AmarokUrlHandler.h"
#include "browsers/collectionbrowser/CollectionWidget.h"
#include "browsers/filebrowser/FileBrowser.h"
#include "browsers/playlistbrowser/PlaylistBrowser.h"
#include "browsers/servicebrowser/ServiceBrowser.h"
#include "collection/CollectionManager.h"
#include "context/ContextScene.h"
#include "context/ContextView.h"
#include "context/ToolbarView.h"
#include "covermanager/CoverManager.h" // for actions
#include "playlist/PlaylistActions.h"
#include "playlist/PlaylistController.h"
#include "playlist/PlaylistModel.h"
#include "playlist/PlaylistWidget.h"
#include "playlistmanager/PlaylistFileProvider.h"
#include "playlistmanager/PlaylistManager.h"
#include "services/ServicePluginManager.h"
#include "services/scriptable/ScriptableService.h"
#include "statusbar/StatusBar.h"
#include "widgets/Splitter.h"
//#include "mediabrowser.h"

#include <QCheckBox>
#include <QDesktopWidget>
#include <QList>
#include <QSizeGrip>
#include <QVBoxLayout>

#include <KAction>          //m_actionCollection
#include <KActionCollection>
#include <KApplication>     //kapp
#include <KFileDialog>      //savePlaylist(), openPlaylist()
#include <KInputDialog>     //slotAddStream()
#include <KMessageBox>
#include <KLocale>
#include <KMenu>
#include <KMenuBar>
#include <KPixmapCache>
#include <KStandardAction>
#include <KWindowSystem>
#include <kabstractfilewidget.h> //savePlaylist()

#include <plasma/plasma.h>

#ifdef Q_WS_X11
#include <fixx11h.h>
#endif

#ifdef Q_WS_MAC
#include "mac/GrowlInterface.h"
#endif

// Let people know OS X and Windows versions are still work-in-progress
#if defined(Q_WS_MAC) || defined(Q_WS_WIN)
#define AMAROK_CAPTION "Amarok 2 beta"
#else
#define AMAROK_CAPTION "Amarok 2"
#endif

class ContextWidget : public KVBox
{
    // Set a useful size default of the center tab.
    public:
        ContextWidget( QWidget *parent ) : KVBox( parent ) {}

        QSize sizeHint() const
        {
            return QSize( static_cast<QWidget*>( parent() )->size().width() / 3, 300 );
        }
};

QPointer<MainWindow> MainWindow::s_instance = 0;

MainWindow::MainWindow()
    : KMainWindow( 0 )
    , EngineObserver( The::engineController() )
    , m_lastBrowser( 0 )
{
    DEBUG_BLOCK

    setObjectName( "MainWindow" );
    s_instance = this;

#ifdef Q_WS_MAC
    QSizeGrip* grip = new QSizeGrip( this );
    GrowlInterface* growl = new GrowlInterface( qApp->applicationName() );
#endif
    //create this object now as we might run into issues if anyone tries to use it during initialization
    //make room for a full width statusbar at the bottom of everything
    m_statusbarArea = new KVBox( this );
    //figure out the needed height based on system font settings
    //do make sure that it is at least 26 pixels tall though
    //or progress bars will not fit...
    QFont currentFont = font();
    currentFont.setBold( true );
    QFontMetrics fm( currentFont );
    int fontHeight = qMax( 26, fm.height() );
    m_statusbarArea->setMinimumHeight( fontHeight );
    m_statusbarArea->setMaximumHeight( fontHeight );

    new ::StatusBar( m_statusbarArea );

    // Sets caption and icon correctly (needed e.g. for GNOME)
    kapp->setTopWidget( this );
    PERF_LOG( "Set Top Widget" )
    createActions();
    PERF_LOG( "Created actions" )

    //new K3bExporter();

    KConfigGroup config = Amarok::config();
    const QSize size = config.readEntry( "MainWindow Size", QSize() );
    const QPoint pos = config.readEntry( "MainWindow Position", QPoint() );
    if( size.isValid() )
    {
        resize( size );
        move( pos );
    }

    The::paletteHandler()->setPalette( palette() );
    setPlainCaption( i18n( AMAROK_CAPTION ) );

    init();  // We could as well move the code from init() here, but meh.. getting a tad long
}

MainWindow::~MainWindow()
{
    DEBUG_BLOCK

    KConfigGroup config = Amarok::config();
    config.writeEntry( "MainWindow Size", size() );
    config.writeEntry( "MainWindow Position", pos() );

    QList<int> sPanels;

    foreach( int a, m_splitter->saveState() )
        sPanels.append( a );

    AmarokConfig::setPanelsSavedState( sPanels );

    delete m_playlistFiles;
    delete m_contextView;
    delete m_corona;
    delete m_splitter;
    delete The::statusBar();
    delete m_controlBar;
    delete The::svgHandler();
    delete The::paletteHandler();
}


///////// public interface

/**
 * This function will initialize the main window.
 */
void
MainWindow::init()
{
    DEBUG_BLOCK

    layout()->setContentsMargins( 0, 0, 0, 0 );
    layout()->setSpacing( 0 );

    m_controlBar = new MainToolbar( 0 );
    m_controlBar->layout()->setContentsMargins( 0, 0, 0, 0 );
    m_controlBar->layout()->setSpacing( 0 );

    PERF_LOG( "Create sidebar" )
    m_browsers = new SideBar( this, new KVBox );
    m_browsers->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Ignored );

    PERF_LOG( "Create Playlist" )
    m_playlistWidget = new Playlist::Widget( this );
    m_playlistWidget->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Ignored );
    m_playlistWidget->setFocus( Qt::ActiveWindowFocusReason );
    PERF_LOG( "Playlist created" )

    createMenus();

    PERF_LOG( "Creating ContextWidget" )
    m_contextWidget = new ContextWidget( this );
    PERF_LOG( "ContextWidget created" )
    m_contextWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
    m_contextWidget->setSpacing( 0 );
    m_contextWidget->setFrameShape( QFrame::NoFrame );
    m_contextWidget->setFrameShadow( QFrame::Sunken );
    PERF_LOG( "Creating ContexScene" )

    m_corona = new Context::ContextScene( this );
    connect( m_corona, SIGNAL( containmentAdded( Plasma::Containment* ) ),
            this, SLOT( createContextView( Plasma::Containment* ) ) );

    PERF_LOG( "ContextScene created" )

    PERF_LOG( "Loading default contextScene" )
    m_corona->loadDefaultSetup(); // this method adds our containment to the scene
    PERF_LOG( "Loaded default contextScene" )

    connect( m_browsers, SIGNAL( widgetActivated( int ) ), SLOT( slotShrinkBrowsers( int ) ) );

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins( 0, 0, 0, 0 );
    mainLayout->setSpacing( 0 );

    QBoxLayout *toolbarSpacer = new QHBoxLayout;
    toolbarSpacer->setContentsMargins( 0, 0, 0, 10 );
    toolbarSpacer->addSpacing( 3 );
    toolbarSpacer->addWidget( m_controlBar, 20);
    toolbarSpacer->addSpacing( 3 );


    QWidget *centralWidget = new QWidget( this );
    centralWidget->setLayout( mainLayout );

    m_splitter = new Amarok::Splitter( Qt::Horizontal, centralWidget );
    m_splitter->setHandleWidth( 0 );
    m_splitter->addWidget( m_browsers );
    m_splitter->addWidget( m_contextWidget );
    m_splitter->addWidget( m_playlistWidget );


    mainLayout->addLayout( toolbarSpacer );
    mainLayout->addWidget( m_splitter );
    mainLayout->addWidget( m_statusbarArea);

    setCentralWidget( centralWidget );

    //<Browsers>
    {
        Debug::Block block( "Creating browsers. Please report long start times!" );

        #define addBrowserMacro( Type, name, text, icon ) { \
            Debug::Block block( name ); \
             m_browsers->addWidget( KIcon( icon ), text, new Type( name , m_browsers ) ); \
             m_browserNames.append( name ); }


        PERF_LOG( "Creating CollectionWidget" )
        addBrowserMacro( CollectionWidget, "CollectionBrowser", i18n("Collection"), "collection-amarok" )
        PERF_LOG( "Created CollectionWidget" )

        //cant use macros here since we need access to the browsers directly
        PERF_LOG( "Creating ServiceBrowser" )
        ServiceBrowser *internetContentServiceBrowser = ServiceBrowser::instance();
        internetContentServiceBrowser->setParent( this );
        m_browsers->addWidget( KIcon( "applications-internet" ), i18n("Internet"), internetContentServiceBrowser );
        m_browserNames.append( "Internet" );
        PERF_LOG( "Created ServiceBrowser" )

        m_playlistFiles = new PlaylistFileProvider();
        The::playlistManager()->addProvider( m_playlistFiles, PlaylistManager::UserPlaylist );

        PERF_LOG( "Creating PlaylistBrowser" )
        addBrowserMacro( PlaylistBrowserNS::PlaylistBrowser, "PlaylistBrowser", i18n("Playlists"), "view-media-playlist-amarok" )
        playlistBrowser()->showCategory( PlaylistManager::Dynamic );
        PERF_LOG( "CreatedPlaylsitBrowser" )

        PERF_LOG( "Creating FileBrowser" )
        addBrowserMacro( FileBrowser::Widget, "FileBrowser::Widget",  i18n("Files"), "folder-amarok" )
        PERF_LOG( "Created FileBrowser" )

        sideBar()->sideBarWidget()->restoreSession();

        PERF_LOG( "Initialising ServicePluginManager" )
        ServicePluginManager::instance()->init();
        PERF_LOG( "Initialised ServicePluginManager" )

        internetContentServiceBrowser->setScriptableServiceManager( The::scriptableServiceManager() );
        PERF_LOG( "ScriptableServiceManager done" )

        #undef addBrowserMacro
        PERF_LOG( "finished MainWindow::init" )
    }
    //</Browsers>

    if( AmarokConfig::panelsSavedState()[0] != -1 )
    {
        QByteArray sPanels;

        foreach( int a, AmarokConfig::panelsSavedState() )
            sPanels.append( a );

        m_splitter->restoreState( sPanels );
    }

    The::amarokUrlHandler(); //Instantiate
}

void
MainWindow::createContextView( Plasma::Containment *containment )
{
    DEBUG_BLOCK
    disconnect( m_corona, SIGNAL( containmentAdded( Plasma::Containment* ) ),
            this, SLOT( createContextView( Plasma::Containment* ) ) );
    PERF_LOG( "Creating ContexView" )
    m_contextView = new Context::ContextView( containment, m_corona, m_contextWidget );
    m_contextView->setFrameShape( QFrame::NoFrame );
    m_contextToolbarView = new Context::ToolbarView( containment, m_corona, m_contextWidget );
    m_contextToolbarView->setFrameShape( QFrame::NoFrame );
    m_contextView->showHome();
    PERF_LOG( "ContexView created" )

    bool hide = AmarokConfig::hideContextView();
    hideContextView( hide );
}

void
MainWindow::deleteBrowsers()
{
    m_browsers->deleteBrowsers();
}

void
MainWindow::slotShrinkBrowsers( int index )
{
    DEBUG_BLOCK

    // Because QSplitter sucks and will not recompute sizes if a pane is shrunk and not hidden.
    if( index == -1 )
    {
        m_splitterState = m_splitter->saveState();

        QList<int> sizes;
        sizes << m_browsers->sideBarWidget()->width() // browser bar
              << m_splitter->sizes()[1] + m_splitter->sizes()[0] - m_browsers->sideBarWidget()->width() // context view
              << m_splitter->sizes()[2]; // playlist
        m_splitter->setSizes( sizes );
    }
    else
    {
        m_splitter->restoreState( m_splitterState );
    }
}

void
MainWindow::addBrowser( const QString &name, QWidget *browser, const QString &text, const QString &icon )
{
    if( !m_browserNames.contains( name ) )
    {
        m_browsers->addWidget( KIcon( icon ), text, browser );
        m_browserNames.append( name );
    }
}

void
MainWindow::showBrowser( const QString &name )
{
    const int index = m_browserNames.indexOf( name );
    showBrowser( index );
}

void
MainWindow::showBrowser( const int index )
{
    if( index >= 0 && index != m_browsers->currentIndex() )
        m_browsers->showWidget( index );
}

void
MainWindow::keyPressEvent( QKeyEvent *e )
{
    if( !( e->modifiers() & Qt::ControlModifier ) )
        return KMainWindow::keyPressEvent( e );

    int n = -1;
    switch( e->key() )
    {
        case Qt::Key_0: n = 0; break;
        case Qt::Key_1: n = 1; break;
        case Qt::Key_2: n = 2; break;
        case Qt::Key_3: n = 3; break;
        case Qt::Key_4: n = 4; break;
        default:
            return KMainWindow::keyPressEvent( e );
    }
    if( n == 0 && m_browsers->currentIndex() >= 0 )
        m_browsers->showWidget( m_browsers->currentIndex() );
    else if( n > 0 )
        showBrowser( n - 1 ); // map from human to computer counting
}

void
MainWindow::closeEvent( QCloseEvent *e )
{
    DEBUG_BLOCK

#ifdef Q_WS_MAC

    Q_UNUSED( e );
    hide();

#else

    //KDE policy states we should hide to tray and not quit() when the
    //close window button is pushed for the main widget

    if( AmarokConfig::showTrayIcon() && e->spontaneous() && !kapp->sessionSaving() )
    {
        KMessageBox::information( this,
                i18n( "<qt>Closing the main window will keep Amarok running in the System Tray. "
                      "Use <B>Quit</B> from the menu, or the Amarok tray icon to exit the application.</qt>" ),
                i18n( "Docking in System Tray" ), "hideOnCloseInfo" );

        hide();
        e->ignore();
        return;
    }

    e->accept();
    kapp->quit();

#endif
}

QSize
MainWindow::sizeHint() const
{
    return QApplication::desktop()->screenGeometry( (QWidget*)this ).size() / 1.5;
}

void
MainWindow::exportPlaylist() const //SLOT
{
    DEBUG_BLOCK

    KFileDialog fileDialog( KUrl("kfiledialog:///amarok-playlist-export"), QString(), 0 );
    QCheckBox *saveRelativeCheck = new QCheckBox( i18n("Use relative path for &saving") );

    fileDialog.fileWidget()->setCustomWidget( saveRelativeCheck );
    fileDialog.setOperationMode( KFileDialog::Saving );
    fileDialog.setMode( KFile::File );
    fileDialog.setCaption( i18n("Save As") );

    fileDialog.exec();

    QString playlistName = fileDialog.selectedFile();

    if( !playlistName.isEmpty() )
    {
        AmarokConfig::setRelativePlaylist( saveRelativeCheck->isChecked() );
        The::playlistModel()->exportPlaylist( playlistName );
    }
}

void
MainWindow::savePlaylist() const
{
    The::playlistModel()->savePlaylist();
}

void
MainWindow::slotShowCoverManager() const //SLOT
{
    CoverManager::showOnce();
}

void
MainWindow::slotPlayMedia() //SLOT
{
    // Request location and immediately start playback
    slotAddLocation( true );
}

void
MainWindow::slotAddLocation( bool directPlay ) //SLOT
{
    // open a file selector to add media to the playlist
    KUrl::List files;
    KFileDialog dlg( KUrl(QString()), QString("*.*|"), this );
    dlg.setCaption( directPlay ? i18n("Play Media (Files or URLs)") : i18n("Add Media (Files or URLs)") );
    dlg.setMode( KFile::Files | KFile::Directory );
    dlg.exec();
    files = dlg.selectedUrls();

    if( files.isEmpty() )
        return;

    The::playlistController()->insertOptioned( files , Playlist::AppendAndPlayImmediately );
}

void
MainWindow::slotAddStream() //SLOT
{
    bool ok;
    QString url = KInputDialog::getText( i18n("Add Stream"), i18n("Enter Stream URL:"), QString(), &ok, this );

    if( !ok )
        return;

    Meta::TrackPtr track = CollectionManager::instance()->trackForUrl( KUrl( url ) );

    The::playlistController()->insertOptioned( track, Playlist::Append );
}

void
MainWindow::playAudioCD() //SLOT
{
    KUrl::List urls;
    if( The::engineController()->getAudioCDContents(QString(), urls) )
    {
        Meta::TrackList tracks = CollectionManager::instance()->tracksForUrls( urls );
        if( !tracks.isEmpty() )
            The::playlistController()->insertOptioned( tracks, Playlist::Replace );
    }
    else
    { // Default behaviour
        showBrowser( "FileBrowser" );
    }
}

void
MainWindow::showScriptSelector() //SLOT
{
    ScriptManager::instance()->show();
    ScriptManager::instance()->raise();
}

/**
 * "Toggle Main Window" global shortcut connects to this slot
 */
void
MainWindow::showHide() //SLOT
{
    setVisible( !isVisible() );
}

void
MainWindow::loveTrack()
{
    Meta::TrackPtr cTrack = The::engineController()->currentTrack();
    if( cTrack )
        emit loveTrack( cTrack );
}

void
MainWindow::activate()
{
#ifdef Q_WS_X11
    const KWindowInfo info = KWindowSystem::windowInfo( winId(), 0, 0 );

    if( KWindowSystem::activeWindow() != winId())
        setVisible( true );
    else if( !info.isMinimized() )
        setVisible( true );
    if( !isHidden() )
        KWindowSystem::activateWindow( winId() );
#else
    setVisible( true );
#endif
}

bool
MainWindow::isReallyShown() const
{
#ifdef Q_WS_X11
    const KWindowInfo info = KWindowSystem::windowInfo( winId(), 0, 0 );
    return !isHidden() && !info.isMinimized() && info.isOnDesktop( KWindowSystem::currentDesktop() );
#else
    return !isHidden();
#endif
}

void
MainWindow::createActions()
{
    KActionCollection* const ac = Amarok::actionCollection();
    const EngineController* const ec = The::engineController();
    const Playlist::Actions* const pa = The::playlistActions();
    const Playlist::Controller* const pc = The::playlistController();

    KStandardAction::keyBindings( kapp, SLOT( slotConfigShortcuts() ), ac );
    KStandardAction::preferences( kapp, SLOT( slotConfigAmarok() ), ac );
    ac->action(KStandardAction::name(KStandardAction::KeyBindings))->setIcon( KIcon( "configure-shortcuts-amarok" ) );
    ac->action(KStandardAction::name(KStandardAction::Preferences))->setIcon( KIcon( "configure-amarok" ) );
    ac->action(KStandardAction::name(KStandardAction::Preferences))->setMenuRole(QAction::PreferencesRole); // Define OS X Prefs menu here, removes need for ifdef later

    KStandardAction::quit( kapp, SLOT( quit() ), ac );

    KAction *action = new KAction( KIcon( "folder-amarok" ), i18n("&Add Media..."), this );
    ac->addAction( "playlist_add", action );
    connect( action, SIGNAL( triggered(bool) ), this, SLOT( slotAddLocation() ) );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_A ) );

    action = new KAction( KIcon( "edit-clear-list-amarok" ), i18nc( "clear playlist", "&Clear Playlist" ), this );
    connect( action, SIGNAL( triggered( bool ) ), pc, SLOT( clear() ) );
    ac->addAction( "playlist_clear", action );

    action = new KAction( KIcon( "folder-amarok" ), i18n("&Add Stream..."), this );
    connect( action, SIGNAL( triggered(bool) ), this, SLOT( slotAddStream() ) );
    ac->addAction( "stream_add", action );

    action = new KAction( KIcon( "document-export-amarok" ), i18n("&Export Playlist As..."), this );
    connect( action, SIGNAL( triggered(bool) ), this, SLOT( exportPlaylist() ) );
    ac->addAction( "playlist_export", action );

    action = new KAction( KIcon( "document-save-amarok" ), i18n("&Save Playlist"), this );
    connect( action, SIGNAL( triggered(bool) ), this, SLOT( savePlaylist() ) );
    ac->addAction( "playlist_save", action );

    action = new KAction( KIcon( "media-album-cover-manager-amarok" ), i18n( "Cover Manager" ), this );
    connect( action, SIGNAL( triggered(bool) ), SLOT( slotShowCoverManager() ) );
    ac->addAction( "cover_manager", action );


//     KAction *update_podcasts = new KAction( this );
//     update_podcasts->setText( i18n( "Update Podcasts" ) );
//     //update_podcasts->setIcon( KIcon("view-refresh-amarok") );
//     ac->addAction( "podcasts_update", update_podcasts );
//     connect(update_podcasts, SIGNAL(triggered(bool)),
//             The::podcastCollection(), SLOT(slotUpdateAll()));

    action = new KAction( KIcon("folder-amarok"), i18n("Play Media..."), this );
    connect(action, SIGNAL(triggered(bool)), SLOT(slotPlayMedia()));
    ac->addAction( "playlist_playmedia", action );

#if 0
    // Audio CD is not currently supported
    action = new KAction( KIcon( "media-optical-audio-amarok" ), i18n("Play Audio CD"), this );
    connect(action, SIGNAL(triggered(bool)), SLOT(playAudioCD()));
    ac->addAction( "play_audiocd", action );
#endif

    action = new KAction( KIcon("preferences-plugin-script-amarok"), i18n("Script Manager"), this );
    connect(action, SIGNAL(triggered(bool)), SLOT(showScriptSelector()));
    ac->addAction( "script_manager", action );

    action = new KAction( KIcon( "media-seek-forward-amarok" ), i18n("&Seek Forward"), this );
    ac->addAction( "seek_forward", action );
    action->setShortcut( Qt::Key_Right );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::SHIFT + Qt::Key_Plus ) );
    connect(action, SIGNAL(triggered(bool)), ec, SLOT(seekForward()));

    action = new KAction( KIcon( "media-seek-backward-amarok" ), i18n("&Seek Backward"), this );
    ac->addAction( "seek_backward", action );
    action->setShortcut( Qt::Key_Left );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::SHIFT + Qt::Key_Minus ) );
    connect(action, SIGNAL(triggered(bool)), ec, SLOT(seekBackward()));

    PERF_LOG( "MainWindow::createActions 6" )
    action = new KAction( KIcon("collection-refresh-amarok"), i18n( "Update Collection" ), this );
    connect(action, SIGNAL(triggered(bool)), CollectionManager::instance(), SLOT(checkCollectionChanges()));
    ac->addAction( "update_collection", action );

    action = new KAction( this );
    ac->addAction( "prev", action );
    action->setIcon( KIcon("media-skip-backward-amarok") );
    action->setText( i18n( "Previous Track" ) );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_Z ) );
    connect( action, SIGNAL(triggered(bool)), pa, SLOT( back() ) );

    action = new KAction( this );
    ac->addAction( "next", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_B ) );
    action->setIcon( KIcon("media-skip-forward-amarok") );
    action->setText( i18n( "Next Track" ) );
    connect( action, SIGNAL(triggered(bool)), pa, SLOT( next() ) );

    action = new KAction( i18n( "Increase Volume" ), this );
    ac->addAction( "increaseVolume", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_Plus ) );
    action->setShortcut( Qt::Key_Plus );
    connect( action, SIGNAL( triggered() ), ec, SLOT( increaseVolume() ) );

    action = new KAction( i18n( "Decrease Volume" ), this );
    ac->addAction( "decreaseVolume", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_Minus ) );
    action->setShortcut( Qt::Key_Minus );
    connect( action, SIGNAL( triggered() ), ec, SLOT( decreaseVolume() ) );

    action = new KAction( i18n( "Toggle Main Window" ), this );
    ac->addAction( "toggleMainWindow", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_P ) );
    connect( action, SIGNAL( triggered() ), SLOT( showHide() ) );

    action = new KAction( i18n( "Show On Screen Display" ), this );
    ac->addAction( "showOsd", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_O ) );
    connect( action, SIGNAL( triggered() ), Amarok::OSD::instance(), SLOT( forceToggleOSD() ) );

    action = new KAction( i18n( "Mute Volume" ), this );
    ac->addAction( "mute", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_M ) );
    connect( action, SIGNAL( triggered() ), ec, SLOT( toggleMute() ) );

    action = new KAction( i18n( "Love Current Track" ), this );
    ac->addAction( "loveTrack", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_L ) );
    connect( action, SIGNAL(triggered()), SLOT(loveTrack()) );

    action = new KAction( i18n( "Rate Current Track: 1" ), this );
    ac->addAction( "rate1", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_1 ) );
    connect( action, SIGNAL( triggered() ), SLOT( setRating1() ) );

    action = new KAction( i18n( "Rate Current Track: 2" ), this );
    ac->addAction( "rate2", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_2 ) );
    connect( action, SIGNAL( triggered() ), SLOT( setRating2() ) );

    action = new KAction( i18n( "Rate Current Track: 3" ), this );
    ac->addAction( "rate3", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_3 ) );
    connect( action, SIGNAL( triggered() ), SLOT( setRating3() ) );

    action = new KAction( i18n( "Rate Current Track: 4" ), this );
    ac->addAction( "rate4", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_4 ) );
    connect( action, SIGNAL( triggered() ), SLOT( setRating4() ) );

    action = new KAction( i18n( "Rate Current Track: 5" ), this );
    ac->addAction( "rate5", action );
    action->setGlobalShortcut( KShortcut( Qt::META + Qt::Key_5 ) );
    connect( action, SIGNAL( triggered() ), SLOT( setRating5() ) );

    action = KStandardAction::redo(pc, SLOT(redo()), this);
    ac->addAction( "playlist_redo", action );
    action->setEnabled(false);
    action->setIcon( KIcon( "edit-redo-amarok" ) );
    connect(pc, SIGNAL(canRedoChanged(bool)), action, SLOT(setEnabled(bool)));

    action = KStandardAction::undo(pc, SLOT(undo()), this);
    ac->addAction( "playlist_undo", action );
    action->setEnabled(false);
    action->setIcon( KIcon( "edit-undo-amarok" ) );
    connect(pc, SIGNAL(canUndoChanged(bool)), action, SLOT(setEnabled(bool)));

    PERF_LOG( "MainWindow::createActions 8" )
    new Amarok::MenuAction( ac, this );
    new Amarok::StopAction( ac, this );
    new Amarok::PlayPauseAction( ac, this );
    new Amarok::RepeatAction( ac, this );
    new Amarok::RandomAction( ac, this );
    new Amarok::FavorAction( ac, this );
    new Amarok::ReplayGainModeAction( ac, this );

    ac->addAssociatedWidget( this );
    foreach (QAction* action, ac->actions())
        action->setShortcutContext(Qt::WindowShortcut);
}

void
MainWindow::setRating( int n )
{
    n *= 2;

    Meta::TrackPtr track = The::engineController()->currentTrack();
    if( track )
    {
        // if we're setting an identical rating then we really must
        // want to set the half-star below rating
        if( track->rating() == n )
            n -= 1;

        track->setRating( n );
        Amarok::OSD::instance()->OSDWidget::ratingChanged( track->rating() );
    }
}

void
MainWindow::createMenus()
{
    //BEGIN Actions menu
    KMenu *actionsMenu;
#ifdef Q_WS_MAC
    m_menubar = new QMenuBar(0);  // Fixes menubar in OS X
    actionsMenu = new KMenu( m_menubar );
    // Add these functions to the dock icon menu in OS X
    //extern void qt_mac_set_dock_menu(QMenu *);
    //qt_mac_set_dock_menu(actionsMenu);
    // Change to avoid duplicate menu titles in OS X
    actionsMenu->setTitle( i18n("&Music") );
#else
    m_menubar = menuBar();
    actionsMenu = new KMenu( m_menubar );
    actionsMenu->setTitle( i18n("&Amarok") );
#endif
    actionsMenu->addAction( Amarok::actionCollection()->action("playlist_playmedia") );
    actionsMenu->addSeparator();
    actionsMenu->addAction( Amarok::actionCollection()->action("prev") );
    actionsMenu->addAction( Amarok::actionCollection()->action("play_pause") );
    actionsMenu->addAction( Amarok::actionCollection()->action("stop") );
    actionsMenu->addAction( Amarok::actionCollection()->action("next") );


#ifndef Q_WS_MAC    // Avoid duplicate "Quit" in OS X dock menu
    actionsMenu->addSeparator();
    actionsMenu->addAction( Amarok::actionCollection()->action(KStandardAction::name(KStandardAction::Quit)) );
#endif
    //END Actions menu

    //BEGIN Playlist menu
    KMenu *playlistMenu = new KMenu( m_menubar );
    playlistMenu->setTitle( i18n("&Playlist") );
    playlistMenu->addAction( Amarok::actionCollection()->action("playlist_add") );
    playlistMenu->addAction( Amarok::actionCollection()->action("stream_add") );
    playlistMenu->addAction( Amarok::actionCollection()->action("playlist_save") );
    playlistMenu->addSeparator();
    playlistMenu->addAction( Amarok::actionCollection()->action("playlist_undo") );
    playlistMenu->addAction( Amarok::actionCollection()->action("playlist_redo") );
    playlistMenu->addSeparator();
    playlistMenu->addAction( Amarok::actionCollection()->action("playlist_clear") );

    QAction *repeat = Amarok::actionCollection()->action("repeat");
    playlistMenu->addAction( repeat );

    KSelectAction *random = static_cast<KSelectAction*>( Amarok::actionCollection()->action("random_mode") );
    playlistMenu->addAction( random );
    random->menu()->addSeparator();
    random->menu()->addAction( Amarok::actionCollection()->action("favor_tracks") );

    playlistMenu->addSeparator();
    //END Playlist menu

    //BEGIN Tools menu
    m_toolsMenu = new KMenu( m_menubar );
    m_toolsMenu->setTitle( i18n("&Tools") );
    m_toolsMenu->addAction( Amarok::actionCollection()->action("cover_manager") );
//FIXME: Reenable when ported//working
//     m_toolsMenu->addAction( Amarok::actionCollection()->action("queue_manager") );
    m_toolsMenu->addAction( Amarok::actionCollection()->action("script_manager") );
    m_toolsMenu->addSeparator();
    m_toolsMenu->addAction( Amarok::actionCollection()->action("update_collection") );
    //END Tools menu

    //BEGIN Settings menu
    m_settingsMenu = new KMenu( m_menubar );
    m_settingsMenu->setTitle( i18n("&Settings") );
    //TODO use KStandardAction or KXmlGuiWindow

    // the phonon-coreaudio  backend has major issues with either the VolumeFaderEffect itself
    // or with it in the pipeline. track playback stops every ~3-4 tracks, and on tracks >5min it
    // stops at about 5:40. while we get this resolved upstream, don't make playing amarok such on osx.
    // so we disable replaygain on osx
#ifndef Q_WS_MAC
    m_settingsMenu->addAction( Amarok::actionCollection()->action("replay_gain_mode") );
    m_settingsMenu->addSeparator();
#endif

    m_settingsMenu->addAction( Amarok::actionCollection()->action(KStandardAction::name(KStandardAction::KeyBindings)) );
    m_settingsMenu->addAction( Amarok::actionCollection()->action(KStandardAction::name(KStandardAction::Preferences)) );
    //END Settings menu

    m_menubar->addMenu( actionsMenu );
    m_menubar->addMenu( playlistMenu );
    m_menubar->addMenu( m_toolsMenu );
    m_menubar->addMenu( m_settingsMenu );
    m_menubar->addMenu( Amarok::Menu::helpMenu() );
}

void
MainWindow::paletteChange(const QPalette & oldPalette)
{
    Q_UNUSED( oldPalette )

    KPixmapCache cache( "Amarok-pixmaps" );
    cache.discard();
    The::paletteHandler()->setPalette( palette() );
}

QSize
MainWindow::backgroundSize()
{
    QPoint topLeft = mapToGlobal( QPoint( 0, 0 ) );
    QPoint bottomRight1 = mapToGlobal( QPoint( width(), height() ) );

    return QSize( bottomRight1.x() - topLeft.x() + 1, bottomRight1.y() - topLeft.y() );
}

int
MainWindow::contextXOffset()
{
    QPoint topLeft1 = mapToGlobal( m_controlBar->pos() );
    QPoint topLeft2 = mapToGlobal( m_contextWidget->pos() );

    return topLeft2.x() - topLeft1.x();
}

void MainWindow::resizeEvent( QResizeEvent * event )
{
    QWidget::resizeEvent( event );
    m_controlBar->reRender();
}

QPoint MainWindow::globalBackgroundOffset()
{
    return menuBar()->mapToGlobal( QPoint( 0, 0 ) );
}

QRect MainWindow::contextRectGlobal()
{
    //debug() << "pos of context vidget within main window is: " << m_contextWidget->pos();
    QPoint contextPos = m_splitter->mapToGlobal( m_contextWidget->pos() );
    return QRect( contextPos.x(), contextPos.y(), m_contextWidget->width(), m_contextWidget->height() );
}

void MainWindow::engineStateChanged( Phonon::State state, Phonon::State oldState )
{
    Q_UNUSED( oldState )
    DEBUG_BLOCK

    debug() << "Phonon state: " << state;

    Meta::TrackPtr track = The::engineController()->currentTrack();
    //track is 0 if the engien state is Empty. we check that in the switch
    switch( state )
    {
    case Phonon::StoppedState:
        setPlainCaption( i18n( AMAROK_CAPTION ) );
        break;

    case Phonon::PlayingState:
        if( track ) {
            unsubscribeFrom( m_currentTrack );
            m_currentTrack = track;
            subscribeTo( track );
            metadataChanged( track );
        }
        else
            warning() << "currentTrack is 0. Can't subscribe to it!";
        break;

    case Phonon::PausedState:
        setPlainCaption( i18n( "Paused  ::  %1", QString( AMAROK_CAPTION ) ) );
        break;

    case Phonon::LoadingState:
    case Phonon::ErrorState:
    case Phonon::BufferingState:
        break;
    }
}

void MainWindow::metadataChanged( Meta::TrackPtr track )
{
    setPlainCaption( i18n( "%1 - %2  ::  %3", track->artist() ? track->artist()->prettyName() : i18n( "Unknown" ), track->prettyName(), AMAROK_CAPTION ) );
}

CollectionWidget * MainWindow::collectionBrowser()
{
    return qobject_cast<CollectionWidget *>( m_browsers->at( 0 ) );
}

QString MainWindow::activeBrowserName()
{
    return m_browserNames[ m_browsers->currentIndex() ];
}

PlaylistBrowserNS::PlaylistBrowser * MainWindow::playlistBrowser()
{
    return qobject_cast<PlaylistBrowserNS::PlaylistBrowser *>( m_browsers->at( 2 ) );
}

void MainWindow::hideContextView( bool hide )
{
    DEBUG_BLOCK
    if ( hide )
        m_contextWidget->hide();
    else
        m_contextWidget->show();
}

namespace The {
    MainWindow* mainWindow() { return MainWindow::s_instance; }
}


#include "MainWindow.moc"



