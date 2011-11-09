/* This file is part of the KDE project
   Copyright (C) 2007 Bart Cerneels <bart.cerneels@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "SqlPodcastProvider.h"

#include "Amarok.h"
#include "CollectionManager.h"
#include "context/popupdropper/libpud/PopupDropperAction.h"
#include "context/popupdropper/libpud/PopupDropperItem.h"
#include "context/popupdropper/libpud/PopupDropper.h"
#include "statusbar/StatusBar.h"
#include "Debug.h"
#include "PodcastModel.h"
#include "PodcastReader.h"
#include "PodcastSettingsDialog.h"
#include "SqlStorage.h"
#include "playlistmanager/sql/SqlPlaylistGroup.h"
#include "SvgHandler.h"

#include <KLocale>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KIO/Job>
#include <KUrl>

#include <QFile>
#include <QDir>
#include <QTimer>

using namespace Meta;

static const int PODCAST_DB_VERSION = 3;
static const QString key("AMAROK_PODCAST");

SqlPodcastProvider::SqlPodcastProvider()
    : m_updateTimer( new QTimer(this) )
    , m_updatingChannels( 0 )
    , m_configureAction( 0 )
    , m_deleteAction( 0 )
    , m_downloadAction( 0 )
    , m_removeAction( 0 )
    , m_renameAction( 0 )
    , m_updateAction( 0 )
{
    connect( m_updateTimer, SIGNAL( timeout() ), SLOT( autoUpdate() ) );

    SqlStorage *sqlStorage = CollectionManager::instance()->sqlStorage();

    QStringList values;
    if ( sqlStorage )
        values = sqlStorage->query( QString("SELECT version FROM admin WHERE component = '%1';").arg(sqlStorage->escape( key ) ) );
    if( sqlStorage && values.isEmpty() )
    {
        debug() << "creating Podcast Tables";
        createTables();
        sqlStorage->query( "INSERT INTO admin(component,version) "
                       "VALUES('" + key + "'," + QString::number( PODCAST_DB_VERSION ) + ");" );
    }
    else
    {
        int version = values.first().toInt();
        if( version == PODCAST_DB_VERSION )
            loadPodcasts();
        else
            updateDatabase( version /*from*/, PODCAST_DB_VERSION /*to*/ );

        bool startAutoRefreshTimer = false;
        foreach( Meta::SqlPodcastChannelPtr channel, m_channels )
        {
            startAutoRefreshTimer = channel->autoScan();
        }
        if( startAutoRefreshTimer )
        {
            float interval = 1.0;
            m_updateTimer->start( interval * 1000 * 60 * 30 );
        }

    }
}

SqlPodcastProvider::~SqlPodcastProvider()
{
    foreach(Meta::SqlPodcastChannelPtr channel, m_channels)
    {
        channel->updateInDb();
        foreach(Meta::SqlPodcastEpisodePtr episode, channel->sqlEpisodes())
        {
            episode->updateInDb();
        }
    }
    m_channels.clear();
}

void
SqlPodcastProvider::loadPodcasts()
{
    m_channels.clear();
    SqlStorage *sqlStorage = CollectionManager::instance()->sqlStorage();

    QStringList results = sqlStorage->query( "SELECT id, url, title, weblink, image, description, copyright, directory, labels, subscribedate, autoscan, fetchtype, haspurge, purgecount FROM podcastchannels;" );

    int rowLength = 14;
    for(int i=0; i < results.size(); i+=rowLength)
    {
        QStringList channelResult = results.mid( i, rowLength );
        m_channels << SqlPodcastChannelPtr( new SqlPodcastChannel( channelResult ) );
    }
    emit( updated() );
}

bool
SqlPodcastProvider::possiblyContainsTrack( const KUrl & url ) const
{
    DEBUG_BLOCK
    SqlStorage *sqlStorage = CollectionManager::instance()->sqlStorage();

    QString command = "SELECT title FROM podcastepisodes WHERE url='%1' OR localurl='%1';";
    command = command.arg( sqlStorage->escape( url.url() ) );

    QStringList dbResult = sqlStorage->query( command );
    return !dbResult.isEmpty();
}

