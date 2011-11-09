/***************************************************************************
                      app.cpp  -  description
                         -------------------
begin                : Mit Okt 23 14:35:18 CEST 2002
copyright            : (C) 2002 by Mark Kretschmann
email                : markey@web.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "App.h"

#include "Amarok.h"
#include "amarokconfig.h"
#include "amarokurls/AmarokUrl.h"
#include "CollectionManager.h"
#include "ConfigDialog.h"
#include "covermanager/CoverFetcher.h"
#include "dbus/CollectionDBusHandler.h"
#include "Debug.h"
#include "EngineController.h"
#include "firstruntutorial/FirstRunTutorial.h"
#include "MainWindow.h"
#include "Meta.h"
#include "meta/MetaConstants.h"
#include "MountPointManager.h"
#include "Osd.h"
#include "PlayerDBusHandler.h"
#include "Playlist.h"
#include "PlaylistFileSupport.h"
#include "playlist/PlaylistActions.h"
#include "playlist/PlaylistModel.h"
#include "playlist/PlaylistController.h"
#include "playlistmanager/PlaylistManager.h"
#include "PluginManager.h"
#include "RootDBusHandler.h"
#include "ScriptManager.h"
#include "statusbar/StatusBar.h"
#include "Systray.h"
#include "TracklistDBusHandler.h"

#include <iostream>

#include <KAboutData>
#include <KAction>
#include <KCalendarSystem>
#include <KCmdLineArgs>                  //initCliArgs()
#include <KEditToolBar>                  //slotConfigToolbars()
#include <KGlobalSettings>
#include <KIO/CopyJob>
#include <KJob>
#include <KJobUiDelegate>
#include <KLocale>
#include <KShortcutsDialog>              //slotConfigShortcuts()
#include <KSplashScreen>
#include <KStandardDirs>

#include <QByteArray>
#include <QFile>
#include <KPixmapCache>
#include <QStringList>
#include <QTimer>                       //showHyperThreadingWarning()
#include <QtDBus/QtDBus>

#ifdef TAGLIB_EXTRAS_FOUND
#include <audiblefiletyperesolver.h>
#include <asffiletyperesolver.h>
#include <wavfiletyperesolver.h>
#include <realmediafiletyperesolver.h>
#include <mp4filetyperesolver.h>
#endif

QMutex Debug::mutex;
QMutex Amarok::globalDirsMutex;
QPointer<KActionCollection> Amarok::actionCollectionObject;

int App::mainThreadId = 0;

#ifdef Q_WS_MAC
#include <CoreFoundation/CoreFoundation.h>
extern void setupEventHandler_mac(long);
#endif


AMAROK_EXPORT KAboutData aboutData( "amarok", 0,
    ki18n( "Amarok" ), APP_VERSION,
    ki18n( "The audio player for KDE" ), KAboutData::License_GPL,
    ki18n( "(C) 2002-2003, Mark Kretschmann\n(C) 2003-2009, The Amarok Development Squad" ),
    ki18n( "IRC:\nirc.freenode.net - #amarok, #amarok.de, #amarok.es, #amarok.fr\n\nFeedback:\namarok@kde.org\n\n(Build Date: %1)" ).subs( __DATE__ ),
             ( "http://amarok.kde.org" ) );

App::App()
        : KUniqueApplication()
        , m_splash( 0 )
{
    DEBUG_BLOCK
    PERF_LOG( "Begin Application ctor" )

    // required for last.fm plugin to grab app version
    setApplicationVersion( APP_VERSION );

    if( AmarokConfig::showSplashscreen() && !isSessionRestored() )
    {
        PERF_LOG( "Init KStandardDirs cache" )
        KStandardDirs *stdDirs = KGlobal::dirs();
        PERF_LOG( "Finding image" )
        QString img = stdDirs->findResource( "data", "amarok/images/splash_screen.jpg" );
        PERF_LOG( "Creating pixmap" )
        QPixmap splashpix( img );
        PERF_LOG( "Creating splashscreen" )
        m_splash = new KSplashScreen( splashpix, Qt::WindowStaysOnTopHint );
        PERF_LOG( "showing splashscreen" )
        m_splash->show();
    }

#ifdef TAGLIB_EXTRAS_FOUND
    PERF_LOG( "Registering taglib plugins" )
    TagLib::FileRef::addFileTypeResolver(new MP4FileTypeResolver);
    TagLib::FileRef::addFileTypeResolver(new ASFFileTypeResolver);
    TagLib::FileRef::addFileTypeResolver(new RealMediaFileTypeResolver);
    TagLib::FileRef::addFileTypeResolver(new AudibleFileTypeResolver);
    TagLib::FileRef::addFileTypeResolver(new WavFileTypeResolver);
    PERF_LOG( "Done Registering taglib plugins" )
#endif

    qRegisterMetaType<Meta::DataPtr>();
    qRegisterMetaType<Meta::DataList>();
    qRegisterMetaType<Meta::TrackPtr>();
    qRegisterMetaType<Meta::TrackList>();
    qRegisterMetaType<Meta::AlbumPtr>();
    qRegisterMetaType<Meta::AlbumList>();
    qRegisterMetaType<Meta::ArtistPtr>();
    qRegisterMetaType<Meta::ArtistList>();
    qRegisterMetaType<Meta::GenrePtr>();
    qRegisterMetaType<Meta::GenreList>();
    qRegisterMetaType<Meta::ComposerPtr>();
    qRegisterMetaType<Meta::ComposerList>();
    qRegisterMetaType<Meta::YearPtr>();
    qRegisterMetaType<Meta::YearList>();


    //make sure we have enough cache space for all our crazy svg stuff
    KPixmapCache cache( "Amarok-pixmaps" );
    cache.setCacheLimit ( 20 * 1024 );

#ifdef Q_WS_MAC
    // this is inspired by OpenSceneGraph: osgDB/FilePath.cpp

    // Start with the the Bundle PlugIns directory.

    // Get the main bundle first. No need to retain or release it since
    //  we are not keeping a reference
    CFBundleRef myBundle = CFBundleGetMainBundle();
    if( myBundle )
    {
        // CFBundleGetMainBundle will return a bundle ref even if
        //  the application isn't part of a bundle, so we need to
        //  check
        //  if the path to the bundle ends in ".app" to see if it is
        //  a
        //  proper application bundle. If it is, the plugins path is
        //  added
        CFURLRef urlRef = CFBundleCopyBundleURL(myBundle);
        if(urlRef)
        {
            char bundlePath[1024];
            if( CFURLGetFileSystemRepresentation( urlRef, true, (UInt8 *)bundlePath, sizeof(bundlePath) ) )
            {
                QByteArray bp( bundlePath );
                size_t len = bp.length();
                if( len > 4 && bp.right( 4 ) == ".app" )
                {
                    bp.append( "/Contents/MacOS" );
                    QByteArray path = qgetenv( "PATH" );
                    if( path.length() > 0 )
                    {
                        path.prepend( ":" );
                    }
                    path.prepend( bp );
                    debug() << "setting PATH=" << path;
                    setenv("PATH", path, 1);
                }
            }
            // docs say we are responsible for releasing CFURLRef
            CFRelease(urlRef);
        }
    }

    setupEventHandler_mac((long)this);
#endif

    PERF_LOG( "Done App ctor" )

    continueInit();
}

App::~App()
{
    DEBUG_BLOCK

    delete m_splash;
    m_splash = 0;

    CollectionManager::instance()->stopScan();

    // Hiding the OSD before exit prevents crash
    Amarok::OSD::instance()->hide();

    if ( AmarokConfig::resumePlayback() )
    {
        if ( The::engineController()->state() != Phonon::StoppedState )
        {
            Meta::TrackPtr track = The::engineController()->currentTrack();
            if( track )
            {
                AmarokConfig::setResumeTrack( track->playableUrl().prettyUrl() );
                AmarokConfig::setResumeTime( The::engineController()->trackPosition() * 1000 );
                AmarokConfig::setLastPlaying( Playlist::Model::instance()->activeRow() );
            }
        }
        else
        {
            AmarokConfig::setResumeTrack( QString() ); //otherwise it'll play previous resume next time!
            AmarokConfig::setLastPlaying( -1 );
        }
    }

    The::engineController()->endSession(); //records final statistics

#ifndef Q_WS_MAC
    // do even if trayicon is not shown, it is safe
    Amarok::config().writeEntry( "HiddenOnExit", mainWindow()->isHidden() );
    AmarokConfig::self()->writeConfig();
#else
    // for some reason on OS X the main window always reports being hidden
    // this means if you have the tray icon enabled, amarok will always open minimized
    Amarok::config().writeEntry( "HiddenOnExit", false );
    AmarokConfig::self()->writeConfig();
#endif

    ScriptManager::destroy();

    // this must be deleted before the connection to the Xserver is
    // severed, or we risk a crash when the QApplication is exited,
    // I asked Trolltech! *smug*
    Amarok::OSD::destroy();

    // I tried this in the destructor for the Model but the object is destroyed after the
    // Config is written. Go figure!
    AmarokConfig::setLastPlaying( Playlist::Model::instance()->rowForTrack( Playlist::Model::instance()->activeTrack() ) );

    AmarokConfig::self()->writeConfig();

    mainWindow()->deleteBrowsers();
    delete mainWindow();

    CollectionManager::destroy();
    MountPointManager::destroy();
    Playlist::Actions::destroy();
    Playlist::Model::destroy();
    PlaylistManager::destroy();
    EngineController::destroy();
    CoverFetcher::destroy();


#ifdef Q_WS_WIN
    // work around for KUniqueApplication being not completely implemented on windows
    QDBusConnectionInterface* dbusService;
    if (QDBusConnection::sessionBus().isConnected() && (dbusService = QDBusConnection::sessionBus().interface()))
    {
        dbusService->unregisterService("org.mpris.amarok");
    }
#endif
}

void
App::handleCliArgs() //static
{
    DEBUG_BLOCK

    KCmdLineArgs* const args = KCmdLineArgs::parsedArgs();

    if( args->isSet( "cwd" ) )
        KCmdLineArgs::setCwd( args->getOption( "cwd" ).toLocal8Bit() );

    bool haveArgs = false;
    if( args->count() > 0 )
    {
        haveArgs = true;

        KUrl::List list;
        for( int i = 0; i < args->count(); i++ )
        {
            KUrl url = args->url( i );
            //TODO:PORTME
//             if( url.protocol() == "itpc" || url.protocol() == "pcast" )
//                 PlaylistBrowserNS::instance()->addPodcast( url );
//             else

            if ( url.protocol() == "amarok" ) {

                AmarokUrl aUrl( url.url() );
                aUrl.run();

            } else {
                list << url;
                DEBUG_LINE_INFO
            }
        }

        int options = Playlist::AppendAndPlay;
        if( args->isSet( "queue" ) )
           options = Playlist::Queue;
        else if( args->isSet( "append" ) )
           options = Playlist::Append;
        else if( args->isSet( "load" ) )
            options = Playlist::Replace;

        if( args->isSet( "play" ) )
            options |= Playlist::DirectPlay;

        The::playlistController()->insertOptioned( list, options );
    }

    //we shouldn't let the user specify two of these since it is pointless!
    //so we prioritise, pause > stop > play > next > prev
    //thus pause is the least destructive, followed by stop as brakes are the most important bit of a car(!)
    //then the others seemed sensible. Feel free to modify this order, but please leave justification in the cvs log
    //I considered doing some sanity checks (eg only stop if paused or playing), but decided it wasn't worth it
    else if ( args->isSet( "pause" ) )
    {
        haveArgs = true;
        The::engineController()->pause();
    }
    else if ( args->isSet( "stop" ) )
    {
        haveArgs = true;
        The::engineController()->stop();
    }
    else if ( args->isSet( "play-pause" ) )
    {
        haveArgs = true;
        The::engineController()->playPause();
    }
    else if ( args->isSet( "play" ) ) //will restart if we are playing
    {
        haveArgs = true;
        The::engineController()->play();
    }
    else if ( args->isSet( "next" ) )
    {
        haveArgs = true;
        The::playlistActions()->next();
    }
    else if ( args->isSet( "previous" ) )
    {
        haveArgs = true;
        The::playlistActions()->back();
    }
    /*
    else if (args->isSet("cdplay"))
    {
        haveArgs = true;
        QString device = args->getOption("cdplay");
        KUrl::List urls;
        if (The::engineController()->getAudioCDContents(device, urls))
        {
            Meta::TrackList tracks = CollectionManager::instance()->tracksForUrls( urls );
            The::playlistController()->insertOptioned( tracks, Playlist::Replace|Playlist::DirectPlay );
        }
        else
        { // Default behaviour
            debug() << "Sorry, the engine does not support direct play from AudioCD...";
        }
    }
    */

    static bool firstTime = true;
    const bool debugWasJustEnabled = !Amarok::config().readEntry( "Debug Enabled", false ) && args->isSet( "debug" );
    const bool debugIsDisabled = !args->isSet( "debug" );
    //allows debugging on OS X. Bundles have to be started with "open". Therefore it is not possible to pass an argument
    const bool forceDebug = Amarok::config().readEntry( "Force Debug", false );
    

    Amarok::config().writeEntry( "Debug Enabled", forceDebug ? true : args->isSet( "debug" ) );

    // Debug output will only work from this point on. If Amarok was run without debug output before,
    // then a part of the output (until this point) will be missing. Inform the user about this:
    if( debugWasJustEnabled || forceDebug )
    {
        debug() << "************************************************************************************************************";
        debug() << "** DEBUGGING OUTPUT IS NOW ENABLED. PLEASE NOTE THAT YOU WILL ONLY SEE THE FULL OUTPUT ON THE NEXT START. **";
        debug() << "************************************************************************************************************";
    }
    else if( firstTime && debugIsDisabled )
    {
        Amarok::config().writeEntry( "Debug Enabled", true );
        debug() << "**********************************************************************************************";
        debug() << "** AMAROK WAS STARTED IN NORMAL MODE. IF YOU WANT TO SEE DEBUGGING INFORMATION, PLEASE USE: **";
        debug() << "** amarok --debug                                                                           **";
        debug() << "**********************************************************************************************";
        Amarok::config().writeEntry( "Debug Enabled", false );
    }

    if( !firstTime && !haveArgs )
        pApp->mainWindow()->activate();
    firstTime = false;

    args->clear();    //free up memory
}


