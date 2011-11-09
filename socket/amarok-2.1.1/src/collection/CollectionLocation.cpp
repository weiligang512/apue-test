/*
 *  Copyright (c) 2007-2008 Maximilian Kossick <maximilian.kossick@googlemail.com>
 *  Copyright (c) 2008 Jason A. Donenfeld <Jason@zx2c4.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "CollectionLocation.h"
#include "Collection.h"
#include "Debug.h"
#include "QueryMaker.h"

CollectionLocation::CollectionLocation()
    :QObject()
    , m_destination( 0 )
    , m_source( 0 )
    , m_sourceTracks()
    , m_parentCollection( 0 )
    , m_removeSources( false )
{
    //nothing to do
}

CollectionLocation::CollectionLocation( const Amarok::Collection* parentCollection)
    :QObject()
    , m_destination( 0 )
    , m_source( 0 )
    , m_sourceTracks()
    , m_parentCollection( parentCollection )
    , m_removeSources( false )
{
    //nothing to do
}

CollectionLocation::~CollectionLocation()
{
    //nothing to do
}

const Amarok::Collection*
CollectionLocation::collection() const
{
    return m_parentCollection;
}

QString
CollectionLocation::prettyLocation() const
{
    return QString();
}

QStringList
CollectionLocation::actualLocation() const
{
    return QStringList();
}

bool
CollectionLocation::isWritable() const
{
    return false;
}

bool
CollectionLocation::isOrganizable() const
{
    return false;
}

void
CollectionLocation::prepareCopy( Meta::TrackPtr track, CollectionLocation *destination )
{
    Meta::TrackList list;
    list.append( track );
    prepareCopy( list, destination );
}


void
CollectionLocation::prepareCopy( const Meta::TrackList &tracks, CollectionLocation *destination )
{
    if( !destination->isWritable() )
    {
        destination->deleteLater();
        deleteLater();
        return;
    }
    m_destination = destination;
    m_destination->setSource( this );
    startWorkflow( tracks, false );
}

void
CollectionLocation::prepareCopy( QueryMaker *qm, CollectionLocation *destination )
{
    if( !destination->isWritable() )
    {
        destination->deleteLater();
        qm->deleteLater();
        deleteLater();
        return;
    }
    m_destination = destination;
    m_removeSources = false;
    connect( qm, SIGNAL( newResultReady( QString, Meta::TrackList ) ), SLOT( resultReady( QString, Meta::TrackList ) ) );
    connect( qm, SIGNAL( queryDone() ), SLOT( queryDone() ) );
    qm->setQueryType( QueryMaker::Track );
    qm->run();
}

void
CollectionLocation::prepareMove( Meta::TrackPtr track, CollectionLocation *destination )
{
    Meta::TrackList list;
    list.append( track );
    prepareMove( list, destination );
}

void
CollectionLocation::prepareMove( const Meta::TrackList &tracks, CollectionLocation *destination )
{
    if( !destination->isWritable() )
    {
        destination->deleteLater();
        deleteLater();
        return;
    }
    m_destination = destination;
    destination->setSource( this );
    startWorkflow( tracks, true );
}

void
CollectionLocation::prepareMove( QueryMaker *qm, CollectionLocation *destination )
{
    if( !destination->isWritable() )
    {
        destination->deleteLater();
        qm->deleteLater();
        deleteLater();
        return;
    }
    m_destination = destination;
    m_removeSources = true;
    connect( qm, SIGNAL( newResultReady( QString, Meta::TrackList ) ), SLOT( resultReady( QString, Meta::TrackList ) ) );
    connect( qm, SIGNAL( queryDone() ), SLOT( queryDone() ) );
    qm->setQueryType( QueryMaker::Track );
    qm->run();
}

bool
CollectionLocation::remove( const Meta::TrackPtr &track )
{
    Q_UNUSED( track )
    return false;
}

void
CollectionLocation::abort()
{
    emit aborted();
}

void
CollectionLocation::getKIOCopyableUrls( const Meta::TrackList &tracks )
{
    QMap<Meta::TrackPtr, KUrl> urls;
    foreach( Meta::TrackPtr track, tracks )
    {
        if( track->isPlayable() )
            urls.insert( track, track->playableUrl() );
    }

    slotGetKIOCopyableUrlsDone( urls );
}

void
CollectionLocation::copyUrlsToCollection( const QMap<Meta::TrackPtr, KUrl> &sources )
{
    //reimplement in implementations which are writeable
    Q_UNUSED( sources )
    slotCopyOperationFinished();
}

void
CollectionLocation::showSourceDialog( const Meta::TrackList &tracks, bool removeSources )
{
    Q_UNUSED( tracks )
    Q_UNUSED( removeSources )
    slotShowSourceDialogDone();
}

void
CollectionLocation::showDestinationDialog( const Meta::TrackList &tracks, bool removeSources )
{
    Q_UNUSED( tracks )
    Q_UNUSED( removeSources )
    slotShowDestinationDialogDone();
}

void
CollectionLocation::slotGetKIOCopyableUrlsDone( const QMap<Meta::TrackPtr, KUrl> &sources )
{
    emit startCopy( sources );
}

void
CollectionLocation::slotCopyOperationFinished()
{
    emit finishCopy();
}

void
CollectionLocation::slotShowSourceDialogDone()
{
    emit prepareOperation( m_sourceTracks, m_removeSources );
}

void
CollectionLocation::slotShowDestinationDialogDone()
{
    emit operationPrepared();
}

void
CollectionLocation::slotPrepareOperation( const Meta::TrackList &tracks, bool removeSources )
{
    m_removeSources = removeSources;
    showDestinationDialog( tracks, removeSources );
}

void
CollectionLocation::slotOperationPrepared()
{
    getKIOCopyableUrls( m_sourceTracks );
}

void
CollectionLocation::slotStartCopy( const QMap<Meta::TrackPtr, KUrl> &sources )
{
    copyUrlsToCollection( sources );
}

void
CollectionLocation::slotFinishCopy()
{
    if( m_removeSources )
        removeSourceTracks( m_sourceTracks );
    m_sourceTracks.clear();
    m_destination->deleteLater();
    m_destination = 0;
    this->deleteLater();
}

void
CollectionLocation::slotAborted()
{
    m_destination->deleteLater();
    deleteLater();
}

void
CollectionLocation::resultReady( const QString &collectionId, const Meta::TrackList &tracks )
{
    Q_UNUSED( collectionId )
    m_sourceTracks << tracks;
}

void
CollectionLocation::queryDone()
{
    QObject *obj = sender();
    if( obj )
    {
        obj->deleteLater();
    }
    if( m_removeSources )
    {
        prepareMove( m_sourceTracks, m_destination );
    }
    else
    {
        prepareCopy( m_sourceTracks, m_destination );
    }
}

void
CollectionLocation::setupConnections()
{
    connect( this, SIGNAL( prepareOperation( Meta::TrackList, bool ) ),
             m_destination, SLOT( slotPrepareOperation( Meta::TrackList, bool ) ) );
    connect( m_destination, SIGNAL( operationPrepared() ), SLOT( slotOperationPrepared() ) );
    connect( this, SIGNAL( startCopy( QMap<Meta::TrackPtr, KUrl> ) ),
             m_destination, SLOT( slotStartCopy( QMap<Meta::TrackPtr, KUrl> ) ) );
    connect( m_destination, SIGNAL( finishCopy() ),
             this, SLOT( slotFinishCopy() ) );
    connect( this, SIGNAL( aborted() ), SLOT( slotAborted() ) );
    connect( m_destination, SIGNAL( aborted() ), SLOT( slotAborted() ) );
}

void
CollectionLocation::startWorkflow( const Meta::TrackList &tracks, bool removeSources )
{
    m_removeSources = removeSources;
    m_sourceTracks = tracks;
    setupConnections();
    if( tracks.size() <= 0 )
        abort();
    else
        showSourceDialog( tracks, m_removeSources );
}

void
CollectionLocation::removeSourceTracks( const Meta::TrackList &tracks )
{
    Meta::TrackList notDeletableTracks;
    int count = m_tracksWithError.count();
    debug() << "Transfer errors: " << count;
    foreach( Meta::TrackPtr track, tracks )
    {
        if( m_tracksWithError.contains( track ) )
        {
            debug() << "transfer error for track " << track->playableUrl();
            continue;
        }

        if( !remove( track ) )
            notDeletableTracks.append( track );
    }
    //TODO inform user about tracks which were not deleted
}

CollectionLocation*
CollectionLocation::source() const
{
    return m_source;
}

void
CollectionLocation::setSource( CollectionLocation *source )
{
    m_source = source;
}
bool
CollectionLocation::movedByDestination( const Meta::TrackPtr &track ) const
{
    return m_tracksRemovedByDestination.contains( track ) && m_tracksRemovedByDestination[ track ];
}
bool
CollectionLocation::consideredByDestination( const Meta::TrackPtr &track ) const
{
    return m_tracksRemovedByDestination.contains( track );
}
void
CollectionLocation::setMovedByDestination( const Meta::TrackPtr &track, bool removeFromDatabase )
{
    m_tracksRemovedByDestination.insert( track, removeFromDatabase );
}
bool
CollectionLocation::isGoingToRemoveSources() const
{
    return m_removeSources;
}
void
CollectionLocation::setGoingToRemoveSources( bool removeSources )
{
    m_removeSources = removeSources;
}

void
CollectionLocation::transferError( const Meta::TrackPtr &track, const QString &error )
{
    m_tracksWithError.insert( track, error );
}

#include "CollectionLocation.moc"
