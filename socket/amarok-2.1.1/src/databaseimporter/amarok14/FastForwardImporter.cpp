/*******************************************************************************
* copyright              : (C) 2008 Seb Ruiz <ruiz@kde.org>                    *
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#include "FastForwardImporter.h"
#include "FastForwardImporterConfig.h"
#include "FastForwardWorker.h"
#include "CollectionManager.h"
#include "Debug.h"


FastForwardImporter::FastForwardImporter( QObject *parent )
    : DatabaseImporter( parent )
    , m_config( 0 )
    , m_worker( 0 )
{
}

FastForwardImporter::~FastForwardImporter()
{
    DEBUG_BLOCK
    if( m_worker )
    {
        m_worker->abort();
        //m_worker->deleteLater();
    }
}

DatabaseImporterConfig*
FastForwardImporter::configWidget( QWidget *parent )
{
    if( !m_config )
        m_config = new FastForwardImporterConfig( parent );
    return m_config;
}

void
FastForwardImporter::import()
{
    DEBUG_BLOCK
    // We've already started
    if( m_worker )
        return;

    Q_ASSERT( m_config );
    if( !m_config )
    {
        error() << "No configuration exists, bailing out of import";
        return;
    }

    m_worker = new FastForwardWorker();
    m_worker->setDriver( m_config->connectionType() );
    m_worker->setDatabaseLocation( m_config->databaseLocation() );
    m_worker->setDatabase( m_config->databaseName() );

    // Work around strange QtSql DNS resolution bug. See bug #174269
    QString host = m_config->databaseHost();
    if( host == "localhost" && m_config->connectionType() == MySQL )
        host = "127.0.0.1";

    m_worker->setHostname( host );
    m_worker->setUsername( m_config->databaseUser() );
    m_worker->setPassword( m_config->databasePass() );
    m_worker->setImportArtwork( m_config->importArtwork() );
    m_worker->setImportArtworkDir( m_config->importArtworkDir() );

    connect( m_worker, SIGNAL( trackAdded( Meta::TrackPtr ) ), 
             this, SIGNAL( trackAdded( Meta::TrackPtr ) ), Qt::QueuedConnection );
    connect( m_worker, SIGNAL( importError( QString ) ),
             this, SIGNAL( importError( QString ) ), Qt::QueuedConnection );
    connect( m_worker, SIGNAL( done(ThreadWeaver::Job*) ), 
             this, SLOT( finishUp() ), Qt::QueuedConnection );
    connect( m_worker, SIGNAL( showMessage( QString ) ),
             this, SIGNAL( showMessage( QString ) ), Qt::QueuedConnection );

    ThreadWeaver::Weaver::instance()->enqueue( m_worker );
}

void
FastForwardImporter::finishUp()
{
    DEBUG_BLOCK
    m_worker->failed() ?
        emit( importFailed() ) :
        emit( importSucceeded() );
    
    delete m_worker;
}

