/***************************************************************************
 * Ported to Collection Framework: *
 * copyright            : (C) 2008 Alejandro Wainzinger <aikawarazuni@gmail.com>

 * Original Work: *
 * copyright            : (C) 2006 Andy Kelk <andy@mopoke.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#define DEBUG_PREFIX "MtpHandler"

#include "MtpHandler.h"

#include "MtpCollection.h"
#include "MtpMeta.h"
#include "../../../statusbar/StatusBar.h"
#include "Debug.h"

#include "File.h" // for KIO file handling

#include <KUniqueApplication> // needed for KIO processes
#include <KIO/Job>
#include <KIO/DeleteJob>
#include "kjob.h"
#include <threadweaver/ThreadWeaver.h>
#include <KUrl>

#include <QString>
#include <QStringList>
#include <QTextStream>

using namespace Mtp;
using namespace Meta;

MtpHandler::MtpHandler( MtpCollection *mc, QObject *parent )
        : QObject( parent )
        , m_memColl( mc )
        , m_statusbar( 0 )
        , m_device( 0 )
        , m_default_parent_folder( 0 )
        , m_folders( 0 )
        , m_folderStructure()
        , m_format()
        , m_name()
        , m_success( false )
        , m_trackCreated( false )
        , m_copyFailed( false )
        , m_isCanceled( false )
        , m_wait( false )
{
    DEBUG_BLOCK
}

MtpHandler::~MtpHandler()
{
    // TODO: free used memory
    terminate();
}

void
MtpHandler::init( const QString &serial )
{
    mtpFileTypes[LIBMTP_FILETYPE_WAV] = "wav";
    mtpFileTypes[LIBMTP_FILETYPE_MP3] = "mp3";
    mtpFileTypes[LIBMTP_FILETYPE_WMA] = "wma";
    mtpFileTypes[LIBMTP_FILETYPE_OGG] = "ogg";
    mtpFileTypes[LIBMTP_FILETYPE_AUDIBLE] = "aa"; // audible
    mtpFileTypes[LIBMTP_FILETYPE_MP4] = "mp4";
    mtpFileTypes[LIBMTP_FILETYPE_UNDEF_AUDIO] = "undef-audio";
    mtpFileTypes[LIBMTP_FILETYPE_WMV] = "wmv";
    mtpFileTypes[LIBMTP_FILETYPE_AVI] = "avi";
    mtpFileTypes[LIBMTP_FILETYPE_MPEG] = "mpg";
    mtpFileTypes[LIBMTP_FILETYPE_ASF] = "asf";
    mtpFileTypes[LIBMTP_FILETYPE_QT] = "mov";
    mtpFileTypes[LIBMTP_FILETYPE_UNDEF_VIDEO] = "undef-video";
    mtpFileTypes[LIBMTP_FILETYPE_JPEG] = "jpg";
    mtpFileTypes[LIBMTP_FILETYPE_JFIF] = "jpg";
    mtpFileTypes[LIBMTP_FILETYPE_TIFF] = "tiff";
    mtpFileTypes[LIBMTP_FILETYPE_BMP] = "bmp";
    mtpFileTypes[LIBMTP_FILETYPE_GIF] = "gif";
    mtpFileTypes[LIBMTP_FILETYPE_PICT] = "pict";
    mtpFileTypes[LIBMTP_FILETYPE_PNG] = "png";
    mtpFileTypes[LIBMTP_FILETYPE_VCALENDAR1] = "vcs"; // vcal1
    mtpFileTypes[LIBMTP_FILETYPE_VCALENDAR2] = "vcs"; // vcal2
    mtpFileTypes[LIBMTP_FILETYPE_VCARD2] = "vcf"; // vcard2
    mtpFileTypes[LIBMTP_FILETYPE_VCARD3] = "vcf"; // vcard3
    mtpFileTypes[LIBMTP_FILETYPE_WINDOWSIMAGEFORMAT] = "wim"; // windows image format
    mtpFileTypes[LIBMTP_FILETYPE_WINEXEC] = "exe";
    mtpFileTypes[LIBMTP_FILETYPE_TEXT] = "txt";
    mtpFileTypes[LIBMTP_FILETYPE_HTML] = "html";
    mtpFileTypes[LIBMTP_FILETYPE_UNKNOWN] = "unknown";

    QString genericError = i18n( "Could not connect to MTP Device" );

    m_success = false;

    // begin checking connected devices

    LIBMTP_raw_device_t * rawdevices;
    int numrawdevices;
    LIBMTP_error_number_t err;

    debug() << "Initializing MTP stuff";
    LIBMTP_Init();

    // get list of raw devices
    debug() << "Getting list of raw devices";
    err = LIBMTP_Detect_Raw_Devices( &rawdevices, &numrawdevices );

    debug() << "Error is: " << err;

    switch ( err )
    {
    case LIBMTP_ERROR_NO_DEVICE_ATTACHED:
        fprintf( stdout, "   No raw devices found.\n" );
        m_success = false;
        break;

    case LIBMTP_ERROR_CONNECTING:
        fprintf( stderr, "Detect: There has been an error connecting. Exiting\n" );
        m_success = false;
        break;

    case LIBMTP_ERROR_MEMORY_ALLOCATION:
        fprintf( stderr, "Detect: Encountered a Memory Allocation Error. Exiting\n" );
        m_success = false;
        break;

    case LIBMTP_ERROR_NONE:
    {
        m_success = true;
        break;
    }

    default:
        debug() << "Unhandled mtp error";
        m_success = false;
        break;
    }



    if ( m_success )
    {
        debug() << "Got mtp list, connecting to device using thread";
        ThreadWeaver::Weaver::instance()->enqueue( new WorkerThread( numrawdevices, rawdevices, serial, this ) );
    }
    else
    {
        free( rawdevices );
        emit failed();
    }

}


// this function is threaded
bool
MtpHandler::iterateRawDevices( int numrawdevices, LIBMTP_raw_device_t* rawdevices, const QString &serial )
{
    DEBUG_BLOCK

    bool success = false;

    LIBMTP_mtpdevice_t *device = 0;
    // test raw device for connectability
    for ( int i = 0; i < numrawdevices; i++ )
    {

        debug() << "Opening raw device number: " << ( i + 1 );
        device = LIBMTP_Open_Raw_Device( &rawdevices[i] );
        if ( device == NULL )
        {
            debug() << "Unable to open raw device: " << ( i + 1 );
            success = false;
            continue;
        }

        debug() << "Testing serial number";

        // HACK: not checking serial to confirm the right device is in place
        // this is not incorrect, and long-term goal is to remove serial number from use altogether

        /*
        QString mtpSerial = QString::fromUtf8( LIBMTP_Get_Serialnumber( device ) );
        if( !mtpSerial.contains(serial) )
        {
            debug() << "Wrong device, going to next";
            debug() << "Expected: " << serial << " but got: " << mtpSerial;
            success = false;
            LIBMTP_Release_Device( device );
            continue;
        }
        */

        debug() << "Correct device found";
        success = true;
        break;
    }

    m_device = device;

    if ( m_device == 0 )
    {
        // TODO: error protection
        success = false;
        free( rawdevices );

    }

    //QString serial = QString::fromUtf8( LIBMTP_Get_Serialnumber( m_device ) );

    debug() << "Serial is: " << serial;

    debug() << "Success is: " << ( success ? "true" : "false" );

    return success;
}