Meta::TrackPtr
SqlPodcastProvider::trackForUrl( const KUrl & url )
{
    DEBUG_BLOCK
            
    SqlStorage *sqlStorage = CollectionManager::instance()->sqlStorage();

    QString command = "SELECT id,channel FROM podcastepisodes WHERE url='%1' OR localurl='%1';";
    command = command.arg( sqlStorage->escape( url.url() ) );
    QStringList dbResult = sqlStorage->query( command );

    int episodeId = dbResult[0].toInt();
    int channelId = dbResult[1].toInt();
    Meta::SqlPodcastChannelPtr channel;
    foreach( channel, m_channels )
        if( channel->dbId() == channelId )
            break;

    if( channel.isNull() )
        return TrackPtr();

    Meta::SqlPodcastEpisodePtr episode;
    foreach( episode, channel->sqlEpisodes() )
    {
        if( episode->dbId() == episodeId )
        {
            debug() << "found it!";
            break;
        }
    }

    return Meta::TrackPtr::dynamicCast( episode );
}

Meta::PlaylistList
SqlPodcastProvider::playlists()
{
    Meta::PlaylistList playlistList;

    QListIterator<Meta::SqlPodcastChannelPtr> i( m_channels );
    while( i.hasNext() )
    {
        playlistList << PlaylistPtr::staticCast( i.next() );
    }
    return playlistList;
}

void
SqlPodcastProvider::addPodcast(const KUrl & url)
{
    KUrl kurl = KUrl( url );

    SqlStorage *sqlStorage = CollectionManager::instance()->sqlStorage();

    QString command = "SELECT title FROM podcastchannels WHERE url='%1';";
    command = command.arg( sqlStorage->escape( kurl.url() ) );

    QStringList dbResult = sqlStorage->query( command );
    if( !dbResult.isEmpty() )
    {
        //Already subscribed to this Channel
        //notify the user.
        The::statusBar()->longMessage(
            i18n("Already subscribed to %1.", dbResult.first())
            , StatusBar::Error );
    }
    else
    {
        bool result = false;
        m_updatingChannels++;
        PodcastReader * podcastReader = new PodcastReader( this );

        connect( podcastReader, SIGNAL( finished( PodcastReader *, bool ) ),
                SLOT( slotReadResult( PodcastReader *, bool ) ) );

        result = podcastReader->read( kurl );
    }
}

Meta::PodcastChannelPtr
SqlPodcastProvider::addChannel( Meta::PodcastChannelPtr channel )
{
    Meta::SqlPodcastChannel * sqlChannel = new Meta::SqlPodcastChannel( channel );
    m_channels << SqlPodcastChannelPtr( sqlChannel );
    return Meta::PodcastChannelPtr( sqlChannel );
}

Meta::PodcastEpisodePtr
SqlPodcastProvider::addEpisode( Meta::PodcastEpisodePtr episode )
{
    DEBUG_BLOCK
    Meta::SqlPodcastEpisodePtr sqlEpisode
            = Meta::SqlPodcastEpisodePtr::dynamicCast( episode );
    if( sqlEpisode.isNull() )
        return Meta::PodcastEpisodePtr();
    if( sqlEpisode->channel().isNull() )
    {
        debug() << "channel is null";
        return Meta::PodcastEpisodePtr();
    }

    if( sqlEpisode->channel()->fetchType() == Meta::PodcastChannel::DownloadWhenAvailable )
        downloadEpisode( sqlEpisode );
    return Meta::PodcastEpisodePtr::dynamicCast( sqlEpisode );
}

Meta::PodcastChannelList
SqlPodcastProvider::channels()
{
    PodcastChannelList list;
    QListIterator<SqlPodcastChannelPtr> i(m_channels);
    while( i.hasNext() )
    {
        list << PodcastChannelPtr::dynamicCast( i.next() );
    }
    return list;
}

void
SqlPodcastProvider::removeSubscription( Meta::PodcastChannelPtr channel )
{
    DEBUG_BLOCK
    Meta::SqlPodcastChannelPtr sqlChannel = Meta::SqlPodcastChannelPtr::dynamicCast( channel );
    if( !sqlChannel )
        return;

    debug() << "Deleting channel " << sqlChannel->title();
    sqlChannel->deleteFromDb();

    m_channels.removeOne( sqlChannel );
    emit updated();
}

