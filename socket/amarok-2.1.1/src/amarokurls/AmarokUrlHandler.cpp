/***************************************************************************
 *   Copyright (c) 2008  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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

#include "AmarokUrlHandler.h"

#include "BookmarkMetaActions.h"
#include "Debug.h"
#include "GlobalCurrentTrackActions.h"
#include "NavigationUrlGenerator.h"
#include "NavigationUrlRunner.h"
#include "PlayUrlRunner.h"
#include "BookmarkModel.h"
#include "timecode/TimecodeObserver.h"

#include <KIcon>

namespace The {
    static AmarokUrlHandler* s_AmarokUrlHandler_instance = 0;

    AmarokUrlHandler* amarokUrlHandler()
    {
        if( !s_AmarokUrlHandler_instance )
            s_AmarokUrlHandler_instance = new AmarokUrlHandler();

        return s_AmarokUrlHandler_instance;
    }
}

AmarokUrlHandler::AmarokUrlHandler()
    : QObject()
    , m_navigationRunner( 0 )
    , m_playRunner ( 0 )
    , m_timecodeObserver( 0 )
{
    //we init some of the default runners here.
    m_navigationRunner = new NavigationUrlRunner();
    m_playRunner = new PlayUrlRunner();
    m_timecodeObserver = new TimecodeObserver();
    registerRunner( m_navigationRunner, m_navigationRunner->command() );
    registerRunner( m_playRunner, m_playRunner->command() );

    //The::globalCollectionActions()->addAlbumAction( new BookmarkAlbumAction( this ) );
    //The::globalCollectionActions()->addArtistAction( new BookmarkArtistAction( this ) );
}


AmarokUrlHandler::~AmarokUrlHandler()
{
    delete m_navigationRunner;
}

void AmarokUrlHandler::registerRunner( AmarokUrlRunnerBase * runner, const QString & command )
{
    m_registeredRunners.insert( command, runner );
}

void AmarokUrlHandler::unRegisterRunner( AmarokUrlRunnerBase * runner )
{
    //get the key of the runner
    QString key = m_registeredRunners.key( runner, QString() );

    if ( !key.isEmpty() )
        m_registeredRunners.remove( key );
}

bool AmarokUrlHandler::run( AmarokUrl url )
{

    DEBUG_BLOCK

    QString command = url.command();

    debug() << "command: " << command;
    debug() << "registered commands: " << m_registeredRunners.keys();

    if ( m_registeredRunners.contains( command ) )
        return m_registeredRunners.value( command )->run( url );
    else
        return false;

}

void AmarokUrlHandler::bookmarkAlbum( Meta::AlbumPtr album ) //slot
{
    NavigationUrlGenerator generator;
    generator.urlFromAlbum( album ).saveToDb();
    BookmarkModel::instance()->reloadFromDb();
}

void AmarokUrlHandler::bookmarkArtist( Meta::ArtistPtr artist ) //slot
{
    NavigationUrlGenerator generator;
    generator.urlFromArtist( artist ).saveToDb();
    BookmarkModel::instance()->reloadFromDb();
}

#include "AmarokUrlHandler.moc"