void
MtpHandler::getDeviceInfo()
{
    // Get information for device

    // Get Battery level and print to debug

    unsigned char max;
    unsigned char curr;
    int failed;

    failed = LIBMTP_Get_Batterylevel( m_device, &max, &curr );

    if ( !failed )
        debug() << "Battery at: " << curr << "/" << max;
    else
        debug() << "Unknown battery level";



    QString modelname = QString( LIBMTP_Get_Modelname( m_device ) );

    // NOTE: on next libmtp bump, may reintroduce owner name
    // for now it doesn't work as expected
    /*
    QString ownername = QString( LIBMTP_Get_Friendlyname( m_device ) );
    m_name = modelname;
    if(! ownername.isEmpty() )
        if( modelname != ownername )
            m_name += " (" + ownername + ')';

    else
        m_name += " (No Owner Name)";

    */

    m_name = modelname;

    m_default_parent_folder = m_device->default_music_folder;
    debug() << "setting default parent : " << m_default_parent_folder;


    m_folders = LIBMTP_Get_Folder_List( m_device );
    uint16_t *filetypes;
    uint16_t filetypes_len;
    int ret = LIBMTP_Get_Supported_Filetypes( m_device, &filetypes, &filetypes_len );
    if ( ret == 0 )
    {
        uint16_t i;
        for ( i = 0; i < filetypes_len; ++i )
        {
            debug() << "Device supports: " << mtpFileTypes[ filetypes[ i ] ];
            m_supportedFiles << mtpFileTypes[ filetypes[ i ] ];
        }
    }
    // find supported image types (for album art).
    if ( m_supportedFiles.indexOf( "jpg" ) )
        m_format = "JPEG";
    else if ( m_supportedFiles.indexOf( "png" ) )
        m_format = "PNG";
    else if ( m_supportedFiles.indexOf( "gif" ) )
        m_format = "GIF";
    free( filetypes );
}

void
MtpHandler::terminate()
{
    DEBUG_BLOCK
    // clear folder structure
    if ( m_folders != 0 )
    {

        LIBMTP_destroy_folder_t( m_folders );

        m_folders = 0;
        debug() << "Folders destroyed";
    }

    // Delete temporary files

    TrackMap trackMap = m_memColl->trackMap();

    foreach( Meta::TrackPtr track,  trackMap.values() )
    {
        Meta::MtpTrackPtr mtptrack = Meta::MtpTrackPtr::staticCast( track );
        mtptrack->deleteTempFile();
    }

    // release device
    if ( m_device != 0 )
    {

        LIBMTP_Release_Device( m_device );
        /* possible race condition with statusbar destructor,
        will uncomment when fixed */
        //The::statusBar()->longMessage(
//                       i18n( "The MTP device %1 has been disconnected", prettyName() ), StatusBar::Information );
        debug() << "Device released";
    }
}



/**
 * Check (and optionally create) the folder structure to put a
 * track into. Return the (possibly new) parent folder ID
 */
