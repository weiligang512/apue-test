/***************************************************************************
  begin                : Fre Nov 15 2002
  copyright            : (C) Mark Kretschmann <markey@web.de>
                       : (C) Max Howell <max.howell@methylblue.com>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "amarok_export.h"
#include "meta/Meta.h"
#include "EngineObserver.h"
#include "Sidebar.h"

#include <KMainWindow>
#include <KVBox>

#include <QPointer>

class CollectionWidget;
class ContextWidget;
class MainToolbar;
class MainWindow;
class PlaylistFileProvider;
class SearchWidget;

namespace PlaylistBrowserNS { class PlaylistBrowser; }

namespace Plasma { class Containment; }
namespace Playlist { class Widget; }

namespace Context {
    class ContextScene;
    class ContextView;
    class ToolbarView;
}

class KMenu;
class QMenuBar;
class QSplitter;
class QTimer;

namespace The {
    AMAROK_EXPORT MainWindow* mainWindow();
}

/**
  * @class MainWindow
  * @short The MainWindow widget class.
  *
  * This is the main window widget.
  */
class AMAROK_EXPORT MainWindow : public KMainWindow, public EngineObserver, public Meta::Observer
{
    friend MainWindow* The::mainWindow();

    Q_OBJECT

    public:
        MainWindow();
       ~MainWindow();

        //allows us to switch browsers from within other browsers etc
        void showBrowser( const QString& name );
        void addBrowser( const QString &name, QWidget *widget, const QString &text, const QString &icon );

        //takes into account minimized, multiple desktops, etc.
        bool isReallyShown() const;

        void activate();

        SideBar *sideBar() const { return m_browsers; }
        QPointer<KMenu> ToolsMenu() const { return m_toolsMenu; }
        QPointer<KMenu> SettingsMenu() const { return m_settingsMenu; }
		QPointer<Playlist::Widget> playlistWidget() { return m_playlistWidget; }
        void deleteBrowsers();

        QString activeBrowserName();

        CollectionWidget * collectionBrowser();
        PlaylistBrowserNS::PlaylistBrowser * playlistBrowser();

        //will return the size of the rect defined top, right and left by the main toolbar and bottom by the context view.
        QSize backgroundSize();

        int contextXOffset();
        QRect contextRectGlobal();
        QPoint globalBackgroundOffset();

    signals:
        void loveTrack( Meta::TrackPtr );

    public slots:
        void showHide();
        void loveTrack();
        void playAudioCD();
        void hideContextView( bool hide );

    protected:
        //Reimplemented from EngineObserver
        virtual void engineStateChanged( Phonon::State state, Phonon::State oldState = Phonon::StoppedState );

        //Reimplemented from Meta::Observer
        using Observer::metadataChanged;
        virtual void metadataChanged( Meta::TrackPtr track );

    private slots:
        void slotShrinkBrowsers( int index );
        void savePlaylist() const;
        void exportPlaylist() const;
        void slotShowCoverManager() const;
        void slotPlayMedia();
        void slotAddLocation( bool directPlay = false );
        void slotAddStream();
        void showScriptSelector();

    protected:
        virtual void closeEvent( QCloseEvent* );
        virtual void keyPressEvent( QKeyEvent* );
        virtual QSize sizeHint() const;
        virtual void resizeEvent ( QResizeEvent * event );

        virtual void paletteChange( const QPalette & oldPalette );

    private slots:
        void setRating1() { setRating( 1 ); }
        void setRating2() { setRating( 2 ); }
        void setRating3() { setRating( 3 ); }
        void setRating4() { setRating( 4 ); }
        void setRating5() { setRating( 5 ); }

    private:
        void init();
        void setRating( int n );
        void showBrowser( const int index );

        QPointer<QMenuBar>  m_menubar;
        QPointer<KMenu>     m_toolsMenu;
        QPointer<KMenu>     m_settingsMenu;
        QPointer<SideBar>   m_browsers;
        QStringList         m_browserNames;
        QPointer<KMenu>     m_searchMenu;
        QPointer<KVBox>     m_statusbarArea;

        QPointer<SearchWidget>     m_searchWidget;
        QPointer<MainToolbar>      m_controlBar;
        QPointer<Playlist::Widget> m_playlistWidget;
        QPointer<QTimer>           m_timer;  //search filter timer
        QPointer<QSplitter>        m_splitter;

        QByteArray                 m_splitterState;

        QPointer<ContextWidget>         m_contextWidget;
        QPointer<Context::ContextScene> m_corona;
        QPointer<Context::ContextView>  m_contextView;
        QPointer<Context::ToolbarView>  m_contextToolbarView;

        PlaylistFileProvider *m_playlistFiles;
        Meta::TrackPtr m_currentTrack;

        void    createActions();
        void    createMenus();
        int     m_lastBrowser;
        int     m_searchField;

        static QPointer<MainWindow> s_instance;

    private slots:
        void createContextView( Plasma::Containment *c );
};


#endif //AMAROK_PLAYLISTWINDOW_H