void
SqlPodcastProvider::configureProvider()
{
    DEBUG_BLOCK
}

void
SqlPodcastProvider::configureChannel( Meta::PodcastChannelPtr channel )
{
    Meta::SqlPodcastChannelPtr sqlChannel = Meta::SqlPodcastChannelPtr::dynamicCast( channel );
    if( !sqlChannel )
        return;

    KUrl oldSaveLocation = sqlChannel->saveLocation();
    bool oldHasPurge = sqlChannel->hasPurge();
    int oldPurgeCount = sqlChannel->purgeCount();

    PodcastSettingsDialog dialog( channel, The::mainWindow() );
    dialog.configure();

    sqlChannel->updateInDb();

    if( sqlChannel->hasPurge() )
    {
        int toPurge = sqlChannel->purgeCount();
        if( !oldHasPurge || ( oldPurgeCount != toPurge && toPurge > 0 ) )
        {
            debug() << "purge to " << toPurge <<" newest episodes for " << sqlChannel->title();
            foreach( Meta::SqlPodcastEpisodePtr episode, sqlChannel->sqlEpisodes() )
            {
                if( --toPurge < 0 )
                    if( !episode->localUrl().isEmpty() )
                        deleteDownloadedEpisode( episode );
            }
            sqlChannel->loadEpisodes();
            emit( updated() );
        }
    }
    else if( oldHasPurge )
    {
        /* changed from purge to no-purge:
        we need to reload all episodes from the database. */
        sqlChannel->loadEpisodes();
        emit( updated() );
    }

    if( oldSaveLocation != channel->saveLocation() )
    {
        debug() << QString("We need to move downloaded episodes of \"%1\" to %2")
            .arg( sqlChannel->title())
            .arg( sqlChannel->saveLocation().prettyUrl() );

        KUrl::List filesToMove;
        foreach( Meta::SqlPodcastEpisodePtr episode, sqlChannel->sqlEpisodes() )
        {
            if( !episode->localUrl().isEmpty() )
            {
                KUrl newLocation = sqlChannel->saveLocation();
                newLocation.addPath( episode->localUrl().fileName() );
                debug() << "Moving from " << episode->localUrl() << " to " << newLocation;

                filesToMove << episode->localUrl();
                episode->setLocalUrl( newLocation );
                episode->updateInDb();
            }
        }
        if( !filesToMove.isEmpty() )
            KIO::move( filesToMove, sqlChannel->saveLocation(), KIO::HideProgressInfo );
    }
}

QList<PopupDropperAction *>
SqlPodcastProvider::episodeActions( Meta::PodcastEpisodeList episodes )
{
    DEBUG_BLOCK
    QList< PopupDropperAction * > actions;

    if( m_deleteAction == 0 )
    {
        m_deleteAction = new PopupDropperAction(
            The::svgHandler()->getRenderer( "amarok/images/pud_items.svg" ),
            "delete",
            KIcon( "edit-delete" ),
            i18n( "&Delete Downloaded Episode" ),
            this
        );
        connect( m_deleteAction, SIGNAL( triggered() ), this, SLOT( slotDeleteEpisodes() ) );
    }
    bool hasDownloaded = false;
    foreach( Meta::PodcastEpisodePtr episode, episodes )
    {
        Meta::SqlPodcastEpisodePtr sqlEpisode
                = Meta::SqlPodcastEpisodePtr::dynamicCast( episode );
        if( sqlEpisode.isNull() )
            break;

        if( !sqlEpisode->localUrl().isEmpty() )
        {
            hasDownloaded = true;
            break;
        }
    }
    if( hasDownloaded )
    {
        actions << m_deleteAction;
    }
    else
    {
        if ( m_downloadAction == 0 )
        {
            m_downloadAction = new PopupDropperAction(
                The::svgHandler()->getRenderer( "amarok/images/pud_items.svg" ),
                "download",
                KIcon( "go-down" ),
                i18n( "&Download Episode" ),
                this
            );
            connect( m_downloadAction, SIGNAL( triggered() ), this, SLOT( slotDownloadEpisodes() ) );
        }
        actions << m_downloadAction;
    }

    return actions;
}

