/* This file is part of the KDE project
   Copyright (C) 2007 Bart Cerneels <bart.cerneels@kde.org>

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

#ifndef PODCASTREADER_H
#define PODCASTREADER_H

#include "PodcastProvider.h"
#include "PodcastMeta.h"

#include <QDateTime>
#include <QXmlStreamReader>
#include <QObject>

namespace KIO
{
    class Job;
}

class KUrl;

/**
    @author Bart Cerneels <bart.cerneels@kde.org>
*/
class PodcastReader : public QObject, public QXmlStreamReader
{
    Q_OBJECT
    public:
        PodcastReader( PodcastProvider * podcastProvider );

        bool read( QIODevice *device );
        bool read( const KUrl &url );
        bool read();
        bool update( Meta::PodcastChannelPtr channel );
        KUrl & url() { return m_url; }

        ~PodcastReader();

    signals:
        void finished( PodcastReader *podcastReader, bool result );

    private slots:
        void slotRedirection( KIO::Job *job, const KUrl & url );
        void slotPermanentRedirection ( KIO::Job * job, const KUrl & fromUrl,
                const KUrl & toUrl );
        void slotAbort();
        void slotAddData( KIO::Job *, const QByteArray & data );

        void downloadResult( KJob * );

    private:
        KUrl m_url;
        PodcastProvider * m_podcastProvider;
        Meta::PodcastMetaCommon *m_current;
        Meta::PodcastChannelPtr m_channel;

        QString m_currentTag;
        QString m_titleString;
        QString m_linkString;
        QString m_descriptionString;
        QString m_urlString;
        QString m_guidString;
        QString m_pubDateString;

        bool m_parsingImage;

        void readUnknownElement();

        QDateTime parsePubDate( const QString &datestring );

        /** podcastEpisodeCheck
        * Check if this PodcastEpisode has been fetched before. Uses a scoring algorithm.
        * @return A pointer to a PodcastEpisode that has been fetched before or the \
        *   same pointer as the argument.
        */
        Meta::PodcastEpisodePtr podcastEpisodeCheck( Meta::PodcastEpisodePtr episode );

        void commitChannel();
        void commitEpisode();
};

#endif
