/*****************************************************************************
* copyright            : (C) 2008 Alejandro Wainzinger <aikawarazuni@gmail.com>
*
 *****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IPOD_DEVICE_INFO_H
#define IPOD_DEVICE_INFO_H

#include "MediaDeviceInfo.h"

#include "mediadevicecollection_export.h"

class MEDIADEVICECOLLECTION_EXPORT IpodDeviceInfo : public MediaDeviceInfo
{
    Q_OBJECT
    public:
        IpodDeviceInfo( QString mountpoint, QString udi );
        ~IpodDeviceInfo();

        QString mountpoint();

    private:
        QString m_mountpoint;
};

#endif