QList<PopupDropperAction *>
SqlPodcastProvider::channelActions( Meta::PodcastChannelList )
{
    DEBUG_BLOCK
    QList< PopupDropperAction * > actions;

    if( m_configureAction == 0 )
    {
        m_configureAction = new PopupDropperAction(
            The::svgHandler()->getRenderer("amarok/images/pud_items.svg"),
            "configure",
            KIcon( "configure" ),
            i18n( "&Configure" ),
            this
        );
        connect( m_configureAction, SIGNAL( triggered() ), this, SLOT( slotConfigureChannel() ));
    }
    actions << m_configureAction;

    if( m_removeAction == 0 )
    {
        m_removeAction = new PopupDropperAction(
            The::svgHandler()->getRenderer( "amarok/images/pud_items.svg" ),
            "remove",
            KIcon( "news-unsubscribe" ),
            i18n( "&Remove Subscription" ),
            this
        );
        connect( m_removeAction, SIGNAL( triggered() ), this, SLOT( slotRemoveChannels() ) );
    }
    actions << m_removeAction;

    if( m_updateAction == 0 )
    {
        m_updateAction = new PopupDropperAction(
            The::svgHandler()->getRenderer( "amarok/images/pud_items.svg" ),
            "update",
            KIcon( "view-refresh-amarok" ),
            i18n( "&Update Channel" ),
            this
        );
        connect( m_updateAction, SIGNAL( triggered() ), this, SLOT( slotUpdateChannels() ) );
    }
    actions << m_updateAction;

    return actions;
}

void
SqlPodcastProvider::slotDeleteEpisodes()
{
    DEBUG_BLOCK
    Meta::PodcastEpisodeList episodes = The::podcastModel()->selectedEpisodes();
    foreach( Meta::PodcastEpisodePtr episode, episodes )
    {
        Meta::SqlPodcastEpisodePtr sqlEpisode =
                Meta::SqlPodcastEpisodePtr::dynamicCast( episode );
        if( !sqlEpisode )
            continue;

        deleteDownloadedEpisode( sqlEpisode );
    }
}

void
SqlPodcastProvider::slotDownloadEpisodes()
{
    DEBUG_BLOCK
    Meta::PodcastEpisodeList episodes = The::podcastModel()->selectedEpisodes();
    debug() << episodes.count() << " episodes selected";
    foreach( Meta::PodcastEpisodePtr episode, episodes )
    {
        Meta::SqlPodcastEpisodePtr sqlEpisode =
                Meta::SqlPodcastEpisodePtr::dynamicCast( episode );
//         if( !sqlEpisode )
//             continue;

        downloadEpisode( sqlEpisode );
    }
}

void
SqlPodcastProvider::slotRemoveChannels()
{
    DEBUG_BLOCK
    foreach( Meta::PodcastChannelPtr channel, The::podcastModel()->selectedChannels() )
    {
        Meta::SqlPodcastChannelPtr sqlChannel =
            Meta::SqlPodcastChannelPtr::dynamicCast( channel );

        //TODO:request confirmation and ask if the files have to be deleted as well
        removeSubscription( channel );
    }
}

void
SqlPodcastProvider::slotUpdateChannels()
{
    DEBUG_BLOCK
    foreach( Meta::PodcastChannelPtr channel, The::podcastModel()->selectedChannels() )
    {
        Meta::SqlPodcastChannelPtr sqlChannel =
            Meta::SqlPodcastChannelPtr::dynamicCast( channel );
        if( !sqlChannel.isNull() )
            update( channel );
    }
}

void
SqlPodcastProvider::slotConfigureChannel()
{
    DEBUG_BLOCK
    //only one channel should be selected or dragged because
    //of the actions we've returned in channelActions()
    if( The::podcastModel()->selectedChannels().count() )
        configureChannel( The::podcastModel()->selectedChannels().first() );
}

