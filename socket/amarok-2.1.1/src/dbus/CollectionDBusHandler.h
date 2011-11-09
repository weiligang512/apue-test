/*
 *  Copyright 2009 Maximilian Kossick <maximilian.kossick@googlemail.com>
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
 
#ifndef COLLECTIONDBUSHANDLER_H
#define COLLECTIONDBUSHANDLER_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>

class CollectionDBusHandler : public QObject
{
    Q_OBJECT
    Q_CLASSINFO( "Collection D-Bus Interface", "org.kde.amarok.Collection" )
    
    public:
        CollectionDBusHandler( QObject *parent );
        
    public slots:
        /*
         * Takes a query in XML form and executes it. Amarok runs queries asynchronously, therefore the result
         * of the query will be returned by the queryResult() signal. The return value of this method is a token
         * that uniquely identifies the query. It will also be the first parameter of the queryResult() signal
         * for the result of the query. Will return an empty string if the XML query is invalid.
         */
        QString query( const QString &xmlQuery );
        
    signals:
        void queryResult( const QString& token, const QList<QMap<QString, QVariant> > &result );
        
};

#endif
