/*
   Copyright (C) 2007 Maximilian Kossick <maximilian.kossick@googlemail.com>
   Copyright (C) 2008 Peter ZHOU         <peterzhoulei@gmail.com>
   Copyright (C) 2008 Seb Ruiz           <ruiz@kde.org>

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

#ifndef AMAROK_META_FILE_P_H
#define AMAROK_META_FILE_P_H

#include "Debug.h"
#include "Meta.h"
#include "MetaUtility.h"
#include "MetaReplayGain.h"

#include <QFile>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QTextCodec>

#include <KEncodingProber>
#include <KLocale>

// Taglib Includes
#include <fileref.h>
#include <tag.h>
#include <flacfile.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <mpcfile.h>
#include <mpegfile.h>
#include <oggfile.h>
#include <oggflacfile.h>
#include <tlist.h>
#include <tstring.h>
#include <vorbisfile.h>

#ifdef TAGLIB_EXTRAS_FOUND
#include <mp4file.h>
#endif

namespace MetaFile
{

//d-pointer implementation

struct MetaData
{
    MetaData()
        : discNumber( 0 )
        , trackNumber( 0 )
        , length( 0 )
        , fileSize( 0 )
        , sampleRate( 0 )
        , bitRate( 0 )
        , year( 0 )
        , trackGain( 0.0 )
        , trackPeak( 0.0 )
        , albumGain( 0.0 )
        , albumPeak( 0.0 )
    { }
    QString title;
    QString artist;
    QString album;
    QString comment;
    QString composer;
    QString genre;
    int discNumber;
    int trackNumber;
    int length;
    int fileSize;
    int sampleRate;
    int bitRate;
    int year;
    qreal trackGain;
    qreal trackPeak;
    qreal albumGain;
    qreal albumPeak;

};

class Track::Private : public QObject
{
public:
    Private( Track *t )
        : QObject()
        , url()
        , batchUpdate( false )
        , album()
        , artist()
        , score(0)
        , rating(0)
        , lastPlayed(0)
        , firstPlayed(0)
        , playCount(0)
        , track( t )
    {}

public:
    KUrl url;
    bool batchUpdate;
    Meta::AlbumPtr album;
    Meta::ArtistPtr artist;
    Meta::GenrePtr genre;
    Meta::ComposerPtr composer;
    Meta::YearPtr year;

    void readMetaData();
    QVariantMap changes;
    void writeMetaData() { DEBUG_BLOCK Meta::Field::writeFields( getFileRef(), changes ); changes.clear(); readMetaData(); }
    MetaData m_data;

    int score;
    int rating;
    uint lastPlayed;
    uint firstPlayed;
    int playCount;

private:
    TagLib::FileRef getFileRef();
    Track *track;
};

TagLib::FileRef
Track::Private::getFileRef()
{
#ifdef COMPLEX_TAGLIB_FILENAME
    const wchar_t * encodedName = reinterpret_cast<const wchar_t *>(url.path().utf16());
#else
    QByteArray fileName = QFile::encodeName( url.path() );
    const char * encodedName = fileName.constData(); // valid as long as fileName exists
#endif
    return TagLib::FileRef( encodedName, true, TagLib::AudioProperties::Fast );
}

void Track::Private::readMetaData()
{
#define strip( x ) TStringToQString( x ).trimmed()
    TagLib::FileRef fileRef = getFileRef();

    TagLib::Tag *tag = 0;
    if( !fileRef.isNull() )
        tag = fileRef.tag();

    if( tag )
    {
        m_data.title = strip( tag->title() );
        m_data.artist = strip( tag->artist() );
        m_data.album = strip( tag->album() );
        m_data.comment = strip( tag->comment() );
        m_data.genre = strip( tag->genre() );
        m_data.trackNumber = tag->track();
        m_data.year = tag->year();
    }
    if( !fileRef.isNull() )
    {
        if( fileRef.audioProperties() )
        {
            m_data.bitRate = fileRef.audioProperties()->bitrate();
            m_data.sampleRate = fileRef.audioProperties()->sampleRate();
            m_data.length = fileRef.audioProperties()->length();
        }

        Meta::ReplayGainTagMap map = Meta::readReplayGainTags( fileRef );
        if ( map.contains( Meta::ReplayGain_Track_Gain ) )
            m_data.trackGain = map[Meta::ReplayGain_Track_Gain];
        if ( map.contains( Meta::ReplayGain_Track_Peak ) )
            m_data.trackPeak = map[Meta::ReplayGain_Track_Peak];
        if ( map.contains( Meta::ReplayGain_Album_Gain ) )
            m_data.albumGain = map[Meta::ReplayGain_Album_Gain];
        else
            m_data.albumGain = m_data.trackGain;
        if ( map.contains( Meta::ReplayGain_Album_Peak ) )
            m_data.albumPeak = map[Meta::ReplayGain_Album_Peak];
        else
            m_data.albumPeak = m_data.trackPeak;
    }
    //This is pretty messy...
    QString disc;

    if( TagLib::MPEG::File *file = dynamic_cast<TagLib::MPEG::File *>( fileRef.file() ) )
    {
        if( file->ID3v2Tag() )
        {
            const TagLib::ID3v2::FrameListMap flm = file->ID3v2Tag()->frameListMap();
            if( !flm[ "TPOS" ].isEmpty() )
                disc = strip( flm[ "TPOS" ].front()->toString() );

            if( !flm[ "TCOM" ].isEmpty() )
                m_data.composer = strip( flm[ "TCOM" ].front()->toString() );

            if( !flm[ "TPE2" ].isEmpty() )
                m_data.artist = strip( flm[ "TPE2" ].front()->toString() );

        }
        if( tag )
        {
            TagLib::String metaData = tag->title() + tag->artist() + tag->album() + tag->comment();
            const char* buf = metaData.toCString();
            size_t len = strlen( buf );
            KEncodingProber prober;
            KEncodingProber::ProberState result = prober.feed( buf, len );
            QString track_encoding( prober.encodingName() );
            if ( result != KEncodingProber::NotMe )
            {
                /*  for further infomation please refer to:
                    http://doc.trolltech.com/4.4/qtextcodec.html
                    http://www.mozilla.org/projects/intl/chardet.html
                */
                if ( ( !track_encoding.isEmpty() ) && ( track_encoding.toUtf8() != "UTF-8" ) )
                {
                    debug () << "Final Codec Name:" << track_encoding.toUtf8();
                    QTextCodec *codec = QTextCodec::codecForName( track_encoding.toUtf8() );
                    QTextCodec* utf8codec = QTextCodec::codecForName( "UTF-8" );
                    QTextCodec::setCodecForCStrings( utf8codec );
                    if ( codec != 0 )
                    {
                        m_data.title = codec->toUnicode( m_data.title.toLatin1() );
                        m_data.artist = codec->toUnicode( m_data.artist.toLatin1() );
                        m_data.album = codec->toUnicode( m_data.album.toLatin1() );
                        m_data.comment = codec->toUnicode( m_data.comment.toLatin1() );
                        debug() << "track Info Decoded!";
                    }
                }
            }
        }
    }

    else if( TagLib::Ogg::Vorbis::File *file = dynamic_cast< TagLib::Ogg::Vorbis::File *>( fileRef.file() ) )
    {
        if( file->tag() )
        {
            const TagLib::Ogg::FieldListMap flm = file->tag()->fieldListMap();
            if( !flm[ "COMPOSER" ].isEmpty() )
                m_data.composer = strip( flm[ "COMPOSER" ].front() );
            if( !flm[ "DISCNUMBER" ].isEmpty() )
                disc = strip( flm[ "DISCNUMBER" ].front() );
        }
    }

    else if( TagLib::FLAC::File *file = dynamic_cast< TagLib::FLAC::File *>( fileRef.file() ) )
    {
        if( file->xiphComment() )
        {
            const TagLib::Ogg::FieldListMap flm = file->xiphComment()->fieldListMap();
            if( !flm[ "COMPOSER" ].isEmpty() )
                m_data.composer = strip( flm[ "COMPOSER" ].front() );
            if( !flm[ "DISCNUMBER" ].isEmpty() )
                disc = strip( flm[ "DISCNUMBER" ].front() );
        }
    }
