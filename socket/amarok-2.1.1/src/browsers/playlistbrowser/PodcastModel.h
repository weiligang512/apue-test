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

#ifndef PLAYLISTBROWSERNSPODCASTMODEL_H
#define PLAYLISTBROWSERNSPODCASTMODEL_H

#include "PodcastMeta.h"
#include "playlist/PlaylistModel.h"
#include "playlist/PlaylistController.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QVariant>

class PopupDropperAction;

namespace PlaylistBrowserNS {

enum {
    ShortDescriptionRole = Qt::UserRole + 1,
    LongDescriptionRole,
    //Where is this Playlist from (collection, service, device)
    OriginRole = Qt::UserRole,
    OnDiskRole = Qt::UserRole //Is the PodcastEpisode downloaded to disk?
};

/**
	@author Bart Cerneels
*/
class PodcastModel : public QAbstractItemModel
{
    Q_OBJECT
    public:
        static PodcastModel *instance();
        static void destroy();

        virtual QVariant data(const QModelIndex &index, int role) const;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const;
        virtual QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const;
        virtual QModelIndex index(int row, int column,
                        const QModelIndex &parent = QModelIndex()) const;
        virtual QModelIndex parent(const QModelIndex &index) const;
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

        virtual QStringList mimeTypes() const;
        QMimeData* mimeData( const QModelIndexList &indexes ) const;
        bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );

        QList<PopupDropperAction *> actionsFor( const QModelIndexList &indexes );

        void loadItems( QModelIndexList list, Playlist::AddOptions insertMode );
        void downloadItems(  QModelIndexList list );
        void deleteItems(  QModelIndexList list );
        void refreshItems( QModelIndexList list );
        void removeSubscription( QModelIndexList list );
        void configureChannels( QModelIndexList list );

        /** @returns all channels currently selected
        **/
        Meta::PodcastChannelList selectedChannels() { return m_selectedChannels; }

        /** @returns all episodes currently selected, this includes children of a selected
        * channel
        **/
        Meta::PodcastEpisodeList selectedEpisodes() { return m_selectedEpisodes; }

    public slots:
        void slotUpdate();
        void addPodcast();
        void refreshPodcasts();
        void setPodcastsInterval();
        void emitLayoutChanged();

    private slots:
        void slotAppend();
        void slotLoad();

    private:
        static PodcastModel* s_instance;
        PodcastModel();
        ~PodcastModel();

        Q_DISABLE_COPY( PodcastModel )

        Meta::PodcastChannelList selectedChannels( const QModelIndexList &indices );
        Meta::PodcastEpisodeList selectedEpisodes( const QModelIndexList &indices );
        QList<PopupDropperAction *> createCommonActions( QModelIndexList indices );
        PopupDropperAction * m_appendAction;
        PopupDropperAction * m_loadAction;
        Meta::PodcastEpisodeList m_selectedEpisodes;
        Meta::PodcastChannelList m_selectedChannels;

        /** A convenience function to convert a PodcastEpisodeList into a TrackList.
        **/
        static Meta::TrackList
        podcastEpisodesToTracks(
            Meta::PodcastEpisodeList episodes );

        void downloadEpisode( Meta::PodcastEpisodePtr episode );
        void deleteDownloadedEpisode( Meta::PodcastEpisodePtr episode );
        void refreshPodcast( Meta::PodcastChannelPtr channel );
        Meta::PodcastChannelList m_channels;
        void removeSubscription( Meta::PodcastChannelPtr channel );
        void configureChannel( Meta::PodcastChannelPtr channel );
};

}

namespace The {
    AMAROK_EXPORT PlaylistBrowserNS::PodcastModel* podcastModel();
}

#endif