/////////////////////////////////////////////////////////////////////////////////////
// INIT
/////////////////////////////////////////////////////////////////////////////////////

void
App::initCliArgs( int argc, char *argv[] )
{
    KCmdLineArgs::reset();
    KCmdLineArgs::init( argc, argv, &::aboutData ); //calls KCmdLineArgs::addStdCmdLineOptions()
    initCliArgs();
}

void
App::initCliArgs() //static
{
    KCmdLineOptions options;

    options.add("+[URL(s)]", ki18n( "Files/URLs to open" ));
    options.add("r");
    options.add("previous", ki18n( "Skip backwards in playlist" ));
    options.add("p");
    options.add("play", ki18n( "Start playing current playlist" ));
    options.add("t");
    options.add("play-pause", ki18n( "Play if stopped, pause if playing" ));
    options.add("pause", ki18n( "Pause playback" ));
    options.add("s");
    options.add("stop", ki18n( "Stop playback" ));
    options.add("f");
    options.add("next", ki18n( "Skip forwards in playlist" ));
    options.add(":", ki18n("Additional options:"));
    options.add("a");
    options.add("append", ki18n( "Append files/URLs to playlist" ));
    options.add("queue", ki18n("Queue URLs after the currently playing track"));
    options.add("l");
    options.add("load", ki18n("Load URLs, replacing current playlist"));
    options.add("d");
    options.add("debug", ki18n("Print verbose debugging information"));
    options.add("m");
    options.add("multipleinstances", ki18n("Allow running multiple Amarok instances"));
    options.add("cwd <directory>", ki18n( "Base for relative filenames/URLs" ));
    //options.add("cdplay <device>", ki18n("Play an AudioCD from <device> or system:/media/<device>"));

    KCmdLineArgs::addCmdLineOptions( options );   //add our own options
}