uint32_t
MtpHandler::checkFolderStructure( const Meta::TrackPtr track, bool create )
{
    QString artistName;
    Meta::ArtistPtr artist = track->artist();
    if ( !artist || artist->prettyName().isEmpty() )
        artistName = i18n( "Unknown Artist" );
    else
        artistName = artist->prettyName();
    //FIXME: Port
//     if( bundle.compilation() == MetaBundle::CompilationYes )
//         artist = i18n( "Various Artists" );
    QString albumName;
    Meta::AlbumPtr album = track->album();
    if ( !album || album->prettyName().isEmpty() )
        albumName = i18n( "Unknown Album" );
    else
        albumName = album->prettyName();
    QString genreName;
    Meta::GenrePtr genre = track->genre();
    if ( !genre || genre->prettyName().isEmpty() )
        genreName = i18n( "Unknown Genre" );
    else
        genreName = genre->prettyName();

    uint32_t parent_id = getDefaultParentId();
    QStringList folders = m_folderStructure.split( '/' ); // use slash as a dir separator
    QString completePath;
    for ( QStringList::Iterator it = folders.begin(); it != folders.end(); ++it )
    {
        if (( *it ).isEmpty() )
            continue;
        // substitute %a , %b , %g
        ( *it ).replace( QRegExp( "%a" ), artistName )
        .replace( QRegExp( "%b" ), albumName )
        .replace( QRegExp( "%g" ), genreName );
        // check if it exists
        uint32_t check_folder = subfolderNameToID(( *it ).toUtf8(), m_folders, parent_id );
        // create if not exists (if requested)
        if ( check_folder == 0 )
        {
            if ( create )
            {
                check_folder = createFolder(( *it ).toUtf8() , parent_id );
                if ( check_folder == 0 )
                {
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }
        completePath += ( *it ).toUtf8() + '/';
        // set new parent
        parent_id = check_folder;
    }
    debug() << "Folder path : " << completePath;
    // return parent
    return parent_id;
}

uint32_t
MtpHandler::getDefaultParentId( void )
{
    // Decide which folder to send it to:
    // If the device gave us a parent_folder setting, we use it
    uint32_t parent_id = 0;
    if ( m_default_parent_folder )
    {
        parent_id = m_default_parent_folder;
    }
    // Otherwise look for a folder called "Music"
    else if ( m_folders != 0 )
    {
        parent_id = folderNameToID( qstrdup( QString( "Music" ).toUtf8() ), m_folders );
        if ( !parent_id )
        {
            debug() << "Parent folder could not be found. Going to use top level.";
        }
    }
    // Give up and don't set a parent folder, let the device deal with it
    else
    {
        debug() << "No folders found. Going to use top level.";
    }
    return parent_id;
}

/**
 * Recursively search the folder list for a matching one
 * and return its ID
 */
uint32_t
MtpHandler::folderNameToID( char *name, LIBMTP_folder_t *folderlist )
{
    uint32_t i;

    if ( folderlist == 0 )
        return 0;

    if ( !strcasecmp( name, folderlist->name ) )
        return folderlist->folder_id;

    if (( i = ( folderNameToID( name, folderlist->child ) ) ) )
        return i;
    if (( i = ( folderNameToID( name, folderlist->sibling ) ) ) )
        return i;

    return 0;
}

/**
 * Recursively search the folder list for a matching one under the specified
 * parent ID and return the child's ID
 */
uint32_t
MtpHandler::subfolderNameToID( const char *name, LIBMTP_folder_t *folderlist, uint32_t parent_id )
{
    uint32_t i;

    if ( folderlist == 0 )
        return 0;

    if ( !strcasecmp( name, folderlist->name ) && folderlist->parent_id == parent_id )
        return folderlist->folder_id;

    if (( i = ( subfolderNameToID( name, folderlist->child, parent_id ) ) ) )
        return i;
    if (( i = ( subfolderNameToID( name, folderlist->sibling, parent_id ) ) ) )
        return i;

    return 0;
}

/**
 * Create a new mtp folder
 */
uint32_t
MtpHandler::createFolder( const char *name, uint32_t parent_id )
{
    debug() << "Creating new folder '" << name << "' as a child of " << parent_id;
    char *name_copy = qstrdup( name );
    // NOTE: api change, 0 refers to default storage_id
    uint32_t new_folder_id = LIBMTP_Create_Folder( m_device, name_copy, parent_id, 0 );
    delete( name_copy );
    debug() << "New folder ID: " << new_folder_id;
    if ( new_folder_id == 0 )
    {
        debug() << "Attempt to create folder '" << name << "' failed.";
        return 0;
    }
    updateFolders();

    return new_folder_id;
}

/**
 * Update local cache of mtp folders
 */
void
MtpHandler::updateFolders( void )
{
    LIBMTP_destroy_folder_t( m_folders );
    m_folders = 0;
    m_folders = LIBMTP_Get_Folder_List( m_device );
}

void
MtpHandler::copyTrackListToDevice( const Meta::TrackList tracklist )
{
    DEBUG_BLOCK

    bool isDupe;
    bool hasDupe;
    TrackMap trackMap = m_memColl->trackMap();

    Meta::TrackList tempTrackList;

    m_copyFailed = false;

    hasDupe = false;

    m_tracksFailed.clear();

    m_lastTrackCopied = 0;

    /* Clear Transfer queue */

    m_tracksToCopy.clear();

    /* Check for same tags, don't copy if same tags */

    foreach( Meta::TrackPtr track, tracklist )
    {
        tempTrackList = m_titlemap.values( track->name() );

        /* If no song with same title, already not a dupe */

        if ( tempTrackList.isEmpty() )
        {
            debug() << "No tracks with same title, track not a dupe";
            m_tracksToCopy.append( track );
            continue;
        }

        /* Songs with same title present, check other tags */

        debug() << "Same title present, checking other tags";

        isDupe = false;

        foreach( Meta::TrackPtr tempTrack, tempTrackList )
        {

            if (( tempTrack->artist()->name() != track->artist()->name() )
                    || ( tempTrack->album()->name() != track->album()->name() )
                    || ( tempTrack->genre()->name() != track->genre()->name() )
                    || ( tempTrack->composer()->name() != track->composer()->name() )
                    || ( tempTrack->year()->name() != track->year()->name() ) )
            {
                debug() << "Same title, but other tags differ, not a dupe";
                continue;
            }

            /* Track is already on there, break */

            isDupe = true;
            hasDupe = true;
            break;
        }

        if ( !isDupe )
            m_tracksToCopy.append( track );
        else
        {
            debug() << "Track " << track->name() << " is a dupe!";

            QString error = "Already on device";
            m_tracksFailed.insert( track, error );
        }
    }

    if ( hasDupe )
        m_copyFailed = true;

    /* List ready, begin copying */

    copyTracksToDevice();

}

void
MtpHandler::copyTracksToDevice()
{
    DEBUG_BLOCK

    // Do not bother copying 0 tracks
    // This could happen if all tracks to copy are dupes

    if ( m_tracksToCopy.size() == 0 )
    {
        emit copyTracksDone( false );
        return;
    }
    debug() << "Copying " << m_tracksToCopy.size() << " tracks";

    m_statusbar = The::statusBar()->newProgressOperation( this, i18n( "Transferring Tracks to MTP" ) );

    m_statusbar->setMaximum( m_tracksToCopy.size() );
    m_statusbar->setValue( 0 );

    connect( this, SIGNAL( incrementProgress() ),
             The::statusBar(), SLOT( incrementProgress() ) );
    connect( this, SIGNAL( endProgressOperation( const QObject* ) ),
             The::statusBar(), SLOT( endProgressOperation( const QObject* ) ) );

    copyNextTrackToDevice();

}

void
MtpHandler::copyNextTrackToDevice()
{
    Meta::TrackPtr track;

    // If there are more tracks to copy, copy the next one
    if ( !m_tracksToCopy.isEmpty() )
    {
        // Pop the track off the front of the list

        track = m_tracksToCopy.first();
        m_tracksToCopy.removeFirst();

        // Copy the track

        m_lastTrackCopied = track;

        ThreadWeaver::Weaver::instance()->enqueue( new CopyWorkerThread( track, this ) );

    }

    // No tracks left to copy, emit done
    else
    {
        emit incrementProgress();
        emit endProgressOperation( this );
        emit copyTracksDone( !m_copyFailed );
    }

}

bool
MtpHandler::privateCopyTrackToDevice( const Meta::TrackPtr &track )
{
    DEBUG_BLOCK

    QString genericError = i18n( "Could not send track" );

    LIBMTP_track_t *trackmeta = LIBMTP_new_track_t();
    trackmeta->item_id = 0;
    debug() << "filetype : " << track->type();
    if ( track->type() == "mp3" )
    {
        trackmeta->filetype = LIBMTP_FILETYPE_MP3;
    }
    else if ( track->type() == "ogg" )
    {
        trackmeta->filetype = LIBMTP_FILETYPE_OGG;
    }
    else if ( track->type() == "wma" )
    {
        trackmeta->filetype = LIBMTP_FILETYPE_WMA;
    }
    else if ( track->type() == "mp4" )
    {
        trackmeta->filetype = LIBMTP_FILETYPE_MP4;
    }
    else
    {
        // Couldn't recognise an Amarok filetype.
        // fallback to checking the extension (e.g. .wma, .ogg, etc)
        debug() << "No filetype found by Amarok filetype";

        const QString extension = track->type().toLower();

        int libmtp_type = m_supportedFiles.indexOf( extension );
        if ( libmtp_type >= 0 )
        {
            int keyIndex = mtpFileTypes.values().indexOf( extension );
            libmtp_type = mtpFileTypes.keys()[keyIndex];
            trackmeta->filetype = ( LIBMTP_filetype_t ) libmtp_type;
            debug() << "set filetype to " << libmtp_type << " based on extension of ." << extension;
        }
        else
        {
            debug() << "We do not support the extension ." << extension;
            /*   The::statusBar()->shortLongMessage(
               genericError,
               i18n( "Cannot determine a valid file type" ),
               StatusBar::Error
                                                 );*/
            return false;
        }
    }

    if ( track->prettyName().isEmpty() )
    {
        trackmeta->title = qstrdup( i18n( "Unknown title" ).toUtf8() );
    }
    else
    {
        trackmeta->title = qstrdup( track->prettyName().toUtf8() );
    }

    if ( !track->album() )
    {
        trackmeta->album = qstrdup( i18n( "Unknown album" ).toUtf8() );
    }
    else
    {
        trackmeta->album = qstrdup( track->album()->prettyName().toUtf8() );
    }

    if ( !track->artist() )
    {
        trackmeta->artist = qstrdup( i18n( "Unknown artist" ).toUtf8() );
    }
    else
    {
        trackmeta->artist = qstrdup( track->artist()->prettyName().toUtf8() );
    }

    if ( !track->genre() )
    {
        trackmeta->genre = qstrdup( i18n( "Unknown genre" ).toUtf8() );
    }
    else
    {
        trackmeta->genre = qstrdup( track->genre()->prettyName().toUtf8() );
    }

    // TODO: port to Qt4

    debug() << "Source track year: " << track->year();

    if ( track->year() > 0 )
    {
        QString date;
        QTextStream dateout( &date );
        dateout << track->year()->name() << "0101T0000.0";
//        debug() << "Proposed date: " << dateout.string();
        trackmeta->date = qstrdup( dateout.string()->toUtf8() );
//        debug() << "Set date to: " << trackmeta->date;
    }

    else
    {
        trackmeta->date = qstrdup( "00010101T0000.0" );
    }

    if ( track->trackNumber() > 0 )
    {
        trackmeta->tracknumber = track->trackNumber();
    }
    debug() << "Has track length" << ( track->length() > 0 ? "true" : "false" );
    if ( track->length() > 0 )
    {
        debug() << "Setting tracklength to: " << track->length() << "s";
        // Multiply by 1000 since this is in milliseconds
        trackmeta->duration = ( track->length() * 1000 );
        debug() << "Track length is now: " << trackmeta->duration << "ms";
    }
    if ( !track->playableUrl().fileName().isEmpty() )
    {
        trackmeta->filename = qstrdup( track->playableUrl().fileName().toUtf8() );
    }
    trackmeta->filesize = track->filesize();

    debug() << "Filesize before copy: " << trackmeta->filesize;

    // try and create the requested folder structure
    uint32_t parent_id = 0;
    if ( !m_folderStructure.isEmpty() )
    {
        parent_id = checkFolderStructure( track, true ); // true means create
        if ( parent_id == 0 )
        {
            debug() << "Could not create new parent (" << m_folderStructure << ")";
            /*The::statusBar()->shortLongMessage(
            genericError,
            i18n( "Cannot create parent folder. Check your structure." ),
            StatusBar::Error
                                              );*/
            return false;
        }
    }
    else
    {
        parent_id = getDefaultParentId();
    }
    debug() << "Parent id : " << parent_id;

    trackmeta->parent_id = parent_id; // api change, set id here
    trackmeta->storage_id = 0; // default storage id
    debug() << "Sending track... " << track->playableUrl();
    debug() << "Filename is: " << QString::fromUtf8( trackmeta->filename );




    // TODO: implement callback correctly, not 0

    debug() << "Playable Url is: " << track->playableUrl();
    debug() << "Sending file with path: " << track->playableUrl().path().toUtf8();

    int ret = LIBMTP_Send_Track_From_File( m_device, qstrdup( track->playableUrl().path().toUtf8() ), trackmeta,
                                           0, this );

    debug() << "Filesize after copy: " << trackmeta->filesize;



    if ( ret < 0 )
    {
        debug() << "Could not write file " << ret;
        /*The::statusBar()->shortLongMessage(
        genericError,
        i18n( "File write failed" ),
        StatusBar::Error
                                          );*/
        return false;
    }
// TODO: cleanup



    addMtpTrackToCollection( trackmeta );



    //LIBMTP_destroy_track_t( trackmeta );

    return true;
}

void
MtpHandler::deleteTracksFromDevice( const Meta::TrackList &tracks )
{
    DEBUG_BLOCK

    // Init the list of tracks to be deleted

    m_tracksToDelete = tracks;

    // Set up statusbar for deletion operation

    m_statusbar = The::statusBar()->newProgressOperation( this, i18n( "Deleting Tracks from MTP" ) );

    m_statusbar->setMaximum( tracks.size() );

    connect( this, SIGNAL( incrementProgress() ),
             The::statusBar(), SLOT( incrementProgress() ) );
    connect( this, SIGNAL( endProgressOperation( const QObject* ) ),
             The::statusBar(), SLOT( endProgressOperation( const QObject* ) ) );

    while ( !m_tracksToDelete.isEmpty() )
        deleteNextTrackFromDevice();

    emit incrementProgress();
    emit deleteTracksDone();

}

void
MtpHandler::deleteNextTrackFromDevice()
{

    Meta::TrackPtr track;

    // Pop the track off the front of the list

    track = m_tracksToDelete.first();
    m_tracksToDelete.removeFirst();

    // Delete the track

    privateDeleteTrackFromDevice( Meta::MtpTrackPtr::staticCast( track ) );

    emit incrementProgress();

}

void
MtpHandler::privateDeleteTrackFromDevice( const Meta::MtpTrackPtr &track )
{
    DEBUG_BLOCK

    //If nothing is left in a folder, delete the folder
    u_int32_t object_id = track->id();

    QString genericError = i18n( "Could not delete item" );

    debug() << "delete this id : " << object_id;


    int status = LIBMTP_Delete_Object( m_device, object_id );

    if ( status != 0 )
    {
        debug() << "delete object failed";
        The::statusBar()->longMessage(
            i18n( "Delete failed" ),
            StatusBar::Error
        );
//       return false;
    }
    debug() << "object deleted";

//   return true;

    m_titlemap.remove( track->name(), Meta::TrackPtr::staticCast( track ) );

}

int
MtpHandler::getTrackToFile( const uint32_t id, const QString & filename )
{
    DEBUG_BLOCK

//    The::statusBar()->shortMessage( i18n( "Loading Track" ) );
    /*
        connect( this, SIGNAL( setProgress( int ) ),
                 The::statusBar(), SLOT( setProgress( int ) ) );

        connect( this, SIGNAL( endProgressOperation( const QObject*) ),
                 The::statusBar(), SLOT( endProgressOperation( const QObject* ) ) );
                */

    return LIBMTP_Get_Track_To_File( m_device, id, filename.toUtf8(), 0, 0 );
}

int
MtpHandler::progressCallback( uint64_t const sent, uint64_t const total, void const * const data )
{
    DEBUG_BLOCK
    MtpHandler *handler = ( MtpHandler* )( data );

    // NOTE: setting max many times wastes cycles,
    // but how else to get total outside of callback?

    debug() << "Setting max to: " << (( int ) total );

    debug() << "Device: " << handler->prettyName();

    handler->setBarMaximum(( int ) total );
    handler->setBarProgress(( int ) sent );

    if ( sent == total )
        handler->endBarProgressOperation();

    return 0;
}

void
MtpHandler::setBarMaximum( int total )
{
    DEBUG_BLOCK
    m_statusbar->setMaximum( total );
}
void
MtpHandler::setBarProgress( int steps )
{
    DEBUG_BLOCK
    emit setProgress( steps );
}

void
MtpHandler::endBarProgressOperation()
{
    DEBUG_BLOCK
    emit endProgressOperation( this );
}

int
MtpHandler::readMtpMusic()
{
    DEBUG_BLOCK



    return 0;
}

void
MtpHandler::getBasicMtpTrackInfo( LIBMTP_track_t *mtptrack, Meta::MtpTrackPtr track )
{
    track->setTitle( QString::fromUtf8( mtptrack->title ) );
    if ( mtptrack->duration > 0 )
        track->setLength(( mtptrack->duration ) / 1000 );
    else
        track->setLength( 0 );
    track->setTrackNumber( mtptrack->tracknumber );
    track->setComment( QString() ); // defaulting, since not provided
    track->setDiscNumber( 1 ); // defaulting, since not provided
    track->setBitrate( mtptrack->bitrate );
    track->setFileSize( mtptrack->filesize );

//    debug() << "Title is: " << track->title();

    /* set proposed temporary file path, to which track will be copied temporarily before attempting to play */

    track->setFormat( getFormat( mtptrack ) );

    // libmtp low-level function data

    track->setFolderId( mtptrack->parent_id );
    track->setId( mtptrack->item_id );

//    debug() << "Id is: " << track->id();

//    track->setPlayableUrl( "" ); // defaulting, since not provided
    track->setUrl( QString::number( track->id(), 10 ) ); // for map key
}

void
MtpHandler::setBasicMtpTrackInfo( LIBMTP_track_t *trackmeta, Meta::MtpTrackPtr track )
{
    if ( track->prettyName().isEmpty() )
    {
        trackmeta->title = qstrdup( i18n( "Unknown title" ).toUtf8() );
    }
    else
    {
        trackmeta->title = qstrdup( track->prettyName().toUtf8() );
    }

    if ( !track->album() )
    {
        trackmeta->album = qstrdup( i18n( "Unknown album" ).toUtf8() );
    }
    else
    {
        trackmeta->album = qstrdup( track->album()->prettyName().toUtf8() );
    }

    if ( !track->artist() )
    {
        trackmeta->artist = qstrdup( i18n( "Unknown artist" ).toUtf8() );
    }
    else
    {
        trackmeta->artist = qstrdup( track->artist()->prettyName().toUtf8() );
    }

    if ( !track->genre() )
    {
        trackmeta->genre = qstrdup( i18n( "Unknown genre" ).toUtf8() );
    }
    else
    {
        trackmeta->genre = qstrdup( track->genre()->prettyName().toUtf8() );
    }

    if ( track->year() > 0 )
    {
        QString date;
        QTextStream( &date ) << track->year() << "0101T0000.0";
        trackmeta->date = qstrdup( date.toUtf8() );
    }
    else
    {
        trackmeta->date = qstrdup( "00010101T0000.0" );
    }

    if ( track->trackNumber() > 0 )
    {
        trackmeta->tracknumber = track->trackNumber();
    }
    if ( track->length() > 0 )
    {
        // Multiply by 1000 since this is in milliseconds
        trackmeta->duration = track->length();
    }
    if ( !track->playableUrl().fileName().isEmpty() )
    {
        trackmeta->filename = qstrdup( track->playableUrl().fileName().toUtf8() );
    }
    trackmeta->filesize = track->filesize();
}

QString
MtpHandler::getFormat( LIBMTP_track_t *mtptrack )
{
    QString format;

    if ( mtptrack->filetype == LIBMTP_FILETYPE_MP3 )
        format = "mp3";
    else if ( mtptrack->filetype == LIBMTP_FILETYPE_WMA )
        format = "wma";
    else if ( mtptrack->filetype == LIBMTP_FILETYPE_OGG )
        format = "ogg";
    else
        format = "other";

    return format;
}

void
MtpHandler::setupArtistMap( LIBMTP_track_t *mtptrack, Meta::MtpTrackPtr track, ArtistMap &artistMap )
{
    QString artist( QString::fromUtf8( mtptrack->artist ) );
    MtpArtistPtr artistPtr;

    if ( artistMap.contains( artist ) )
    {
        artistPtr = MtpArtistPtr::staticCast( artistMap.value( artist ) );
    }
    else
    {
        artistPtr = MtpArtistPtr( new MtpArtist( artist ) );
        artistMap.insert( artist,  ArtistPtr::staticCast( artistPtr ) );
    }

    artistPtr->addTrack( track );
    track->setArtist( artistPtr );
}

void
MtpHandler::setupAlbumMap( LIBMTP_track_t *mtptrack, Meta::MtpTrackPtr track, AlbumMap &albumMap )
{
    QString album( QString::fromUtf8( mtptrack->album ) );
    MtpAlbumPtr albumPtr;

    if ( albumMap.contains( album ) )
        albumPtr = MtpAlbumPtr::staticCast( albumMap.value( album ) );

    else
    {
        albumPtr = MtpAlbumPtr( new MtpAlbum( album ) );
        albumMap.insert( album,  AlbumPtr::staticCast( albumPtr ) );
    }

    albumPtr->addTrack( track );
    track->setAlbum( albumPtr );
}

void
MtpHandler::setupGenreMap( LIBMTP_track_t *mtptrack, Meta::MtpTrackPtr track, GenreMap &genreMap )
{
    QString genre = mtptrack->genre;
    MtpGenrePtr genrePtr;

    if ( genreMap.contains( genre ) )
        genrePtr = MtpGenrePtr::staticCast( genreMap.value( genre ) );

    else
    {
        genrePtr = MtpGenrePtr( new MtpGenre( genre ) );
        genreMap.insert( genre,  GenrePtr::staticCast( genrePtr ) );
    }

    genrePtr->addTrack( track );
    track->setGenre( genrePtr );
}

void
MtpHandler::setupComposerMap( LIBMTP_track_t *mtptrack, Meta::MtpTrackPtr track, ComposerMap &composerMap )
{
    QString composer( QString::fromUtf8( mtptrack->composer ) );
    MtpComposerPtr composerPtr;

    if ( composerMap.contains( composer ) )
    {
        composerPtr = MtpComposerPtr::staticCast( composerMap.value( composer ) );
    }
    else
    {
        composerPtr = MtpComposerPtr( new MtpComposer( composer ) );
        composerMap.insert( composer, ComposerPtr::staticCast( composerPtr ) );
    }

    composerPtr->addTrack( track );
    track->setComposer( composerPtr );
}

void
MtpHandler::setupYearMap( LIBMTP_track_t *mtptrack, Meta::MtpTrackPtr track, YearMap &yearMap )
{
    QString year;
    year = year.setNum(( QString::fromUtf8( mtptrack->date ) ).mid( 0, 4 ).toUInt() );
    MtpYearPtr yearPtr;
    if ( yearMap.contains( year ) )
        yearPtr = MtpYearPtr::staticCast( yearMap.value( year ) );
    else
    {
        yearPtr = MtpYearPtr( new MtpYear( year ) );
        yearMap.insert( year,  YearPtr::staticCast( yearPtr ) );
    }
    yearPtr->addTrack( track );
    track->setYear( yearPtr );
}

bool
MtpHandler::kioCopyTrack( const KUrl &src, const KUrl &dst )
{
    DEBUG_BLOCK

    KIO::FileCopyJob *job = KIO::file_copy( src, dst,
                                            -1 /* permissions */,
                                            KIO::HideProgressInfo );

    connect( job, SIGNAL( result( KJob * ) ),
             this,  SLOT( fileTransferred( KJob * ) ) );

    The::statusBar()->newProgressOperation( job, i18n( "Transferring Tracks to MTP Device" ) );
    job->start();

    return true;
}
void
MtpHandler::deleteFile( const KUrl &url )
{
    Q_UNUSED( url );
}


void
MtpHandler::parseTracks()
{
    // NOTE: look at ReadMtpMusic function in MTPMediaDevice

    TrackMap trackMap;
    ArtistMap artistMap;
    AlbumMap albumMap;
    GenreMap genreMap;
    ComposerMap composerMap;
    YearMap yearMap;

    // TODO: implement callback right, not 0

    LIBMTP_track_t *tracks = LIBMTP_Get_Tracklisting_With_Callback( m_device, 0, this );



    if ( tracks == 0 )
    {
        return;
    }

    QMap<LIBMTP_track_t*, MtpTrackPtr> mtpTrackMap;

    /* iterate through tracklist and add to appropriate map */
    for ( ; tracks != 0; tracks = tracks->next )
    {
        /* mtptrack - provides libmtp info */
        /* track - the new track whose data is being set up */
        LIBMTP_track_t *mtptrack = tracks;
        QString format = getFormat( mtptrack );

        MtpTrackPtr track( new MtpTrack( m_memColl, format ) );

        /* fetch basic information */

        getBasicMtpTrackInfo( mtptrack, track );

        /* map-related info retrieval */

        setupArtistMap( mtptrack, track, artistMap );
        setupAlbumMap( mtptrack, track, albumMap );
        setupGenreMap( mtptrack, track, genreMap );
        setupComposerMap( mtptrack, track, composerMap );
        setupYearMap( mtptrack, track, yearMap );

        /* TrackMap stuff to be subordinated later */

        trackMap.insert( track->uidUrl(), TrackPtr::staticCast( track ) );

        m_titlemap.insert( track->name(), TrackPtr::staticCast( track ) );

        track->setMtpTrack( mtptrack ); // convenience pointer
        mtpTrackMap.insert( mtptrack, track ); // map for playlist formation
    }

    // TODO: Iterate through mtp's playlists to set track's playlists

    // Finally, assign the created maps to the collection

    m_memColl->acquireWriteLock();
    m_memColl->setTrackMap( trackMap );
    m_memColl->setArtistMap( artistMap );
    m_memColl->setAlbumMap( albumMap );
    m_memColl->setGenreMap( genreMap );
    m_memColl->setComposerMap( composerMap );
    m_memColl->setYearMap( yearMap );
    m_memColl->releaseLock();
}

void
MtpHandler::updateTrackInDB( const Meta::MtpTrackPtr track )
{
    DEBUG_BLOCK

    // pull out track struct to prepare for update

    LIBMTP_track_t *mtptrack = track->getMtpTrack();

    // update all fields

    setBasicMtpTrackInfo( mtptrack, track );

    // metadata set, commence update on device

    int failed = LIBMTP_Update_Track_Metadata( m_device, mtptrack );

    if ( !failed )
        debug() << "Metadata update succeeded!";

    else
        debug() << "Failed to update metadata";
}

void
MtpHandler::addMtpTrackToCollection( LIBMTP_track_t *mtptrack )
{

    // TODO: implement
    TrackMap trackMap = m_memColl->trackMap();
    ArtistMap artistMap = m_memColl->artistMap();
    AlbumMap albumMap = m_memColl->albumMap();
    GenreMap genreMap = m_memColl->genreMap();
    ComposerMap composerMap = m_memColl->composerMap();
    YearMap yearMap = m_memColl->yearMap();

    QString format;

    if ( mtptrack->filetype == LIBMTP_FILETYPE_MP3 )
        format = "mp3";
    else if ( mtptrack->filetype == LIBMTP_FILETYPE_WMA )
        format = "wma";
    else if ( mtptrack->filetype == LIBMTP_FILETYPE_OGG )
        format = "ogg";
    else
        format = "other";


    MtpTrackPtr track( new MtpTrack( m_memColl, format ) );

    /* 1-liner info retrieval */

    getBasicMtpTrackInfo( mtptrack, track );

    /* map-related info retrieval */
    setupArtistMap( mtptrack, track, artistMap );
    setupAlbumMap( mtptrack, track, albumMap );
    setupGenreMap( mtptrack, track, genreMap );
    setupComposerMap( mtptrack, track, composerMap );
    setupYearMap( mtptrack, track, yearMap );

    /* trackmap also soon to be subordinated */

    trackMap.insert( track->uidUrl(), TrackPtr::staticCast( track ) );

    m_titlemap.insert( track->name(), TrackPtr::staticCast( track ) );

    track->setMtpTrack( mtptrack ); // convenience pointer
    // NOTE: not supporting adding track that's already on a playlist
    //mtpTrackMap.insert( mtptrack, track ); // map for playlist formation

    // Finally, assign the created maps to the collection

    m_memColl->acquireWriteLock();
    m_memColl->setTrackMap( trackMap );
    m_memColl->setArtistMap( artistMap );
    m_memColl->setAlbumMap( albumMap );
    m_memColl->setGenreMap( genreMap );
    m_memColl->setComposerMap( composerMap );
    m_memColl->setYearMap( yearMap );
    m_memColl->releaseLock();

    return;
}

void
MtpHandler::fileTransferred( KJob *job )  //SLOT
{
    if ( job->error() )
    {
        m_copyFailed = true;
        debug() << "file transfer failed: " << job->errorText();
    }
    else
    {
        m_copyFailed = false;
    }

    m_wait = false;
}

void
MtpHandler::fileDeleted( KJob *job )  //SLOT
{
    if ( job->error() )
    {
        debug() << "file deletion failed: " << job->errorText();
    }
}

QString
MtpHandler::prettyName() const
{
    return m_name;
}

void
MtpHandler::slotDeviceMatchSucceeded( ThreadWeaver::Job* job )
{
    DEBUG_BLOCK
    if ( job->success() )
    {
        getDeviceInfo();
        debug() << "Device matches serial, emitting succeeded()";
        emit succeeded();
    }
}

void
MtpHandler::slotDeviceMatchFailed( ThreadWeaver::Job* job )
{
    DEBUG_BLOCK
    debug() << "Running slot device match failed";
    disconnect( job, SIGNAL( done( ThreadWeaver::Job* ) ), this, SLOT( slotDeviceMatchSucceeded() ) );
    emit failed();
}

WorkerThread::WorkerThread( int numrawdevices, LIBMTP_raw_device_t* rawdevices, const QString &serial, MtpHandler* handler )
        : ThreadWeaver::Job()
        , m_success( false )
        , m_numrawdevices( numrawdevices )
        , m_rawdevices( rawdevices )
        , m_serial( serial )
        , m_handler( handler )
{
    connect( this, SIGNAL( failed( ThreadWeaver::Job* ) ), m_handler, SLOT( slotDeviceMatchFailed( ThreadWeaver::Job* ) ) );
    connect( this, SIGNAL( done( ThreadWeaver::Job* ) ), m_handler, SLOT( slotDeviceMatchSucceeded( ThreadWeaver::Job* ) ) );
    connect( this, SIGNAL( done( ThreadWeaver::Job* ) ), this, SLOT( deleteLater() ) );
}

WorkerThread::~WorkerThread()
{
    //nothing to do
}

bool
WorkerThread::success() const
{
    return m_success;
}

void
WorkerThread::run()
{
    m_success = m_handler->iterateRawDevices( m_numrawdevices, m_rawdevices, m_serial );
}

void
MtpHandler::slotCopyNextTrackFailed( ThreadWeaver::Job* job )
{
    Q_UNUSED( job );
    m_copyFailed = true;
    QString error = "Job Failed";
    m_tracksFailed.insert( m_lastTrackCopied, error );

    copyNextTrackToDevice();
}

void
MtpHandler::slotCopyNextTrackToDevice( ThreadWeaver::Job* job )
{
    if ( job->success() )
    {
        emit incrementProgress();
    }
    else
    {
        m_copyFailed = true;
        QString error = "MTP copy error";
        m_tracksFailed.insert( m_lastTrackCopied, error );
    }

    copyNextTrackToDevice();
}

CopyWorkerThread::CopyWorkerThread( const Meta::TrackPtr &track, MtpHandler* handler )
        : ThreadWeaver::Job()
        , m_success( false )
        , m_track( track )
        , m_handler( handler )
{
    connect( this, SIGNAL( failed( ThreadWeaver::Job* ) ), m_handler, SLOT( slotCopyNextTrackFailed( ThreadWeaver::Job* ) ) );
    connect( this, SIGNAL( done( ThreadWeaver::Job* ) ), m_handler, SLOT( slotCopyNextTrackToDevice( ThreadWeaver::Job* ) ) );
    connect( this, SIGNAL( done( ThreadWeaver::Job* ) ), this, SLOT( deleteLater() ) );
}

CopyWorkerThread::~CopyWorkerThread()
{
    //nothing to do
}

bool
CopyWorkerThread::success() const
{
    return m_success;
}

void
CopyWorkerThread::run()
{
    m_success = m_handler->privateCopyTrackToDevice( m_track );
}
