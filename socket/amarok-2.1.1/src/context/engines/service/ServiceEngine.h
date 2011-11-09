/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *             (c) 2007  Leo Franchi <lfranchi@gmail.com>                  * 
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

#ifndef AMAROK_SERVICE_ENGINE
#define AMAROK_SERVICE_ENGINE

#include "ContextObserver.h"
#include "services/ServiceInfoObserver.h"
#include "context/DataEngine.h"

/**
    This class provides context information realted to the currently active service 

    There is no data source: if you connect to the engine, you immediately
    start getting updates when there is data. 

    The key of the data is "service".
    The data is a QMap with the keys
        * service_name - the name of the currently running service
 

*/

class ServiceEngine : public Context::DataEngine, 
                      public ServiceInfoObserver,
                      public ContextObserver
{
    Q_OBJECT

    
public:

    ServiceEngine( QObject* parent, const QList<QVariant>& args );
    ~ServiceEngine();

    QStringList sources() const;
    void message( const Context::ContextState& state );

    void serviceInfoChanged( QVariantMap infoMap );

protected:
    bool sourceRequestEvent( const QString& name );
    
private:
    void update();

    QStringList m_sources;
    bool m_requested;
    QVariantMap m_storedInfo;

};

K_EXPORT_AMAROK_DATAENGINE( service, ServiceEngine )

#endif