#ifdef TAGLIB_EXTRAS_FOUND
    else if( TagLib::MP4::File *file = dynamic_cast<TagLib::MP4::File *>( fileRef.file() ) )
    {
        TagLib::MP4::Tag *mp4tag = dynamic_cast< TagLib::MP4::Tag *>( file->tag() );
        if( mp4tag )
        {
            if ( !mp4tag->itemListMap()["\xA9wrt"].toStringList().isEmpty() )
                m_data.composer = strip( mp4tag->itemListMap()["\xA9wrt"].toStringList().front() );

            if ( !mp4tag->itemListMap()["disk"].toStringList().isEmpty() )
                disc = QString::number( mp4tag->itemListMap()["disk"].toIntPair().first );
        }
    }
#endif
    if( !disc.isEmpty() )
    {
        int i = disc.indexOf( '/' );
        if( i != -1 )
            m_data.discNumber = disc.left( i ).toInt();
        else
            m_data.discNumber = disc.toInt();
    }
#undef strip
    m_data.fileSize = QFile( url.path() ).size();

    debug() << "Read metadata from file for: " + m_data.title;
}

// internal helper classes

class FileArtist : public Meta::Artist
{
public:
    FileArtist( MetaFile::Track::Private *dptr )
        : Meta::Artist()
        , d( dptr )
    {}

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    Meta::AlbumList albums()
    {
        return Meta::AlbumList();
    }

