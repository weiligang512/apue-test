/***************************************************************************
 *   Copyright (c) 2007  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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

#ifndef SERVICEALBUMCOVERDOWNLOADER_H
#define SERVICEALBUMCOVERDOWNLOADER_H

#include "ServiceMetaBase.h"
#include "amarok_export.h"

#include <kio/job.h>
#include <kio/jobclasses.h>
#include <KTempDir>

#include <QObject>

namespace Meta
{

//forward declaration
class ServiceAlbumCoverDownloader;
class ServiceAlbumWithCover;

typedef KSharedPtr<ServiceAlbumWithCover> ServiceAlbumWithCoverPtr;

/**
A specialized ServiceAlbum that supports fetching its cover from a known url.
Takes care of caching covers.

    @author  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>
*/
class AMAROK_EXPORT ServiceAlbumWithCover : public ServiceAlbum
{

public:

    /**
     * Constructor, reimplemented from ServiceAlbum.
     * @param name The name of the album.
     */
    ServiceAlbumWithCover( const QString &name );

    /**
     * Constructor, reimplemented from ServiceAlbum.
     * @param name The result list used to initialize the album.
     */
    ServiceAlbumWithCover( const QStringList &resultRow );

    /**
     * Destructor.
     */
    virtual ~ServiceAlbumWithCover();

    /**
     * Get the download prefix used for caching the cover.
     * @return The prefix, most often the name of the parent service.
     */
    virtual QString downloadPrefix() const = 0;

    /**
     * Set the url from where to fetch the cover.
     * @param coverUrl The cover url.
     */
    virtual void setCoverUrl( const QString &coverUrl ) = 0;

    /**
     * Ger the cover url
     * @return The url of the cover.
     */
    virtual QString coverUrl() const = 0;

    /**
     * Set the cover image of thes album.
     * @param image The cover image.
     */
    void setImage( const QPixmap &image );

    /**
     * Notify album that the download of the cover has been cancelled.
     */
    void imageDownloadCanceled() const;

    /**
     * Reimplemented from Meta::Album. Get whether an album has a cover. This function always returns true to avoid the standard cover fetcher kicking in.
     * @param size Unused.
     * @return True.
     */
    virtual bool hasImage( int size = 1 ) const { Q_UNUSED( size ); return true; }

    /**
     * Get the image of this album. If the image has not yet been fetched, this call will return a standard
     * "no-cover" cover and then start the download of the real cover. When the download is complete, any oberserves will be notified that the metadata of this album has been changed.
     * @param size The required size of the album.
     * @param withShadow Unused.
     * @return The cover image or a default cover.
     */
    virtual QPixmap image( int size = 1 ); //overridden from Meta::Album

protected:

    mutable QPixmap *m_cover;
    mutable bool m_hasFetchedCover;
    mutable bool m_isFetchingCover;
    QString m_coverDownloadPath;
    mutable ServiceAlbumCoverDownloader *m_coverDownloader;
};



/**
A helper class for fetching covers from online services, used by ServiceAlbumWithCover

    @author  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>
*/
class ServiceAlbumCoverDownloader : public QObject
{
    Q_OBJECT

    public:

        /**
         * Constructor.
         */
        ServiceAlbumCoverDownloader();

        /**
         * Destructor.
         */
        ~ServiceAlbumCoverDownloader();

        /**
         * Start the download of the cover of a ServiceAlbumWithCover.
         * @param album The albumwhose cover should be downloaded.
         */
        void downloadCover( Meta::ServiceAlbumWithCover * album );

    private slots:

        /**
         * Slot called when the download job is complete.
         * @param downloadJob The job responsible for the cover download.
         */
        void coverDownloadComplete( KJob * downloadJob );

        /**
         * Slot called if the download job is cancelled.
         * @param downloadJob The job responsible for the cover download.
         */
        void coverDownloadCanceled( KJob * downloadJob );
    private:
        ServiceAlbumWithCover * m_album;
        QString m_coverDownloadPath;
        KIO::FileCopyJob * m_albumDownloadJob;
        KTempDir * m_tempDir;
};

}

#endif
