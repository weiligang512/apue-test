/***************************************************************************
 * copyright            : (C) 2007-2008 Leo Franchi <lfranchi@gmail.com>   *
 * copyright            : (C) 2008 Mark Kretschmann <kretschmann@kde.org>  *
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "LyricsEngine.h"

#include "Amarok.h"
#include "Debug.h"
#include "ContextView.h"
#include "EngineController.h"
#include "ScriptManager.h"


using namespace Context;

LyricsEngine::LyricsEngine( QObject* parent, const QList<QVariant>& /*args*/ )
    : DataEngine( parent )
    , ContextObserver( ContextView::self() )
    , LyricsObserver( LyricsManager::self() )
{
    m_requested = true; // testing
}

QStringList LyricsEngine::sources() const
{
    QStringList sourcesList;
    sourcesList << "lyrics" << "suggested";

    return sourcesList;
}

bool LyricsEngine::sourceRequestEvent( const QString& name )
{
    Q_UNUSED( name )

    m_requested = true; // someone is asking for data, so we turn ourselves on :)
    removeAllData( name );
    setData( name, QVariant());
    update();

    return true;
}

void LyricsEngine::message( const ContextState& state )
{
    DEBUG_BLOCK

    if( state == Current && m_requested )
        update();
}

void LyricsEngine::metadataChanged( Meta::TrackPtr track )
{
    DEBUG_BLOCK

    const bool hasChanged = track->name() != m_title || 
                            track->artist()->name() != m_artist;

    if( hasChanged )
        update();
}

void LyricsEngine::update()
{
    DEBUG_BLOCK

    Meta::TrackPtr currentTrack = The::engineController()->currentTrack();
    if( !currentTrack || !currentTrack->artist() )
        return;

    unsubscribeFrom( m_currentTrack );
    m_currentTrack = currentTrack;
    subscribeTo( currentTrack );

    QString lyrics = currentTrack->cachedLyrics();
    
    // don't rely on caching for streams
    const bool cached = !lyrics.isEmpty() && !The::engineController()->isStream() && ( currentTrack->name() == m_title ) && ( currentTrack->artist()->name() == m_artist );
    
    m_title = currentTrack->name();
    m_artist = currentTrack->artist()->name();

    if( m_title.contains("PREVIEW: buy it at www.magnatune.com", Qt::CaseSensitive) )
        m_title = m_title.remove(" (PREVIEW: buy it at www.magnatune.com)");
    if( m_artist.contains("PREVIEW: buy it at www.magnatune.com", Qt::CaseSensitive) )
        m_artist = m_artist.remove(" (PREVIEW: buy it at www.magnatune.com)");

    if( m_title.isEmpty() )
    {
        /* If title is empty, try to use pretty title.
           The fact that it often (but not always) has "artist name" together, can be bad,
           but at least the user will hopefully get nice suggestions. */
        QString prettyTitle = The::engineController()->currentTrack()->prettyName();
        int h = prettyTitle.indexOf( '-' );
        if ( h != -1 )
        {
            m_title = prettyTitle.mid( h+1 ).trimmed();
            if( m_title.contains("PREVIEW: buy it at www.magnatune.com", Qt::CaseSensitive) )
                m_title = m_title.remove(" (PREVIEW: buy it at www.magnatune.com)");
            if( m_artist.isEmpty() ) {
                m_artist = prettyTitle.mid( 0, h ).trimmed();
                if( m_artist.contains("PREVIEW: buy it at www.magnatune.com", Qt::CaseSensitive) )
                    m_artist = m_artist.remove(" (PREVIEW: buy it at www.magnatune.com)");
            }
        }
    }

    if( cached )
    {
        if( lyrics.contains( "<html>" ) )
            newLyricsHtml( lyrics );
        else
        {
            QStringList info;
            info << m_title << m_artist << QString() <<  lyrics;
            newLyrics( info );
        }
    } else if( !ScriptManager::instance()->lyricsScriptRunning() ) // no lyrics, and no lyrics script!
    {
        removeAllData( "lyrics" );
        setData( "lyrics", "noscriptrunning", "noscriptrunning" );
        return;
    }
    else
    {
        // fetch by lyrics script
        removeAllData( "lyrics" );
        setData( "lyrics", "fetching", "fetching" );
        ScriptManager::instance()->notifyFetchLyrics( m_artist, m_title );
    }
}

void LyricsEngine::newLyrics( QStringList& lyrics )
{
    DEBUG_BLOCK

    removeAllData( "lyrics" );
    setData( "lyrics", "lyrics", lyrics );
}

void LyricsEngine::newLyricsHtml( QString& lyrics )
{
    removeAllData( "lyrics" );
    setData( "lyrics", "html", lyrics );
}

void LyricsEngine::newSuggestions( QStringList& suggested )
{
    DEBUG_BLOCK
    // each string is in "title - artist <url>" form
    removeAllData( "lyrics" );
    setData( "lyrics", "suggested", suggested );
}

void LyricsEngine::lyricsMessage( QString& key, QString &val )
{
    DEBUG_BLOCK

    removeAllData( "lyrics" );
    setData( "lyrics", key, val );
}

#include "LyricsEngine.moc"