/////////////////////////////////////////////////////////////////////////////////////
// METHODS
/////////////////////////////////////////////////////////////////////////////////////

#include <id3v1tag.h>
#include <tbytevector.h>
#include <QTextCodec>
#include <KGlobal>

//this class is only used in this module, so I figured I may as well define it
//here and save creating another header/source file combination

// Local version of taglib's QStringToTString macro. It is here, because taglib's one is
// not Qt3Support clean (uses QString::utf8()). Once taglib will be clean of qt3support
// it is safe to use QStringToTString again
#define Qt4QStringToTString(s) TagLib::String(s.toUtf8().data(), TagLib::String::UTF8)

class ID3v1StringHandler : public TagLib::ID3v1::StringHandler
{
    QTextCodec *m_codec;

    virtual TagLib::String parse( const TagLib::ByteVector &data ) const
    {
        return Qt4QStringToTString( m_codec->toUnicode( data.data(), data.size() ) );
    }

    virtual TagLib::ByteVector render( const TagLib::String &ts ) const
    {
        const QByteArray qcs = m_codec->fromUnicode( TStringToQString(ts) );
        return TagLib::ByteVector( qcs, (uint) qcs.length() );
    }

public:
    ID3v1StringHandler( int codecIndex )
            : m_codec( QTextCodec::codecForName( QTextCodec::availableCodecs().at( codecIndex ) ) )
    {
        debug() << "codec: " << m_codec;
        debug() << "codec-name: " << m_codec->name();
    }