void
SqlPodcastProvider::deleteDownloadedEpisode( Meta::SqlPodcastEpisodePtr episode )
{
    DEBUG_BLOCK
    if( episode->localUrl().isEmpty() )
        return;

    debug() << "deleting " << episode->title();
    KIO::del( episode->localUrl(), KIO::HideProgressInfo );

    episode->setLocalUrl( KUrl() );
    episode->updateInDb();
    emit( updated() );
}

Meta::SqlPodcastChannelPtr
SqlPodcastProvider::podcastChannelForId( int podcastChannelId )
{
    QListIterator<Meta::SqlPodcastChannelPtr> i( m_channels );
    while( i.hasNext() )
    {
        int id = i.next()->dbId();
        if( id == podcastChannelId )
            return i.previous();
    }
    return Meta::SqlPodcastChannelPtr();
}

void
SqlPodcastProvider::updateAll()
{
    foreach( Meta::SqlPodcastChannelPtr channel, m_channels )
    {
        update( channel );
        
    }
}

void
SqlPodcastProvider::autoUpdate()
{
    DEBUG_BLOCK
    foreach( Meta::SqlPodcastChannelPtr channel, m_channels )
    {
        if( channel->autoScan() )
            update( channel );
    }
}

void
SqlPodcastProvider::update( Meta::PodcastChannelPtr channel )
{
    m_updatingChannels++;
    PodcastReader * podcastReader = new PodcastReader( this );

    connect( podcastReader, SIGNAL( finished( PodcastReader *, bool ) ),
             SLOT( slotReadResult( PodcastReader *, bool ) ) );
    //PodcastReader will create a progress bar in The StatusBar.

    podcastReader->update( channel );
}

void
SqlPodcastProvider::downloadEpisode( Meta::SqlPodcastEpisodePtr sqlEpisode )
{
    if( sqlEpisode.isNull() )
    {
        debug() << "Error: SqlPodcastProvider::downloadEpisode(  Meta::SqlPodcastEpisodePtr sqlEpisode ) was called for a non-SqlPodcastEpisode";
        return;
    }

    KIO::StoredTransferJob *storedTransferJob = KIO::storedGet( sqlEpisode->uidUrl(), KIO::Reload, KIO::HideProgressInfo );

    m_jobMap[storedTransferJob] = sqlEpisode.data();
    m_fileNameMap[storedTransferJob] = KUrl( sqlEpisode->uidUrl() ).fileName();

    debug() << "starting download for " << sqlEpisode->title() << " url: " << sqlEpisode->prettyUrl();
    The::statusBar()->newProgressOperation( storedTransferJob, sqlEpisode->title().isEmpty()
            ? i18n("Downloading Podcast Media") : i18n("Downloading Podcast \"%1\"", sqlEpisode->title()) )
            ->setAbortSlot( this, SLOT( abortDownload()) );

    connect( storedTransferJob, SIGNAL(  finished( KJob * ) ), SLOT( downloadResult( KJob * ) ) );
    connect( storedTransferJob, SIGNAL( redirection( KIO::Job *, const KUrl& ) ), SLOT( redirected( KIO::Job *,const KUrl& ) ) );
}

void
SqlPodcastProvider::downloadEpisode( Meta::PodcastEpisodePtr episode )
{
    downloadEpisode( SqlPodcastEpisodePtr::dynamicCast( episode ) );
}

void
SqlPodcastProvider::deleteDownloadedEpisode( Meta::PodcastEpisodePtr episode )
{
    DEBUG_BLOCK
    deleteDownloadedEpisode( SqlPodcastEpisodePtr::dynamicCast( episode ) );
}

void
SqlPodcastProvider::slotReadResult( PodcastReader *podcastReader, bool result )
{
    DEBUG_BLOCK
    if ( !result )
    {
        debug() << "Parse error in podcast "
            << podcastReader->url() << " line: "
            << podcastReader->lineNumber() << " column "
            << podcastReader->columnNumber() << " : "
            << podcastReader->errorString();
    }
    else
    {
        debug() << "Finished updating: " << podcastReader->url();
        debug() << "Updating counter reached " << m_updatingChannels-1;
        //decrement the counter and load podcasts if needed.
        if( --m_updatingChannels == 0 )
        {
            //TODO: start downloading episodes here.
        }
    }

    podcastReader->deleteLater();
    emit( updated() );
}