    QString name() const
    {
        const QString artist = d->m_data.artist;
        if( !artist.isEmpty() )
            return artist;
        return i18nc( "The value is not known", "Unknown" );
    }

    QString prettyName() const
    {
        return name();
    }

    QPointer<MetaFile::Track::Private> const d;
};

class FileAlbum : public Meta::Album
{
public:
    FileAlbum( MetaFile::Track::Private *dptr )
        : Meta::Album()
        , d( dptr )
    {}

    bool isCompilation() const
    {
        return false;
    }

    bool hasAlbumArtist() const
    {
        return false;
    }

    Meta::ArtistPtr albumArtist() const
    {
        return Meta::ArtistPtr();
    }

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    QString name() const
    {
        if( d )
        {
            const QString albumName = d->m_data.album;
            if( !albumName.isEmpty() )
                return albumName;
            else
                return i18nc( "The value is not known", "Unknown" );
        }
        else
            return i18nc( "The value is not known", "Unknown" );
    }

    QString prettyName() const
    {
        return name();
    }

    QPixmap image( int size )
    {
        return Meta::Album::image( size );
    }

    bool operator==( const Meta::Album &other ) const {
        return name() == other.name();
    }

    QPointer<MetaFile::Track::Private> const d;
};

class FileGenre : public Meta::Genre
{
public:
    FileGenre( MetaFile::Track::Private *dptr )
        : Meta::Genre()
        , d( dptr )
    {}

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    QString name() const
    {
        const QString genreName = d->m_data.genre;
        if( !genreName.isEmpty() )
            return genreName;
        return i18nc( "The value is not known", "Unknown" );
    }

    QString prettyName() const
    {
        return name();
    }

    QPointer<MetaFile::Track::Private> const d;
};

class FileComposer : public Meta::Composer
{
public:
    FileComposer( MetaFile::Track::Private *dptr )
        : Meta::Composer()
        , d( dptr )
    {}

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    QString name() const
    {
        const QString composer = d->m_data.composer;
        if( !composer.isEmpty() )
            return composer;
        return i18nc( "The value is not known", "Unknown" );
    }

    QString prettyName() const
    {
        return name();
    }

    QPointer<MetaFile::Track::Private> const d;
};

class FileYear : public Meta::Year
{
public:
    FileYear( MetaFile::Track::Private *dptr )
        : Meta::Year()
        , d( dptr )
    {}

    Meta::TrackList tracks()
    {
        return Meta::TrackList();
    }

    QString name() const
    {
        const QString year = QString::number( d->m_data.year );
        if( !year.isEmpty()  )
            return year;
        return i18nc( "The value is not known", "Unknown" );
    }

    QString prettyName() const
    {
        return name();
    }

    QPointer<MetaFile::Track::Private> const d;
};


}

#endif
