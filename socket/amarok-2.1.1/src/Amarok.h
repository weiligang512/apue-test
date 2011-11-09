/*
    Copyright (c) 2004-2005 Max Howell <max.howell@methylblue.com>
    Copyright (c) 2009 Mark Kretschmann <kretschmann@kde.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AMAROK_H
#define AMAROK_H

#include "amarok_export.h"

#include <KActionCollection>
#include <KConfig>
#include <KIO/NetAccess>
#include <KUrl> // recursiveUrlExpand

#include <QPointer>

class QColor;
class QDateTime;
class QEvent;
class QMutex;
class QPixmap;
class QWidget;

namespace KIO { class Job; }

namespace Amarok
{
    const int VOLUME_MAX = 100;
    const int SCOPE_SIZE = 9; //= 2**9 = 512
    const int blue       = 0x202050;
    const int VOLUME_SENSITIVITY = 30; //for mouse wheels
    const int GUI_THREAD_ID = 0;

    extern QMutex globalDirsMutex; // defined in App.cpp
    extern QPointer<KActionCollection> actionCollectionObject; // defined in App.cpp


    namespace ColorScheme
    {
        ///eg. base of the Amarok Player-window
        extern QColor Base; //Amarok::blue
        ///eg. text in the Amarok Player-window
        extern QColor Text; //Qt::white
        ///eg. background colour for Amarok::PrettySliders
        extern QColor Background; //brighter blue
        ///eg. outline of slider widgets in Player-window
        extern QColor Foreground; //lighter blue
        ///eg. K3ListView alternative row color
        extern QColor AltBase; //grey toned base
    }

    /** The version of the playlist XML format. Increase whenever it changes backwards-incompatibly. */
    inline QString xmlVersion() { return "2.4"; }

    /**
     * Convenience function to return the KApplication instance KConfig object
     * pre-set to a specific group.
     * @param group Will pre-set the KConfig object to this group.
     */
    /* FIXME: This function can lead to very bizarre and hard to figure bugs.
              While we don`t fix it properly, use it like this: amarok::config( Group )->readEntry( ... ) */
    AMAROK_EXPORT KConfigGroup config( const QString &group = "General" ); //defined in App.cpp

    /**
     * @return the KActionCollection used by Amarok
     */
    KActionCollection *actionCollection(); //defined in App.cpp

    /**
     * Compute score for a track that has finished playing.
     */
    inline double computeScore( double oldScore, int playCount, double playedFraction )
    {
        const int percentage = static_cast<int>(playedFraction * 100); 
        double newScore;

        if( playCount == 0 )
            newScore = ( oldScore + percentage ) / 2;
        else
            newScore = ( ( oldScore * playCount ) + percentage ) / ( playCount + 1 );

        return newScore;
    }

    /**
     * The mainWindow is the playlistWindow
     */
    AMAROK_EXPORT QWidget *mainWindow(); //defined in App.cpp

    /**
     * Allocate one on the stack, and it'll set the busy cursor for you until it
     * is destroyed
     */
    class OverrideCursor { //defined in App.cpp
    public:
        OverrideCursor( Qt::CursorShape cursor = Qt::WaitCursor );
       ~OverrideCursor();
    };

    /**
     * For saving files to ~/.kde/share/apps/amarok/directory
     * @param directory will be created if not existing, you MUST end the string
     *                  with '/'
     */
    AMAROK_EXPORT QString saveLocation( const QString &directory = QString() ); //defined in collectionreader.cpp

    KIO::Job *trashFiles( const KUrl::List &files ); //defined in App.cpp

    /**
     * For recursively expanding the contents of a directory into a KUrl::List
     * (playlists are ignored)
     */

    //New in Amarok2 -> recursiveUrlExpand has been replaced
    //existing code depending on this port need to be changed (max urls is removed)
    AMAROK_EXPORT KUrl::List recursiveUrlExpand( const KUrl &url ); //defined in PlaylistHandler.cpp
    AMAROK_EXPORT KUrl::List recursiveUrlExpand( const KUrl::List &urls ); //defined in PlaylistHandler.cpp

    AMAROK_EXPORT QString verboseTimeSince( const QDateTime &datetime ); // defined in App.cpp

    AMAROK_EXPORT QString verboseTimeSince( uint time_t ); // defined in App.cpp

    AMAROK_EXPORT QString conciseTimeSince( uint time_t ); // defined in App.cpp

    /**
     * Function that must be used when separating contextBrowser escaped urls
     */
    // defined in App.cpp
    void albumArtistTrackFromUrl( QString url, QString &artist, QString &album, QString &detail );

    /**
     * @return the LOWERCASE file extension without the preceding '.', or "" if there is none
     */
    inline QString extension( const QString &fileName )
    {
        if( fileName.contains( '.' ) )
        {
            QString ext = fileName.mid( fileName.lastIndexOf( '.' ) + 1 ).toLower();
            // Remove url parameters (some remote playlists use these)
            if( ext.contains( '?' ) )
                return ext.left( ext.indexOf( '?' ) );
            return ext;
        }

        return QString();
    }

    /** Transform url into a file url if possible */
    inline KUrl mostLocalURL( const KUrl &url )
    {
        return KIO::NetAccess::mostLocalUrl( url, mainWindow() );
    }

    /**
     * @return the last directory in @param fileName
     */
    inline QString directory( const QString &fileName )
    {
        return fileName.section( '/', 0, -2 );
    }

    /**
    * Returns internal code for database type, DbConnection::sqlite or DbConnection::mysql
    * @param type either "SQLite", or "MySQL".
    */
    int databaseTypeCode( const QString type ); //defined in configdialog.cpp

    void setUseScores( bool use ); //defined in App.cpp
    void setUseRatings( bool use );

    bool repeatNone(); //defined in actionclasses.cpp
    bool repeatTrack();
    bool repeatAlbum();
    bool repeatPlaylist();
    bool randomOff();
    bool randomTracks();
    bool randomAlbums();
    bool repeatEnabled();
    bool randomEnabled();
    bool favorNone();
    bool favorScores();
    bool favorRatings();
    bool favorLastPlay();

    void setDynamicPlaylist( const QString& title );  // defined in dynamicmodel.cpp
    void enableDynamicMode( bool enable );


    QStringList splitPath( QString path ); //defined in playlistbrowser.cpp

    /**
     * Removes accents from the string
     * @param path The original path.
     * @return The cleaned up path.
     */
    AMAROK_EXPORT QString cleanPath( const QString &path ); //defined in App.cpp

    /**
     * Replaces all non-ASCII characters with '_'.
     * @param path The original path.
     * @return The ASCIIfied path.
     */
    AMAROK_EXPORT QString asciiPath( const QString &path ); //defined in App.cpp

    /**
     * Transform path into one valid on VFAT file systems
     * @param path The original path.
     * @return The cleaned up path.
     */
    AMAROK_EXPORT QString vfatPath( const QString &path ); //defined in App.cpp

    /**
     * Compare both strings from left to right and remove the common part from input
     * @param input the string that get's cleaned.
     * @param ref a reference to compare input with.
     * @return The cleaned up string.
     */
    AMAROK_EXPORT QString decapitateString( const QString &input, const QString &ref );

    /*
     * Transform to be usable within HTML/HTML attributes
     */
    AMAROK_EXPORT QString escapeHTMLAttr( const QString &s ); // defined in App.cpp
    AMAROK_EXPORT QString unescapeHTMLAttr( const QString &s ); //defined in App.cpp

    /* defined in scriptmanager.cpp */
    /**
     * Returns the proxy that should be used for a given URL.
     * @param url the url.
     * @return The url of the proxy, or a empty string if no proxy should be used.
     */
    QString proxyForUrl(const QString& url);

    /**
     * Returns the proxy that should be used for a given protocol.
     * @param protocol the protocol.
     * @return The url of the proxy, or a empty string if no proxy should be used.
     */
    QString proxyForProtocol(const QString& protocol);

    /*defined in collectionbrowser/collectiontreeitemmodel.cpp */
    /**
     * Small function aimed to convert Eagles, The -> The Eagles (and back again).
     * @param str the string to manipulate
     * @param reverse if true, The Eagles -> Eagles, The. If false, Eagles, The -> The Eagles
     */
    AMAROK_EXPORT void manipulateThe( QString &str, bool reverse );

}


/**
 * Use this to const-iterate over QStringLists, if you like.
 * Watch out for the definition of last in the scope of your for.
 *
 *     QStringList strings;
 *     oldForeach( strings )
 *         debug() << *it << endl;
 */
#define oldForeach( x ) \
    for( QStringList::ConstIterator it = x.constBegin(), end = x.constEnd(); it != end; ++it )

/**
 * You can use this for lists that aren't QStringLists.
 * Watch out for the definition of last in the scope of your for.
 *
 *     BundleList bundles;
 *     oldForeachType( BundleList, bundles )
 *         debug() << *it.url() << endl;
 */
#define oldForeachType( Type, x ) \
    for( Type::ConstIterator it = x.constBegin(), end = x.constEnd(); it != end; ++it )

/**
 * Creates iterators of type @p Type.
 * Watch out for the definitions of last and end in your scope.
 *
 *     BundleList bundles;
 *     for( for_iterators( BundleList, bundles ); it != end; ++it )
 *         debug() << *it.url() << endl;
 */
#define for_iterators( Type, x ) \
    Type::ConstIterator it = x.constBegin(), end = x.constEnd(), last = x.fromLast()


/// Update this when necessary
#define APP_VERSION "2.1.1"

#endif