void
SqlPodcastProvider::update( Meta::SqlPodcastChannelPtr channel )
{
    update( PodcastChannelPtr::dynamicCast( channel ) );
}

void
SqlPodcastProvider::slotUpdated()
{
    emit updated();
}

void
SqlPodcastProvider::downloadResult( KJob * job )
{
    if( job->error() )
    {
        The::statusBar()->longMessage( job->errorText() );
        debug() << "Unable to retrieve podcast media. KIO Error: " << job->errorText();
    }
    else if( ! m_jobMap.contains( job ) )
    {
        warning() << "Download is finished for a job that was not added to m_jobMap. Waah?";
    }
    else
    {
        Meta::SqlPodcastEpisode *sqlEpisode = m_jobMap.value( job );
        if( sqlEpisode == 0 )
        {
            debug() << "sqlEpisodePtr is NULL after download";
            return;
        }

        QDir dir( sqlEpisode->channel()->saveLocation().path() );
        dir.mkpath( "." );
        KUrl localUrl = KUrl::fromPath( dir.absolutePath() );
        localUrl.addPath( m_fileNameMap[job] );

        QFile *localFile = new QFile( localUrl.path() );
        if( localFile->open( QIODevice::WriteOnly ) &&
            localFile->write( (static_cast<KIO::StoredTransferJob *>(job))->data() ) != -1 )
        {
            debug() << "successfully written Podcast Episode " << sqlEpisode->title() << " to " << localUrl.path();
            sqlEpisode->setLocalUrl( localUrl );
            //force an update so the icon can be updated in the PlaylistBrowser
            emit( updated() );
        }
        else
        {
            The::statusBar()->longMessage( i18n("Unable to save podcast episode file to %1",
                            localUrl.prettyUrl()) );
        }
        localFile->close();
    }
    //remove it from the jobmap
    m_jobMap.remove( job );
    m_fileNameMap.remove( job );
}

void
SqlPodcastProvider::redirected( KIO::Job *job, const KUrl & redirectedUrl )
{
    debug() << "redirecting to " << redirectedUrl << ". filename: " << redirectedUrl.fileName();
    m_fileNameMap[job] = redirectedUrl.fileName();
}

void
SqlPodcastProvider::createTables() const
{
    DEBUG_BLOCK

    SqlStorage *sqlStorage = CollectionManager::instance()->sqlStorage();
    sqlStorage->query( QString( "CREATE TABLE podcastchannels ("
                    "id " + sqlStorage->idType() +
                    ",url " + sqlStorage->longTextColumnType() +
                    ",title " + sqlStorage->longTextColumnType() +
                    ",weblink " + sqlStorage->longTextColumnType() +
                    ",image " + sqlStorage->longTextColumnType() +
                    ",description " + sqlStorage->longTextColumnType() +
                    ",copyright "  + sqlStorage->textColumnType() +
                    ",directory "  + sqlStorage->textColumnType() +
                    ",labels " + sqlStorage->textColumnType() +
                    ",subscribedate " + sqlStorage->textColumnType() +
                    ",autoscan BOOL, fetchtype INTEGER"
                    ",haspurge BOOL, purgecount INTEGER );" ) );

    sqlStorage->query( QString( "CREATE TABLE podcastepisodes ("
                    "id " + sqlStorage->idType() +
                    ",url " + sqlStorage->longTextColumnType() +
                    ",channel INTEGER"
                    ",localurl " + sqlStorage->longTextColumnType() +
                    ",guid " + sqlStorage->exactTextColumnType() +
                    ",title " + sqlStorage->longTextColumnType() +
                    ",subtitle " + sqlStorage->longTextColumnType() +
                    ",sequencenumber INTEGER" +
                    ",description " + sqlStorage->longTextColumnType() +
                    ",mimetype "  + sqlStorage->textColumnType() +
                    ",pubdate "  + sqlStorage->textColumnType() +
                    ",duration INTEGER"
                    ",filesize INTEGER"
                    ",isnew BOOL );" ));

    sqlStorage->query( "CREATE FULLTEXT INDEX url_podchannel ON podcastchannels( url );" );
    sqlStorage->query( "CREATE FULLTEXT INDEX url_podepisode ON podcastepisodes( url );" );
    sqlStorage->query( "CREATE FULLTEXT INDEX localurl_podepisode ON podcastepisodes( localurl );" );
}