    ID3v1StringHandler( QTextCodec *codec )
            : m_codec( codec )
    {
        debug() << "codec: " << m_codec;
        debug() << "codec-name: " << m_codec->name();
    }

    virtual ~ID3v1StringHandler()
    {}
};

#undef Qt4QStringToTString

//SLOT
void App::applySettings( bool firstTime )
{
    ///Called when the configDialog is closed with OK or Apply

    DEBUG_BLOCK

    Amarok::OSD::instance()->applySettings();
    m_tray->setVisible( AmarokConfig::showTrayIcon() );

    //on startup we need to show the window, but only if it wasn't hidden on exit
    //and always if the trayicon isn't showing
    QWidget* main_window = mainWindow();

    if( ( main_window && firstTime && !Amarok::config().readEntry( "HiddenOnExit", false ) ) || ( main_window && !AmarokConfig::showTrayIcon() ) )
    {
        PERF_LOG( "showing main window again" )
        main_window->show();
        PERF_LOG( "after showing mainWindow" )
    }

    { //<Engine>
        if( The::engineController()->volume() != AmarokConfig::masterVolume() )
        {
            // block signals to make sure that the OSD isn't shown
            const bool osdEnabled = Amarok::OSD::instance()->isEnabled();
            Amarok::OSD::instance()->setEnabled( false );
            The::engineController()->setVolume( AmarokConfig::masterVolume() );
            Amarok::OSD::instance()->setEnabled( osdEnabled );
        }

        if( The::engineController()->isMuted() != AmarokConfig::muteState() )
            The::engineController()->setMuted( AmarokConfig::muteState() );

#if 0
    // Audio CD is not currently supported
    Amarok::actionCollection()->action( "play_audiocd" )->setEnabled( false );
#endif

    } //</Engine>

    {   // delete unneeded cover images from cache
        PERF_LOG( "Begin cover handling" )
        const QString size = QString::number( 100 ) + '@';
        const QDir cacheDir = Amarok::saveLocation( "albumcovers/cache/" );
        const QStringList obsoleteCovers = cacheDir.entryList( QStringList("*") );
        foreach( const QString &it, obsoleteCovers )
            //34@ is for playlist album cover images
            if ( !it.startsWith( size  ) && !it.startsWith( "50@" ) && !it.startsWith( "32@" ) && !it.startsWith( "34@" ) )
                QFile( cacheDir.filePath( it ) ).remove();

        PERF_LOG( "done cover handling" )
    }

    // show or hide CV
    if( mainWindow() )
        mainWindow()->hideContextView( AmarokConfig::hideContextView() );
    //if ( !firstTime )
        // Bizarrely and ironically calling this causes crashes for
        // some people! FIXME
        //AmarokConfig::self()->writeConfig();
}

