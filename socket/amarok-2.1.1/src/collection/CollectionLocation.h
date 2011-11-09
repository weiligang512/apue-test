/*
 *  Copyright (c) 2007-2008 Maximilian Kossick <maximilian.kossick@googlemail.com>
 *  Copyright (c) 2008 Jason A. Donenfeld <Jason@zx2c4.com>
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
#ifndef AMAROK_COLLECTIONLOCATION_H
#define AMAROK_COLLECTIONLOCATION_H

#include "amarok_export.h"
#include "Meta.h"

#include <QList>
#include <QObject>

#include <KUrl>

class QueryMaker;

/**
    This base class defines the the methods necessary to allow the copying and moving 
    of tracks between different collections in a generic way.

    This class should be used as follows in client code:
    - select a source and a destination CollectionLocation
    - call prepareCopy or prepareMove on the source CollectionLocation
    - forget about the rest of the workflow

    Implementations which are writeable must reimplement the following methods
    - prettyLocation()
    - isWritable()
    - remove( Meta::Track )
    - copyUrlsToCollection( QMap<Meta::TrackPtr, KUrl> )

    Writable collections that are also organizable should reimplement isOrganizable().
    Organizable means that the user is able to decide (to varying degrees, the details
    depend on the actual collection) where the files are stored in the filesystem (or some 
    kind of VFS). An example would be the local collection, where the user can select the directory
    structure that Amarok uses to store the files. An example for a writable collection that is not
    organizable are ipods, where the user has no control about the actual location of the music files
    (they are automatically stored in a not human-readable form).

    Implementations which are only readable can reimplement getKIOCopyableUrls( Meta::TrackList )
    if it is necessary, but can use the default implementations if possible.

    Implementations that have a string expressable location(s), such as a URL or path on disk
    should reimplement actualLocation().

    Implementations that need additional information provided by the user have to implement
    showSourceDialog() and showDestinationDialog(), depending on whether the information is required
    in the source, the destination, or both.

    The methods will be called in the following order:
    startWorkflow (source)
    showSourceDialog (source) (reimplementable)
    slotShowSourceDialogDone (source)
    slotPrepareOperation (destination)
    showDestinationDialog (destination) (reimplementable)
    slotShowDestinationDialogDone (destination)
    slotOperationPrepared (source)
    getKIOCopyableUrls (source) (reimplementable)
    slotGetKIOCopyableUrlsDone (source)
    slotStartCopy (destination)
    copyUrlsToCollection (destination) (reimplementable)
    slotCopyOperationFinished (destination)
    slotFinishCopy (source)
*/

class AMAROK_EXPORT CollectionLocation : public QObject
{
    Q_OBJECT

    public:
        CollectionLocation();
        CollectionLocation( const Amarok::Collection* parentCollection );
        virtual  ~CollectionLocation();

        /**
            Returns a pointer to the collection location's corresponding collection.
            @return a pointer to the collection location's corresponding collection
         */
        const Amarok::Collection* collection() const;

        /**
            a displayable string representation of the collection location. use the return
            value of this method to display the collection location to the user.
            @return a string representation of the collection location
        */
        virtual QString prettyLocation() const;

        /**
            Returns a list of machine usable strings representingthe collection location.
            For example, a local collection would return a list of paths where tracks are 
            stored, while an Ampache collection would return a list with one string 
            containing the URL of an ampache server. An iPod collection and a MTP device 
            collection are examples of collections that do not need to reimplement this method.
        */
        virtual QStringList actualLocation() const;

        /**
            Returns whether the collection location is writeable or not. For example, a
            local collection or an ipod collection would return true, a daap collection 
            or a service collection false. The value returned by this method indicates 
            if it is possible to copy tracks to the collection, and if it is generally 
            possible to remove/delete files from the collection.
            @return @c true if the collection location is writeable
            @return @c false if the collection location is not writeable
        */
        virtual bool isWritable() const;

        /**
           Returns whether the collection is organizable or not. Organizable collections
           allow move operations where the source and destination collection are the same.
           @return @c true if the collection location is organizable, false otherwise
         */
        virtual bool isOrganizable() const;

        /**
           convenience method for copying a single track, 
           @see prepareCopy( Meta::TrackList, CollectionLocation* )
        */
        void prepareCopy( Meta::TrackPtr track, CollectionLocation *destination );
        void prepareCopy( const Meta::TrackList &tracks, CollectionLocation *destination );
        void prepareCopy( QueryMaker *qm, CollectionLocation *destination );

