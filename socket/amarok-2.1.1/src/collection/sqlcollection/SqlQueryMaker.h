/*
 *  Copyright (c) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>
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
#ifndef AMAROK_COLLECTION_SQLQUERYMAKER_H
#define AMAROK_COLLECTION_SQLQUERYMAKER_H

#include "QueryMaker.h"

#include "amarok_export.h"

#include <threadweaver/Job.h>

class SqlCollection;

class /*AMAROK_EXPORT*/ SqlQueryMaker : public QueryMaker
{
    Q_OBJECT

    public:
        SqlQueryMaker( SqlCollection* collection );
        virtual ~SqlQueryMaker();

        virtual QueryMaker* reset();
        virtual void abortQuery();
        virtual void run();

        virtual QueryMaker* setQueryType( QueryType type );

        virtual QueryMaker* setReturnResultAsDataPtrs( bool resultAsDataPtrs );

        virtual QueryMaker* includeCollection( const QString &collectionId );
        virtual QueryMaker* excludeCollection( const QString &collectionId );

        virtual QueryMaker* addMatch( const Meta::TrackPtr &track );
        virtual QueryMaker* addMatch( const Meta::ArtistPtr &artist );
        virtual QueryMaker* addMatch( const Meta::AlbumPtr &album );
        virtual QueryMaker* addMatch( const Meta::ComposerPtr &composer );
        virtual QueryMaker* addMatch( const Meta::GenrePtr &genre );
        virtual QueryMaker* addMatch( const Meta::YearPtr &year );
        virtual QueryMaker* addMatch( const Meta::DataPtr &data );

        virtual QueryMaker* addFilter( qint64 value, const QString &filter, bool matchBegin, bool matchEnd );
        virtual QueryMaker* excludeFilter( qint64 value, const QString &filter, bool matchBegin, bool matchEnd );

        virtual QueryMaker* addNumberFilter( qint64 value, qint64 filter, NumberComparison compare );
        virtual QueryMaker* excludeNumberFilter( qint64 value, qint64 filter, NumberComparison compare );

        virtual QueryMaker* addReturnValue( qint64 value );
        virtual QueryMaker* addReturnFunction( ReturnFunction function, qint64 value );
        virtual QueryMaker* orderBy( qint64 value, bool descending = false );
        virtual QueryMaker* orderByRandom();

        virtual QueryMaker* limitMaxResultSize( int size );

        virtual QueryMaker* setAlbumQueryMode( AlbumQueryMode mode );

        virtual QueryMaker* beginAnd();
        virtual QueryMaker* beginOr();
        virtual QueryMaker* endAndOr();

        QString query();
        QStringList runQuery( const QString &query );
        void handleResult( const QStringList &result );

        // for using it blocking (only for collection internal use)

        void setBlocking( bool enabled );
        
        QStringList collectionIds() const;

        Meta::DataList data( const QString &id ) const;
        Meta::TrackList tracks( const QString &id ) const;
        Meta::AlbumList albums( const QString &id ) const;
        Meta::ArtistList artists( const QString &id ) const;
        Meta::GenreList genres( const QString &id ) const;
        Meta::ComposerList composers( const QString &id ) const;
        Meta::YearList years( const QString &id ) const;
        QStringList customData( const QString &id ) const;

    protected:
        virtual QString escape( QString text ) const;

        /** returns a pattern for LIKE operator that will match given text with given options
         *
         * @param text the text to match (should not be escape()'d, function does it itself)
         */
        virtual QString likeCondition( const QString &text, bool anyBegin, bool anyEnd ) const;

    public slots:
        void done( ThreadWeaver::Job * job );

    private:

        void linkTables();
        void buildQuery();

        QString nameForValue( qint64 value );
        QString andOr() const;

        void handleTracks( const QStringList &result );
        void handleArtists( const QStringList &result );
        void handleAlbums( const QStringList &result );
        void handleGenres( const QStringList &result );
        void handleComposers( const QStringList &result );
        void handleYears( const QStringList &result );

        SqlCollection *m_collection;

        struct Private;
        Private * const d;

};

#endif /* AMAROK_COLLECTION_SQLQUERYMAKER_H */