//SLOT
void
App::continueInit()
{
    DEBUG_BLOCK

    PERF_LOG( "Begin App::continueInit" )
    const KCmdLineArgs* const args = KCmdLineArgs::parsedArgs();
    const bool restoreSession = args->count() == 0 || args->isSet( "append" ) || args->isSet( "queue" )
                                || Amarok::config().readEntry( "AppendAsDefault", false );

    QTextCodec* utf8codec = QTextCodec::codecForName( "UTF-8" );
    QTextCodec::setCodecForCStrings( utf8codec ); //We need this to make CollectionViewItem showing the right charecters.

    PERF_LOG( "Creating MainWindow" )
    m_mainWindow = new MainWindow();
    PERF_LOG( "Done creating MainWindow" )

    m_tray = new Amarok::TrayIcon( mainWindow() );

    PERF_LOG( "Creating DBus handlers" )
    new Amarok::RootDBusHandler();
    new Amarok::PlayerDBusHandler();
    new Amarok::TracklistDBusHandler();
    new CollectionDBusHandler( this );
    QDBusConnection::sessionBus().registerService("org.mpris.amarok");
    PERF_LOG( "Done creating DBus handlers" )
    //DON'T DELETE THIS NEXT LINE or the app crashes when you click the X (unless we reimplement closeEvent)
    //Reason: in ~App we have to call the deleteBrowsers method or else we run afoul of refcount foobar in KHTMLPart
    //But if you click the X (not Action->Quit) it automatically kills MainWindow because KMainWindow sets this
    //for us as default (bad KMainWindow)
    mainWindow()->setAttribute( Qt::WA_DeleteOnClose, false );
    //init playlist window as soon as the database is guaranteed to be usable

    //create engine, show TrayIcon etc.
    applySettings( true );
    // Start ScriptManager. Must be created _after_ MainWindow.
    PERF_LOG( "Starting ScriptManager" )
    ScriptManager::instance();
    PERF_LOG( "ScriptManager started" )

    if ( AmarokConfig::resumePlayback() && restoreSession && !args->isSet( "stop" ) ) {
        //restore session as long as the user didn't specify media to play etc.
        //do this after applySettings() so OSD displays correctly
        The::engineController()->restoreSession();
    }

    if( AmarokConfig::monitorChanges() )
        CollectionManager::instance()->checkCollectionChanges();

    // Restore keyboard shortcuts etc from config
    Amarok::actionCollection()->readSettings();

    delete m_splash;
    m_splash = 0;
    PERF_LOG( "App init done" )
    KConfigGroup config = KGlobal::config()->group( "General" );

    // NOTE: First Run Tutorial disabled for 2.1-beta1 release (too buggy / unfinished)
#if 0
    const bool firstruntut = config.readEntry( "FirstRunTutorial", true );
    debug() << "Checking whether to run first run tutorial..." << firstruntut;
    if( firstruntut )
    {
        config.writeEntry( "FirstRunTutorial", false );
        debug() << "Starting first run tutorial";
        FirstRunTutorial *frt = new FirstRunTutorial( mainWindow() );
        QTimer::singleShot( 1000, frt, SLOT( initOverlay() ) );
    }

    if( config.readEntry( "First Run", true ) )
    {
        slotConfigAmarok( "CollectionConfig" );
        config.writeEntry( "First Run", false );
    }
#endif
}

