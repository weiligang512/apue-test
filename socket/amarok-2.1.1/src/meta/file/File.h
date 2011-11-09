/*
   Copyright (C) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>

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

#ifndef AMAROK_META_FILE_H
#define AMAROK_META_FILE_H

#include "Meta.h"

namespace MetaFile
{
    class AMAROK_EXPORT Track : public Meta::Track
    {
        public:
            Track( const KUrl &url );
            virtual ~Track();

        //methods inherited from Meta::MetaBase
            virtual QString name() const;
            virtual QString prettyName() const;
            virtual QString fullPrettyName() const;
            virtual QString sortableName() const;

        //methods inherited from Meta::Track
            virtual KUrl playableUrl() const;
            virtual QString prettyUrl() const;
            virtual QString uidUrl() const;

            virtual bool isPlayable() const;
            virtual bool isEditable() const;

            virtual Meta::AlbumPtr album() const;
            virtual Meta::ArtistPtr artist() const;
            virtual Meta::GenrePtr genre() const;
            virtual Meta::ComposerPtr composer() const;
            virtual Meta::YearPtr year() const;

            virtual void setAlbum( const QString &newAlbum );
            virtual void setArtist( const QString &newArtist );
            virtual void setGenre( const QString &newGenre );
            virtual void setComposer( const QString &newComposer );
            virtual void setYear( const QString &newYear );

            virtual void setTitle( const QString &newTitle );

            virtual QString comment() const;
            virtual void setComment( const QString &newComment );

            virtual double score() const;
            virtual void setScore( double newScore );

            virtual int rating() const;
            virtual void setRating( int newRating );

            virtual int trackNumber() const;
            virtual void setTrackNumber( int newTrackNumber );

            virtual int discNumber() const;
            virtual void setDiscNumber( int newDiscNumber );

            virtual int length() const;
            virtual int filesize() const;
            virtual int sampleRate() const;
            virtual int bitrate() const;
            virtual uint lastPlayed() const;
            virtual void setLastPlayed( uint newTime );
            virtual uint firstPlayed() const;
            virtual void setFirstPlayed( uint newTime );
            virtual int playCount() const;
            virtual void setPlayCount( int newCount );

            virtual qreal replayGain( ReplayGainMode mode ) const;
            virtual qreal replayPeakGain( ReplayGainMode mode ) const;

            virtual QString type() const;

            virtual void beginMetaDataUpdate();
            virtual void endMetaDataUpdate();
            virtual void abortMetaDataUpdate();

            virtual void finishedPlaying( double playedFraction );

            virtual bool inCollection() const;
            virtual Amarok::Collection *collection() const;

            virtual bool hasCapabilityInterface( Meta::Capability::Type type ) const;
            virtual Meta::Capability* createCapabilityInterface( Meta::Capability::Type type );

            class Private;

        private:
            Private * const d;
    };
}

#endif
