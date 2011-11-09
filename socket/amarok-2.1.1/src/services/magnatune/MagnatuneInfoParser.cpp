/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#include "MagnatuneInfoParser.h"

#include "Debug.h"
#include "statusbar/StatusBar.h"

#include <KLocale>


using namespace Meta;

void MagnatuneInfoParser::getInfo(ArtistPtr artist)
{

    showLoading( i18n( "Loading artist info..." ) );

    MagnatuneArtist * magnatuneArtist = dynamic_cast<MagnatuneArtist *>( artist.data() );
    if ( magnatuneArtist == 0) return;

    debug() << "MagnatuneInfoParser: getInfo about artist";

    // first get the entire artist html page
   /* QString tempFile;
    QString orgHtml;*/

    m_infoDownloadJob = KIO::storedGet( magnatuneArtist->magnatuneUrl(), KIO::Reload, KIO::HideProgressInfo );
    The::statusBar()->newProgressOperation( m_infoDownloadJob, i18n( "Fetching %1 Artist Info", magnatuneArtist->prettyName() ) );
    connect( m_infoDownloadJob, SIGNAL(result(KJob *)), SLOT( artistInfoDownloadComplete( KJob*) ) );

}


void MagnatuneInfoParser::getInfo(AlbumPtr album)
{

    showLoading( i18n( "Loading album info..." ) );
    
    MagnatuneAlbum * magnatuneAlbum = dynamic_cast<MagnatuneAlbum *>( album.data() );

    const QString artistName = album->albumArtist()->name();

    QString infoHtml = "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" "
                       "CONTENT=\"text/html; charset=utf-8\"></HEAD><BODY>";

    infoHtml += "<div align=\"center\"><strong>";
    infoHtml += artistName;
    infoHtml += "</strong><br><em>";
    infoHtml += magnatuneAlbum->name();
    infoHtml += "</em><br><br>";
    infoHtml += "<img src=\"" + magnatuneAlbum->coverUrl() +
                "\" align=\"middle\" border=\"1\">";

    // Disable Genre line in Magnatune applet since, well, it doesn't actually put a genre there...
    // Nikolaj, FYI: either the thumbnails aren't working, or they aren't getting through the proxy here.  That would be odd, however, as the database and
    // all HTML are coming through the proxy
    //infoHtml += "<br><br>" + i18n( "Genre: ");// + magnatuneAlbum->
    infoHtml += "<br>" + i18n( "Release Year: ") + QString::number( magnatuneAlbum->launchYear() );

    if ( !magnatuneAlbum->description().isEmpty() ) {

        //debug() << "MagnatuneInfoParser: Writing description: '" << album->getDescription() << "'";
        infoHtml += "<br><br><b>" + i18n( "Description:" ) + "</b><br><p align=\"left\" >" + magnatuneAlbum->description();

    }

    infoHtml += "</p><br><br>" + i18n( "From Magnatune.com" ) + "</div>";
    infoHtml += "</BODY></HTML>";

    emit ( info( infoHtml ) );
}

void MagnatuneInfoParser::getInfo(TrackPtr track)
{
    Q_UNUSED( track );
    return;
}




void
MagnatuneInfoParser::artistInfoDownloadComplete( KJob *downLoadJob )
{

    if ( !downLoadJob->error() == 0 )
    {
        //TODO: error handling here
        return ;
    }
    if ( downLoadJob != m_infoDownloadJob )
        return ; //not the right job, so let's ignore it



    QString infoString = ( (KIO::StoredTransferJob* ) downLoadJob )->data();
    //debug() << "MagnatuneInfoParser: Artist info downloaded: " << infoString;
    infoString = extractArtistInfo( infoString );

    //debug() << "html: " << trimmedInfo;

    emit ( info( infoString ) );

}

QString
MagnatuneInfoParser::extractArtistInfo( const QString &artistPage )
{
    QString trimmedHtml;


    int sectionStart = artistPage.indexOf( "<!-- ARTISTBODY -->" );
    int sectionEnd = artistPage.indexOf( "<!-- /ARTISTBODY -->", sectionStart );

    trimmedHtml = artistPage.mid( sectionStart, sectionEnd - sectionStart );

    int buyStartIndex = trimmedHtml.indexOf( "<!-- PURCHASE -->" );
    int buyEndIndex;

    //we are going to integrate the buying of music (I hope) so remove these links

    while ( buyStartIndex != -1 )
    {
        buyEndIndex = trimmedHtml.indexOf( "<!-- /PURCHASE -->", buyStartIndex ) + 18;
        trimmedHtml.remove( buyStartIndex, buyEndIndex - buyStartIndex );
        buyStartIndex = trimmedHtml.indexOf( "<!-- PURCHASE -->", buyStartIndex );
    }


    QString infoHtml = "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" "
                       "CONTENT=\"text/html; charset=iso-8859-1\"></HEAD><BODY>";

    infoHtml += trimmedHtml;
    infoHtml += "</BODY></HTML>";


    return infoHtml;
}

void MagnatuneInfoParser::getFrontPage()
{
    showLoading( i18n( "Loading Magnatune.com frontpage..." ) );
    
    m_frontPageDownloadJob = KIO::storedGet( KUrl( "http://magnatune.com/amarok_frontpage.html" ), KIO::Reload, KIO::HideProgressInfo );
    The::statusBar()->newProgressOperation( m_frontPageDownloadJob, i18n( "Fetching Magnatune.com front page" ) );
    connect( m_frontPageDownloadJob, SIGNAL(result(KJob *)), SLOT( frontPageDownloadComplete( KJob*) ) );
}

void MagnatuneInfoParser::frontPageDownloadComplete(KJob * downLoadJob)
{
    if ( !downLoadJob->error() == 0 )
    {
        //TODO: error handling here
        return ;
    }
    if ( downLoadJob != m_frontPageDownloadJob )
        return ; //not the right job, so let's ignore it



    QString infoString = ((KIO::StoredTransferJob* )downLoadJob)->data();
    emit ( info( infoString ) );
}


#include "MagnatuneInfoParser.moc"