void App::slotConfigEqualizer() //SLOT
{
//    PORT 2.0
//    EqualizerSetup::instance()->show();
//    EqualizerSetup::instance()->raise();
}

void App::slotConfigAmarok( const QString& page )
{
    Amarok2ConfigDialog* dialog = static_cast<Amarok2ConfigDialog*>( KConfigDialog::exists( "settings" ) );

    if( !dialog )
    {
        //KConfigDialog didn't find an instance of this dialog, so lets create it :
        dialog = new Amarok2ConfigDialog( mainWindow(), "settings", AmarokConfig::self() );

        connect( dialog, SIGNAL( settingsChanged( const QString& ) ), SLOT( applySettings() ) );
    }

    dialog->show( page );
}

void App::slotConfigShortcuts()
{
    KShortcutsDialog::configure( Amarok::actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, mainWindow() );
    AmarokConfig::self()->writeConfig();
}

KIO::Job *App::trashFiles( const KUrl::List &files )
{
    KIO::Job *job = KIO::trash( files );
    The::statusBar()->newProgressOperation( job, i18n("Moving files to trash") );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotTrashResult( KJob* ) ) );
    return job;
}

void App::slotTrashResult( KJob *job )
{
    if( job->error() )
        job->uiDelegate()->showErrorMessage();
}

void App::quit()
{
    emit prepareToQuit();
    /*
    if( MediaBrowser::instance() && MediaBrowser::instance()->blockQuit() )
    {
        // don't quit yet, as some media devices still have to finish transferring data
        QTimer::singleShot( 100, this, SLOT( quit() ) );
        return;
    }
    */
    KApplication::quit();
}

bool App::event( QEvent *event )
{

    switch( event->type() )
    {
        //allows Amarok to open files from the finder on OS X
        case QEvent::FileOpen:
        {
            QString file = static_cast<QFileOpenEvent*>( event )->file();
            //we are only going to receive local files here
            KUrl url( file );
            if( PlaylistManager::instance()->isPlaylist( url ) )
            {
                Meta::PlaylistPtr playlist = Meta::loadPlaylist( url );
                The::playlistController()->insertOptioned( playlist, Playlist::AppendAndPlay );
            }
            else
            {
                Meta::TrackPtr track = CollectionManager::instance()->trackForUrl( url );
                The::playlistController()->insertOptioned( track, Playlist::AppendAndPlay );
            }
            return true;
        }
        default:
            return KUniqueApplication::event( event );
    }
}

namespace Amarok
{
    /// @see amarok.h

    /*
    * Transform to be usable within HTML/HTML attributes
    */
    QString escapeHTMLAttr( const QString &s )
    {
        return QString(s).replace( '%', "%25" ).replace( '\'', "%27" ).replace( '"', "%22" ).
                replace( '#', "%23" ).replace( '?', "%3F" );
    }
    QString unescapeHTMLAttr( const QString &s )
    {
        return QString(s).replace( "%3F", "?" ).replace( "%23", "#" ).replace( "%22", "\"" ).
                replace( "%27", "'" ).replace( "%25", "%" );
    }

    /**
     * Function that must be used when separating contextBrowser escaped urls
     * detail can contain track/discnumber
     */
    void albumArtistTrackFromUrl( QString url, QString &artist, QString &album, QString &detail )
    {
        if ( !url.contains("@@@") ) return;
        //KHTML removes the trailing space!
        if ( url.endsWith( " @@@" ) )
            url += ' ';

        const QStringList list = url.split( " @@@ ", QString::KeepEmptyParts );

        int size = list.count();

        if( size<=0 )
            error() << "size<=0";

        artist = size > 0 ? unescapeHTMLAttr( list[0] ) : "";
        album  = size > 1 ? unescapeHTMLAttr( list[1] ) : "";
        detail = size > 2 ? unescapeHTMLAttr( list[2] ) : "";
    }

