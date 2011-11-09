/***************************************************************************
*   Copyright (c) 2009  Casey Link <unnamedrambler@gmail.com>             *
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

#include "PlayUrlRunner.h"

#include "Debug.h"
#include "AmarokUrl.h"
#include "AmarokUrlHandler.h"
#include "collection/CollectionManager.h"
#include "EngineController.h"
#include "playlist/PlaylistController.h"
#include "SqlStorage.h"

PlayUrlRunner::PlayUrlRunner() : AmarokUrlRunnerBase()
{
}

PlayUrlRunner::~PlayUrlRunner()
{
    The::amarokUrlHandler()->unRegisterRunner ( this );
}

bool PlayUrlRunner::run ( AmarokUrl url )
{
    DEBUG_BLOCK
    if ( url.numberOfArgs() == 0 )
        return false;

    QUrl track_url = QUrl::fromEncoded ( QByteArray::fromBase64 ( url.arg ( 0 ).toUtf8() ) );
    debug() << "decoded track url: " << track_url.toString();
    int pos = url.arg ( 1 ).toInt() * 1000;
    debug() << "seek pos: " << pos;
    Meta::TrackPtr track = CollectionManager::instance()->trackForUrl ( track_url );
    if ( !track )
        return false;

//     The::playlistController()->insertOptioned( track, Playlist::AppendAndPlay );
    The::engineController()->play ( track, pos );
//     The::engineController()->seek(pos);
    return true;
}

QString PlayUrlRunner::command() const
{
    return "play";
}
BookmarkList PlayUrlRunner::bookmarksFromUrl ( KUrl url )
{
    DEBUG_BLOCK
    BookmarkList list;

    //See PlayUrlGenerator for the description of a 'play' amarokurl
    QString track_encoded = url.toEncoded().toBase64();

    // The last character of a base64 encoded string is always '=', which
    // chokes the SQL. Since we are using a substring like text comparison
    // and every url in the database will have the '=', just chop it off.
    track_encoded.chop ( 1 );

    // Queries the database for bookmarks where the url field contains
    // the base64 encoded url (minus the '=').
    QString query = "SELECT id, parent_id, name, url, description, custom FROM bookmarks WHERE url LIKE '%%1%'";
    query = query.arg ( track_encoded );
    debug() << "query: " << query;
    QStringList result = CollectionManager::instance()->sqlStorage()->query ( query );

    int resultRows = result.count() / 6;

    for ( int i = 0; i < resultRows; i++ )
    {
        QStringList row = result.mid ( i*6, 6 );
        list << AmarokUrlPtr ( new AmarokUrl ( row ) );
    }
    return list;
}