void
SqlPodcastProvider::updateDatabase( int fromVersion, int toVersion )
{
    debug() << QString( "Updating Podcast tables from version %1 to version %2" ).arg(fromVersion).arg(toVersion);

    SqlStorage *sqlStorage = CollectionManager::instance()->sqlStorage();
    #define escape(x) sqlStorage->escape(x)

    if( fromVersion == 1 && toVersion == 2 )
    {
        QString updateChannelQuery = QString( "ALTER TABLE podcastchannels"
            " ADD subscribedate " + sqlStorage->textColumnType() + ';' );

        sqlStorage->query( updateChannelQuery );

        QString setDateQuery = QString( "UPDATE podcastchannels SET subscribedate='%1' WHERE subscribedate='';" ).arg( escape(QDate::currentDate().toString()) );
        sqlStorage->query( setDateQuery );
    }
    else if ( fromVersion < 3 && toVersion == 3 )
    {
        sqlStorage->query( QString( "CREATE TABLE podcastchannels_temp ("
                    "id " + sqlStorage->idType() +
                    ",url " + sqlStorage->exactTextColumnType() + " UNIQUE"
                    ",title " + sqlStorage->textColumnType() +
                    ",weblink " + sqlStorage->exactTextColumnType() +
                    ",image " + sqlStorage->exactTextColumnType() +
                    ",description " + sqlStorage->longTextColumnType() +
                    ",copyright "  + sqlStorage->textColumnType() +
                    ",directory "  + sqlStorage->textColumnType() +
                    ",labels " + sqlStorage->textColumnType() +
                    ",subscribedate " + sqlStorage->textColumnType() +
                    ",autoscan BOOL, fetchtype INTEGER"
                    ",haspurge BOOL, purgecount INTEGER );" ) );

        sqlStorage->query( QString( "CREATE TABLE podcastepisodes_temp ("
                    "id " + sqlStorage->idType() +
                    ",url " + sqlStorage->exactTextColumnType() + " UNIQUE"
                    ",channel INTEGER"
                    ",localurl " + sqlStorage->exactTextColumnType() +
                    ",guid " + sqlStorage->exactTextColumnType() +
                    ",title " + sqlStorage->textColumnType() +
                    ",subtitle " + sqlStorage->textColumnType() +
                    ",sequencenumber INTEGER" +
                    ",description " + sqlStorage->longTextColumnType() +
                    ",mimetype "  + sqlStorage->textColumnType() +
                    ",pubdate "  + sqlStorage->textColumnType() +
                    ",duration INTEGER"
                    ",filesize INTEGER"
                    ",isnew BOOL );" ));

        sqlStorage->query( "INSERT INTO podcastchannels_temp SELECT * FROM podcastchannels;" );
        sqlStorage->query( "INSERT INTO podcastepisodes_temp SELECT * FROM podcastepisodes;" );

        sqlStorage->query( "DROP TABLE podcastchannels;" );
        sqlStorage->query( "DROP TABLE podcastepisodes;" );

        createTables();

        sqlStorage->query( "INSERT INTO podcastchannels SELECT * FROM podcastchannels_temp;" );
        sqlStorage->query( "INSERT INTO podcastepisodes SELECT * FROM podcastepisodes_temp;" );

        sqlStorage->query( "DROP TABLE podcastchannels_temp;" );
        sqlStorage->query( "DROP TABLE podcastepisodes_temp;" );
    }


    QString updateAdmin = QString( "UPDATE admin SET version=%1 WHERE component='%2';" );
    sqlStorage->query( updateAdmin.arg( toVersion ).arg( escape(key) ) );

    loadPodcasts();
}

#include "SqlPodcastProvider.moc"