    QString verboseTimeSince( const QDateTime &datetime )
    {
        const QDateTime now = QDateTime::currentDateTime();
        const int datediff = datetime.daysTo( now );

        if( datediff >= 6*7 /*six weeks*/ ) {  // return absolute month/year
            const KCalendarSystem *cal = KGlobal::locale()->calendar();
            const QDate date = datetime.date();
            return i18nc( "monthname year", "%1 %2", cal->monthName(date),
                          cal->yearString(date, KCalendarSystem::LongFormat) );
        }

        //TODO "last week" = maybe within 7 days, but prolly before last sunday

        if( datediff >= 7 )  // return difference in weeks
            return i18np( "One week ago", "%1 weeks ago", (datediff+3)/7 );

        if( datediff == -1 )
            return i18nc( "When this track was last played", "Tomorrow" );

        const int timediff = datetime.secsTo( now );

        if( timediff >= 24*60*60 /*24 hours*/ )  // return difference in days
            return datediff == 1 ?
                    i18n( "Yesterday" ) :
                    i18np( "One day ago", "%1 days ago", (timediff+12*60*60)/(24*60*60) );

        if( timediff >= 90*60 /*90 minutes*/ )  // return difference in hours
            return i18np( "One hour ago", "%1 hours ago", (timediff+30*60)/(60*60) );

        //TODO are we too specific here? Be more fuzzy? ie, use units of 5 minutes, or "Recently"

        if( timediff >= 0 )  // return difference in minutes
            return timediff/60 ?
                    i18np( "One minute ago", "%1 minutes ago", (timediff+30)/60 ) :
                    i18n( "Within the last minute" );

        return i18n( "The future" );
    }

    QString verboseTimeSince( uint time_t )
    {
        if( !time_t )
            return i18nc( "The amount of time since last played", "Never" );

        QDateTime dt;
        dt.setTime_t( time_t );
        return verboseTimeSince( dt );
    }

    QString conciseTimeSince( uint time_t )
    {
        if( !time_t )
            return i18nc( "The amount of time since last played", "0" );

        QDateTime datetime;
        datetime.setTime_t( time_t );

        const QDateTime now = QDateTime::currentDateTime();
        const int datediff = datetime.daysTo( now );

        if( datediff >= 6*7 /*six weeks*/ ) {  // return difference in months
            return i18nc( "number of months ago", "%1M", datediff/7/4 );
        }

        if( datediff >= 7 )  // return difference in weeks
            return i18nc( "w for weeks", "%1w", (datediff+3)/7 );

        if( datediff == -1 )
            return i18nc( "When this track was last played", "Tomorrow" );

        const int timediff = datetime.secsTo( now );

        if( timediff >= 24*60*60 /*24 hours*/ )  // return difference in days
            // xgettext: no-c-format
            return i18nc( "d for days", "%1d", (timediff+12*60*60)/(24*60*60) );

        if( timediff >= 90*60 /*90 minutes*/ )  // return difference in hours
            return i18nc( "h for hours", "%1h", (timediff+30*60)/(60*60) );

        //TODO are we too specific here? Be more fuzzy? ie, use units of 5 minutes, or "Recently"

        if( timediff >= 60 )  // return difference in minutes
            return QString("%1'").arg( ( timediff + 30 )/60 );
        if( timediff >= 0 )  // return difference in seconds
            return QString("%1\"").arg( ( timediff + 1 )/60 );

        return i18n( "0" );

    }

    QWidget *mainWindow()
    {
        return pApp->mainWindow();
    }

    KActionCollection* actionCollection()  // TODO: constify?
    {
        if( !actionCollectionObject )
        {
            actionCollectionObject = new KActionCollection( pApp );
            actionCollectionObject->setObjectName( "Amarok-KActionCollection" );
        }

        return actionCollectionObject;
    }

    KConfigGroup config( const QString &group )
    {
        //Slightly more useful config() that allows setting the group simultaneously
        return KGlobal::config()->group( group );
    }

    namespace ColorScheme
    {
        QColor Base;
        QColor Text;
        QColor Background;
        QColor Foreground;
        QColor AltBase;
    }

    OverrideCursor::OverrideCursor( Qt::CursorShape cursor )
    {
        QApplication::setOverrideCursor( cursor == Qt::WaitCursor ?
                                        Qt::WaitCursor :
                                        Qt::BusyCursor );
    }

    OverrideCursor::~OverrideCursor()
    {
        QApplication::restoreOverrideCursor();
    }

    QString saveLocation( const QString &directory )
    {
        globalDirsMutex.lock();
        QString result = KGlobal::dirs()->saveLocation( "data", QString("amarok/") + directory, true );
        globalDirsMutex.unlock();
        return result;
    }

