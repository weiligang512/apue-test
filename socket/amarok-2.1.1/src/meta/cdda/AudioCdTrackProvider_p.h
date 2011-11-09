/* This file is part of the KDE project
   Copyright (C) 2008 Maximilian Kossick <maximilian.kossick@googlemail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#ifndef AMAROK_AUDIOCDTRACKPROVIDER_P_H
#define AMAROK_AUDIOCDTRACKPROVIDER_P_H

#include <QMap>
#include <QObject>
#include <QString>

class AudioCdTrackProvider::Private : public QObject
{
    Q_OBJECT
    public:
        Private();
        ~Private();

        bool isPathOnCd( const QString &path ) const;
        

    public slots:
        void deviceAdded( const QString &udi );
        void deviceRemoved( const QString &udi );

    private:
        QMap<QString, QString> m_cddaDevices;
};

#endif