        /**
           convenience method for moving a single track, 
           @see prepareMove( Meta::TrackList, CollectionLocation* )
        */
        void prepareMove( Meta::TrackPtr track, CollectionLocation *destination );
        void prepareMove( const Meta::TrackList &tracks, CollectionLocation *destination );
        void prepareMove( QueryMaker *qm, CollectionLocation *destination );

        /**
           remove the track from the collection.
           Return true if the removal was successful, false otherwise.
         */
        virtual bool remove( const Meta::TrackPtr &track );

        
        /**
        * Sets or gets which files the source will potentially need to remove
        */
        virtual bool movedByDestination( const Meta::TrackPtr &track ) const;
        virtual bool consideredByDestination( const Meta::TrackPtr &track ) const;
        virtual void setMovedByDestination( const Meta::TrackPtr &track, bool removeFromDatabase );
    
        /**
        * tells the source location that an error occurred during the transfer of the file
        */
        virtual void transferError( const Meta::TrackPtr &track, const QString &error );
        
        /**
         * Inserts a set of TrackPtrs directly into the database without needing to actuall move any files
         * This is a hack required by the DatabaseImporter
         */
        virtual void insertTracks( const QMap<Meta::TrackPtr, QString> &trackMap ) { Q_UNUSED( trackMap ); }
        virtual void insertStatistics( const QMap<Meta::TrackPtr, QString> &trackMap ) { Q_UNUSED( trackMap ); }

    signals:
        void startCopy( const QMap<Meta::TrackPtr, KUrl> &sources );
        void finishCopy();
        void prepareOperation( const Meta::TrackList &tracks, bool removeSources );
        void operationPrepared();
        void aborted();

    protected:
        /**
         * aborts the workflow
         */
        void abort();

        /**
         * allows the destination location to access the source CollectionLocation.
         * note: subclasses do not take ownership  of the pointer
         */
        CollectionLocation* source() const;
        /**
            this method is called on the source location, and should return a list of urls
            which the destination location can copy using KIO. You must call 
            slotGetKIOCopyableUrlsDone( QMap<Meta::TrackPtr, KUrl> ) after retrieving the 
            urls. The order of urls passed to that method has to be the same as the order 
            of the tracks passed to this method.
        */
        virtual void getKIOCopyableUrls( const Meta::TrackList &tracks );
        /**
            this method is called on the destination. reimplement it if your implementation
            is writeable. you must call slotCopyOperationFinished() when you are done copying
            the files.
        */
        virtual void copyUrlsToCollection( const QMap<Meta::TrackPtr, KUrl> &sources );

        /**
         * this method is called on the source. It allows the source CollectionLocation to
         * show a dialog. Classes that reimplement this method must call 
         * slotShowSourceDialogDone() after they have acquired all necessary information from the user.
         */
        virtual void showSourceDialog( const Meta::TrackList &tracks, bool removeSources );

        /**
         * this method is called on the destination. It allows the destination 
         * CollectionLocation to show a dialog. Classes that reimplement this method 
         * must call slotShowDestinationDialogDone() after they have acquired all necessary 
         * information from the user.
         */
        virtual void showDestinationDialog( const Meta::TrackList &tracks, bool removeSources );

        /**
        * Sets or gets whether some source files may be removed
        */
        virtual bool isGoingToRemoveSources() const;
        virtual void setGoingToRemoveSources( bool removeSources );

    protected slots:
        /**
         * this slot has to be called from getKIOCopyableUrls( Meta::TrackList )
         * Please note: the order of urls in the argument has to be the same as in the
         * tracklist
         */
        void slotGetKIOCopyableUrlsDone( const QMap<Meta::TrackPtr, KUrl> &sources );
        void slotCopyOperationFinished();
        void slotShowSourceDialogDone();
        void slotShowDestinationDialogDone();

    private slots:

        void slotPrepareOperation( const Meta::TrackList &tracks, bool removeSources );
        void slotOperationPrepared();
        void slotStartCopy( const QMap<Meta::TrackPtr, KUrl> &sources );
        void slotFinishCopy();
        void slotAborted();
        void resultReady( const QString &collectionId, const Meta::TrackList &tracks );
        void queryDone();

    private:
        void setupConnections();
        void startWorkflow( const Meta::TrackList &tracks, bool removeSources );
        void removeSourceTracks( const Meta::TrackList &tracks );
        void setSource( CollectionLocation *source );

        //only used in the source CollectionLocation
        CollectionLocation * m_destination;
        //only used in destination CollectionLocation
        CollectionLocation *m_source;
        Meta::TrackList m_sourceTracks;

        const Amarok::Collection* m_parentCollection;
        
        bool m_removeSources;
        QMap<Meta::TrackPtr, bool> m_tracksRemovedByDestination;
        QMap<Meta::TrackPtr, QString> m_tracksWithError;
};

#endif
