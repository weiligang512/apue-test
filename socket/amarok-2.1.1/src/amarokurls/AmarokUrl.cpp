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
 
#include "AmarokUrl.h"

#include "AmarokUrlHandler.h"
#include "BookmarkGroup.h"
#include "CollectionManager.h"
#include "Debug.h"
#include "SqlStorage.h"

#include <QUrl>

AmarokUrl::AmarokUrl()
    : m_id( -1 )
    , m_parent( 0 )
{}

AmarokUrl::AmarokUrl( const QString & urlString, BookmarkGroupPtr parent )
    : m_id( -1 )
    , m_parent( parent )
{
    initFromString( urlString );
}

AmarokUrl::AmarokUrl( const QStringList & resultRow, BookmarkGroupPtr parent )
    : m_parent( parent )
{
    m_id = resultRow[0].toInt();
    m_name = resultRow[2];
    const QString urlString = resultRow[3];
    m_description = resultRow[4];
    m_customValue = resultRow[5];

    initFromString( urlString );
}

AmarokUrl::~AmarokUrl()
{}

void AmarokUrl::initFromString( const QString & urlString )
{
    //first, strip amarok://

    QString strippedUrlString = urlString;

    strippedUrlString = strippedUrlString.replace( "amarok://", "" );

    //some poor man's unescaping
    strippedUrlString = strippedUrlString.replace( "%22", "\"" );
    strippedUrlString = strippedUrlString.replace( "%20", " " );
    
    m_fields = strippedUrlString.split( '/' );
}

void AmarokUrl::setCommand( const QString & command )
{
    if ( m_fields.count() > 0 )
        m_fields[0] = command;
    else
        m_fields << command;
}

QString AmarokUrl::command()
{
    if ( m_fields.count() != 0 )
        return m_fields[0];
    else
        return QString();
}

int AmarokUrl::numberOfArgs()
{
    if ( m_fields.count() != 0 )
        return m_fields.count() - 1;
    else
        return 0;
}

void AmarokUrl::appendArg( const QString & arg )
{
    if ( m_fields.count() > 0 )
        m_fields << arg;
    else
        m_fields << QString() << arg; //reserve space for command
}

QString AmarokUrl::arg( int arg )
{
    if ( m_fields.count() != 0 )
        return m_fields[arg + 1];
    else
        return ""; 
}

bool AmarokUrl::run()
{
    DEBUG_BLOCK
    return The::amarokUrlHandler()->run( *this );
}

QString AmarokUrl::url()
{
    QString url = "amarok:/";
    foreach( const QString &field, m_fields ) {
        url += '/';
        url += field;
    }

    //escape it
    url = url.replace( ' ', "%20" );
    url = url.replace( "\"", "%22" );

    return url;
}

bool AmarokUrl::saveToDb()
{
    DEBUG_BLOCK

    if ( isNull() )
        return false;

    const int parentId = m_parent ? m_parent->id() : -1;

    SqlStorage * sql =  CollectionManager::instance()->sqlStorage();

    if( m_id != -1 )
    {
        //update existing
        debug() << "Updating bookmark";
        QString query = "UPDATE bookmarks SET parent_id=%1, name='%2', url='%3', description='%4', custom='%5' WHERE id=%6;";
        query = query.arg( QString::number( parentId ) ).arg( sql->escape( m_name ) ).arg( sql->escape( url() ) ).arg( sql->escape( m_description ) ).arg( sql->escape( m_customValue ) ).arg( QString::number( m_id ) );
        CollectionManager::instance()->sqlStorage()->query( query );
    }
    else
    {
        //insert new
        debug() << "Creating new bookmark in the db";
        QString query = "INSERT INTO bookmarks ( parent_id, name, url, description, custom ) VALUES ( %1, '%2', '%3', '%4', '%5' );";
        query = query.arg( QString::number( parentId ) ).arg( sql->escape( m_name ) ).arg( sql->escape( url() ) ).arg( sql->escape( m_description ) ).arg( sql->escape( m_customValue ) );
        m_id = CollectionManager::instance()->sqlStorage()->insert( query, NULL );
    }

    return true;
}

void AmarokUrl::setName( const QString & name )
{
    m_name = name;
}

QString AmarokUrl::name() const
{
    return m_name;
}

void AmarokUrl::setDescription( const QString & description )
{
    m_description = description;
}

QString AmarokUrl::description() const
{
    return m_description;
}

void AmarokUrl::removeFromDb()
{
    QString query = "DELETE FROM bookmarks WHERE id=%1";
    query = query.arg( QString::number( m_id ) );
    CollectionManager::instance()->sqlStorage()->query( query );
}

void AmarokUrl::rename( const QString &name )
{
    m_name = name;
    if ( m_id != -1 )
        saveToDb();
}

bool AmarokUrl::isNull()
{
    return m_fields.count() == 0;
}

void AmarokUrl::reparent( BookmarkGroupPtr parent )
{
    m_parent = parent;
    saveToDb();
}

void AmarokUrl::setCustomValue( const QString & custom )
{
    m_customValue = custom;
}

QString AmarokUrl::customValue()
{
    return m_customValue;
}

