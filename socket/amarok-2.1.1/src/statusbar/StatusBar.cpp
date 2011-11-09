/***************************************************************************
 *   Copyright (C) 2005 Max Howell <max.howell@methylblue.com>             *
 *   Copyright (C) 2008 Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>     *
 *   Copyright (C) 2008 Mark Kretschmann <kretschmann@kde.org>             *
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

#include "statusbar/StatusBar.h"

#include "Debug.h"
#include "EngineController.h"
#include "LongMessageWidget.h"
#include "meta/MetaUtility.h"
#include "meta/capabilities/SourceInfoCapability.h"
#include "playlist/PlaylistModel.h"

#include "KJobProgressBar.h"

#include <QTextDocument>

#include <cmath>

StatusBar* StatusBar::s_instance = 0;

namespace The
{
    StatusBar* statusBar()
    {
        return StatusBar::instance();
    }
}

StatusBar::StatusBar( QWidget * parent )
        : KStatusBar( parent )
        , EngineObserver( The::engineController() )
        , m_progressBar( new CompoundProgressBar( this ) )
        , m_busy( false )
        , m_shortMessageTimer( new QTimer( this ) )
{
    s_instance = this;

    addWidget( m_progressBar, 1 );
    m_progressBar->hide();

    connect( m_progressBar, SIGNAL( allDone() ), this, SLOT( hideProgress() ) );

    //setMaximumSize( The::mainWindow()->width(), m_progressBar->height() );
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    setContentsMargins( 0, 0, 0, 0 );
    setSizeGripEnabled( false );

    m_shortMessageTimer->setSingleShot( true );
    connect( m_shortMessageTimer, SIGNAL( timeout() ), this, SLOT( nextShortMessage() ) );

    m_nowPlayingWidget = new KHBox( 0 );
    m_nowPlayingWidget->setSpacing( 4 );

    m_nowPlayingLabel = new KSqueezedTextLabel( m_nowPlayingWidget );
    m_nowPlayingLabel->setTextElideMode( Qt::ElideRight );
    m_nowPlayingLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    m_nowPlayingEmblem = new QLabel( m_nowPlayingWidget );
    m_nowPlayingEmblem->setFixedSize( 16, 16 );

    m_separator = new QFrame( m_nowPlayingWidget );
    m_separator->setFrameShape( QFrame::VLine );

    m_playlistLengthLabel = new QLabel( m_nowPlayingWidget);
    m_playlistLengthLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    QWidget * spacer = new QWidget( m_nowPlayingWidget );
    spacer->setFixedWidth( 3 );

    addPermanentWidget( m_nowPlayingWidget );

    qRegisterMetaType<MessageType>( "MessageType" );
    connect( this, SIGNAL( signalLongMessage( const QString &, MessageType ) ), SLOT( slotLongMessage( const QString &, MessageType ) ), Qt::QueuedConnection );

    connect( The::playlistModel(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( updateTotalPlaylistLength() ) );
    connect( The::playlistModel(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ), this, SLOT( updateTotalPlaylistLength() ) );
    connect( The::playlistModel(), SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ), this, SLOT( updateTotalPlaylistLength() ) );
    connect( The::playlistModel(), SIGNAL( removedIds( const QList<quint64>& ) ), this, SLOT( updateTotalPlaylistLength() ) );

    updateTotalPlaylistLength();
}


StatusBar::~StatusBar()
{
    DEBUG_BLOCK

    delete m_progressBar;
    m_progressBar = 0;

    s_instance = 0;
}

ProgressBar * StatusBar::newProgressOperation( QObject * owner, const QString & description )
{
    //clear any short message currently being displayed and stop timer if running...
    clearMessage();
    m_shortMessageTimer->stop();

    //also hide the now playing stuff:
    m_nowPlayingWidget->hide();
    ProgressBar * newBar = new ProgressBar( 0 );
    newBar->setDescription( description );
    m_progressBar->addProgressBar( newBar, owner );
    m_progressBar->show();
    m_busy = true;

    return newBar;
}

ProgressBar * StatusBar::newProgressOperation( KJob * job, const QString & description )
{
    //clear any short message currently being displayed and stop timer if running...
    clearMessage();
    m_shortMessageTimer->stop();

    //also hide the now playing stuff:
    m_nowPlayingWidget->hide();
    KJobProgressBar * newBar = new KJobProgressBar( 0, job );
    newBar->setDescription( description );
    m_progressBar->addProgressBar( newBar, job );
    m_progressBar->show();
    m_busy = true;

    return newBar;
}

void StatusBar::shortMessage( const QString & text )
{
    if ( !m_busy )
    {
        //not busy, so show right away
        showMessage( text );
        m_shortMessageTimer->start( SHORT_MESSAGE_DURATION );
    }
    else
    {
        m_shortMessageQue.append( text );
    }
}

void StatusBar::hideProgress()
{
    DEBUG_BLOCK

    m_progressBar->hide();
    m_busy = false;

    nextShortMessage();
}

void StatusBar::nextShortMessage()
{
    if ( m_shortMessageQue.count() > 0 )
    {
        m_busy = true;
        showMessage( m_shortMessageQue.takeFirst() );
        m_shortMessageTimer->start( SHORT_MESSAGE_DURATION );
    }
    else
    {
        clearMessage();
        m_busy = false;
        m_nowPlayingWidget->show();
    }
}

void StatusBar::metadataChanged( Meta::TrackPtr track )
{
    Q_UNUSED( track );

    if ( m_currentTrack )
        updateInfo( m_currentTrack );
    else
        engineNewTrackPlaying();
}

void StatusBar::engineStateChanged( Phonon::State state, Phonon::State oldState )
{
    Q_UNUSED( oldState )
    DEBUG_BLOCK

    switch ( state )
    {
    case Phonon::StoppedState:
        m_nowPlayingLabel->setText( QString() );
        m_nowPlayingEmblem->hide();
        break;

    case Phonon::LoadingState:
        debug() << "LoadingState: clear text";
        if ( m_currentTrack )
            updateInfo( m_currentTrack );
        else
            m_nowPlayingLabel->setText( QString() );
        m_nowPlayingEmblem->hide();
        break;

    case Phonon::PausedState:
        m_nowPlayingLabel->setText( i18n( "Amarok is paused" ) ); // display TEMPORARY message
        m_nowPlayingEmblem->hide();
        break;

    case Phonon::PlayingState:
        debug() << "PlayingState: clear text";
        if ( m_currentTrack )
            updateInfo( m_currentTrack );
        //else
        //resetMainText(); // if we were paused, this is necessary
        break;

    case Phonon::ErrorState:
    case Phonon::BufferingState:
        break;
    }
}

void StatusBar::engineNewTrackPlaying()
{
    if ( m_currentTrack )
        unsubscribeFrom( m_currentTrack );

    m_currentTrack = The::engineController()->currentTrack();

    if ( !m_currentTrack )
    {
        m_currentTrack = Meta::TrackPtr();
        return;
    }
    subscribeTo( m_currentTrack );
    updateInfo( m_currentTrack );
}

void StatusBar::updateInfo( Meta::TrackPtr track )
{
    QString title       = Qt::escape( track->name() );
    QString prettyTitle = Qt::escape( track->prettyName() );
    QString artist      = track->artist() ? Qt::escape( track->artist()->name() ) : QString();
    QString album       = track->album() ? Qt::escape( track->album()->name() ) : QString();
    QString length      = Qt::escape( Meta::secToPrettyTime( track->length() ) );

    // ugly because of translation requirements
    if ( !title.isEmpty() && !artist.isEmpty() && !album.isEmpty() )
        title = i18nc( "track by artist on album", "<b>%1</b> by <b>%2</b> on <b>%3</b>", title, artist, album );

    else if ( !title.isEmpty() && !artist.isEmpty() )
        title = i18nc( "track by artist", "<b>%1</b> by <b>%2</b>", title, artist );

    else if ( !album.isEmpty() )
        // we try for pretty title as it may come out better
        title = i18nc( "track on album", "<b>%1</b> on <b>%2</b>", prettyTitle, album );
    else
        title = "<b>" + prettyTitle + "</b>";

    if ( title.isEmpty() )
        title = i18n( "Unknown track" );


    // check if we have any source info:

    Meta::SourceInfoCapability *sic = track->create<Meta::SourceInfoCapability>();
    if ( sic )
    {
        //is the source defined
        QString source = sic->sourceName();
        if ( !source.isEmpty() )
        {
            title += ' ' + i18n( "from" ) + " <b>" + source + "</b>";
            m_nowPlayingEmblem->setPixmap( sic->emblem() );
            m_nowPlayingEmblem->show();
        }
        else
            m_nowPlayingEmblem->hide();
        delete sic;
    }
    else
        m_nowPlayingEmblem->hide();

    // don't show '-' or '?'
    if ( length.length() > 1 )
    {
        title += " (";
        title += length;
        title += ')';
    }

    m_nowPlayingLabel->setText( i18n( "Playing: %1", title ) );
}

void StatusBar::longMessage( const QString & text, MessageType type )
{
    DEBUG_BLOCK

    // The purpose of this emit is to make the operation thread safe. If this
    // method is called from a non-GUI thread, the "emit" relays it over the
    // event loop to the GUI thread, so that we can safely create widgets.

    emit signalLongMessage( text, type );
}

void StatusBar::slotLongMessage( const QString & text, MessageType type ) //SLOT
{
    DEBUG_BLOCK

    LongMessageWidget * message = new LongMessageWidget( this, text, type );
    connect( message, SIGNAL( closed() ), this, SLOT( hideLongMessage() ) );
}

void StatusBar::hideLongMessage()
{
    sender()->deleteLater();
}

void
StatusBar::updateTotalPlaylistLength() //SLOT
{
    const int totalLength = The::playlistModel()->totalLength();
    const int trackCount = The::playlistModel()->rowCount();
    const QString totalTime = Meta::secToPrettyTime( totalLength );

    if( totalLength > 0 && trackCount > 0 )
    {
        m_playlistLengthLabel->setText( i18ncp( "%1 is number of tracks, %2 is time", "%1 track (%2)", "%1 tracks (%2)", trackCount, totalTime ) );
        m_playlistLengthLabel->show();
        m_separator->show();
    }
    else if( ( totalLength == 0 ) && ( trackCount > 0 ) )
    {
        m_playlistLengthLabel->setText( i18ncp( "%1 is number of tracks", "%1 track", "%1 tracks", trackCount ) );
        m_playlistLengthLabel->show();
        m_separator->show();
    }
    //Total Length will not be > 0 if trackCount is 0, so we can ignore it
    else { // TotalLength = 0 and trackCount = 0;
        m_playlistLengthLabel->hide();
        m_separator->hide();
    }
}

#include "StatusBar.moc"