    QString cleanPath( const QString &path )
    {
        QString result = path;
        // german umlauts
        result.replace( QChar(0x00e4), "ae" ).replace( QChar(0x00c4), "Ae" );
        result.replace( QChar(0x00f6), "oe" ).replace( QChar(0x00d6), "Oe" );
        result.replace( QChar(0x00fc), "ue" ).replace( QChar(0x00dc), "Ue" );
        result.replace( QChar(0x00df), "ss" );

        // some strange accents
        result.replace( QChar(0x00e7), "c" ).replace( QChar(0x00c7), "C" );
        result.replace( QChar(0x00fd), "y" ).replace( QChar(0x00dd), "Y" );
        result.replace( QChar(0x00f1), "n" ).replace( QChar(0x00d1), "N" );

        // czech letters with carons
        result.replace( QChar(0x0161), "s" ).replace( QChar(0x0160), "S" );
        result.replace( QChar(0x010d), "c" ).replace( QChar(0x010c), "C" );
        result.replace( QChar(0x0159), "r" ).replace( QChar(0x0158), "R" );
        result.replace( QChar(0x017e), "z" ).replace( QChar(0x017d), "Z" );
        result.replace( QChar(0x0165), "t" ).replace( QChar(0x0164), "T" );
        result.replace( QChar(0x0148), "n" ).replace( QChar(0x0147), "N" );
        result.replace( QChar(0x010f), "d" ).replace( QChar(0x010e), "D" );

        // accented vowels
        QChar a[] = { 'a', 0xe0,0xe1,0xe2,0xe3,0xe5, 0 };
        QChar A[] = { 'A', 0xc0,0xc1,0xc2,0xc3,0xc5, 0 };
        QChar E[] = { 'e', 0xe8,0xe9,0xea,0xeb,0x11a, 0 };
        QChar e[] = { 'E', 0xc8,0xc9,0xca,0xcb,0x11b, 0 };
        QChar i[] = { 'i', 0xec,0xed,0xee,0xef, 0 };
        QChar I[] = { 'I', 0xcc,0xcd,0xce,0xcf, 0 };
        QChar o[] = { 'o', 0xf2,0xf3,0xf4,0xf5,0xf8, 0 };
        QChar O[] = { 'O', 0xd2,0xd3,0xd4,0xd5,0xd8, 0 };
        QChar u[] = { 'u', 0xf9,0xfa,0xfb,0x16e, 0 };
        QChar U[] = { 'U', 0xd9,0xda,0xdb,0x16f, 0 };
        QChar nul[] = { 0 };
        QChar *replacements[] = { a, A, e, E, i, I, o, O, u, U, nul };

        for( int i = 0; i < result.length(); i++ )
        {
            QChar c = result[ i ];
            for( uint n = 0; replacements[n][0] != QChar(0); n++ )
            {
                for( uint k=0; replacements[n][k] != QChar(0); k++ )
                {
                    if( replacements[n][k] == c )
                    {
                        c = replacements[n][0];
                    }
                }
            }
            result[ i ] = c;
        }
        return result;
    }

    QString asciiPath( const QString &path )
    {
        QString result = path;
        for( int i = 0; i < result.length(); i++ )
        {
            QChar c = result[ i ];
            if( c > QChar(0x7f) || c == QChar(0) )
            {
                c = '_';
            }
            result[ i ] = c;
        }
        return result;
    }

    QString vfatPath( const QString &path )
    {
        QString s = path;

        for( int i = 0; i < s.length(); i++ )
        {
            QChar c = s[ i ];
            if( c < QChar(0x20)
                    || c=='*' || c=='?' || c=='<' || c=='>'
                    || c=='|' || c=='"' || c==':' || c=='/'
                    || c=='\\' )
                c = '_';
            s[ i ] = c;
        }

        uint len = s.length();
        if( len == 3 || (len > 3 && s[3] == '.') )
        {
            QString l = s.left(3).toLower();
            if( l=="aux" || l=="con" || l=="nul" || l=="prn" )
                s = '_' + s;
        }
        else if( len == 4 || (len > 4 && s[4] == '.') )
        {
            QString l = s.left(3).toLower();
            QString d = s.mid(3,1);
            if( (l=="com" || l=="lpt") &&
                    (d=="0" || d=="1" || d=="2" || d=="3" || d=="4" ||
                     d=="5" || d=="6" || d=="7" || d=="8" || d=="9") )
                s = '_' + s;
        }

        while( s.startsWith( '.' ) )
            s = s.mid(1);

        while( s.endsWith( '.' ) )
            s = s.left( s.length()-1 );

        s = s.left(255);
        len = s.length();
        if( s[len-1] == ' ' )
            s[len-1] = '_';

        return s;
    }

    /* Strip the common prefix of two strings from the first one and trim
     * whitespaces from the beginning of the resultant string.
     * Case-insensitive.
     *
     * @param input the string being processed
     * @param ref the string used to determine prefix
     */
    QString decapitateString( const QString &input, const QString &ref )
    {
        int len;    //the length of common prefix calculated so far
        for ( len = 0; len < input.length() && len < ref.length(); len++ )
        {
            if ( input.at( len ).toUpper() != ref.at( len ).toUpper() )
                break;
        }

        return input.right( input.length() - len ).trimmed();
    }

    KIO::Job *trashFiles( const KUrl::List &files ) { return App::instance()->trashFiles( files ); }
}

int App::newInstance()
{
    DEBUG_BLOCK

    static bool first = true;
    if ( isSessionRestored() && first )
    {
        first = false;
        return 0;
    }

    first = false;

    handleCliArgs();
    return 0;
}


#include "App.moc"

