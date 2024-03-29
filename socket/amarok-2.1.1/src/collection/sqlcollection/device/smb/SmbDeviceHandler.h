/*
 *  Copyright (c) 2006-2007 Maximilian Kossick <maximilian.kossick@googlemail.com>
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
#ifndef SMBDEVICEHANDLER_H
#define SMBDEVICEHANDLER_H

#include <MountPointManager.h>

class SmbDeviceHandlerFactory : public DeviceHandlerFactory
{
public:
    SmbDeviceHandlerFactory();
    virtual ~SmbDeviceHandlerFactory();

    virtual bool canHandle( const Solid::Device &device ) const;

    virtual bool canCreateFromMedium() const;

    virtual DeviceHandler* createHandler( const Solid::Device &volume, const QString &udi ) const;

    virtual bool canCreateFromConfig() const;

    virtual DeviceHandler* createHandler( KSharedConfigPtr c ) const;

    virtual QString type() const;
};

/**
	@author Maximilian Kossick <maximilian.kossick@googlemail.com>
*/
class SmbDeviceHandler : public DeviceHandler
{
public:
    SmbDeviceHandler(int deviceId, QString server, QString dir, QString mountPoint );

    virtual ~SmbDeviceHandler();

    virtual bool isAvailable() const;
    virtual QString type() const;
    virtual int getDeviceID( );
    virtual const QString &getDevicePath() const;
    virtual void getURL( KUrl &absolutePath, const KUrl &relativePath );
    virtual void getPlayableURL( KUrl &absolutePath, const KUrl &relativePath );
    virtual bool deviceMatchesUdi( const QString &udi ) const;

private:

    int m_deviceID;
    const QString m_mountPoint;
    QString m_server;
    QString m_dir;

};

#endif
