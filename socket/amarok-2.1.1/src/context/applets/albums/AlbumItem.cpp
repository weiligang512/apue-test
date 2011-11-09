/*******************************************************************************
* copyright              : (C) 2008 Seb Ruiz <ruiz@kde.org>                    *
*                                                                              *
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#include "AlbumItem.h"

#include <KLocale>

#include <QIcon>
#include <QPixmap>

AlbumItem::AlbumItem()
    : QStandardItem()
    , m_iconSize( 40 )
    , m_showArtist( false )
{
    setEditable( false );
}

void
AlbumItem::setAlbum( Meta::AlbumPtr albumPtr )
{
    if( m_album )
        unsubscribeFrom( m_album );
    m_album = albumPtr;
    subscribeTo( m_album );

    metadataChanged( m_album );
}

void
AlbumItem::setIconSize( const int iconSize )
{
    static const int padding = 5;

    m_iconSize = iconSize;

    QSize size = sizeHint();
    size.setHeight( iconSize + padding*2 );
    setSizeHint( size );
}

void
AlbumItem::setShowArtist( const bool showArtist )
{
    if( showArtist != m_showArtist )
    {
        m_showArtist = showArtist;
        metadataChanged( m_album );
    }
}

void
AlbumItem::metadataChanged( Meta::AlbumPtr album )
{
    if( !album )
        return;

    QString albumName = album->name();
    albumName = albumName.isEmpty() ? i18n("Unknown") : albumName;

    QString displayText = albumName;
    Meta::TrackList tracks = album->tracks();

    QString year;
    if( !tracks.isEmpty() )
    {
        Meta::TrackPtr first = tracks.first();
        year = first->year()->name();
        // do some sanity checking
        if( year.length() != 4 )
            year.clear();
    }

    if( !year.isEmpty() )
        displayText += QString( " (%1)" ).arg( year );

    if( m_showArtist && album->hasAlbumArtist() )
        displayText = QString( "%1 - %2" ).arg( album->albumArtist()->name(), displayText );

    QString trackCount = i18np( "%1 track", "%1 tracks", album->tracks().size() );
    displayText += '\n' + trackCount;

    setText( displayText );

    QPixmap cover = album->imageWithBorder( m_iconSize, 3 );
    setIcon( QIcon( cover ) );
}


