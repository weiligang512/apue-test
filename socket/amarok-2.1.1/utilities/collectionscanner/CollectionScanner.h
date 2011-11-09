/***************************************************************************
 *   Copyright (C) 2003-2008 Mark Kretschmann <kretschmann@kde.org>        *
 *             (C) 2008 Dan Meltzer <parallelgrapefruit@gmail.com>         *
 *             (C) 2008-2009 Jeff Mitchell <mitchell@kde.org>              *
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

#ifndef COLLECTIONSCANNER_H
#define COLLECTIONSCANNER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDBusInterface>
#include <QHash>
#include <QStringList>

//Taglib includes..
#include <audioproperties.h>
#include <fileref.h>

typedef QHash<QString, QString> AttributeHash;

/**
 * @class CollectionScanner
 * @short Scans directories and builds the Collection
 */

class CollectionScanner : public QCoreApplication
{
    Q_OBJECT

public:
    CollectionScanner( int &argc, char **argv );

    ~CollectionScanner();
    int newInstance() { return 0; }

private slots:
    void doJob();

private:
    enum FileType
    {
        mp3,
        ogg,
        flac,
        mp4
    };

    bool readBatchIncrementalFile();

    void readDir( const QString& dir, QStringList& entries );
    void scanFiles( const QStringList& entries );
    
    /**
     * Get the render() of a tag from TagLib, if possible, to assist in AFT UID generation.
     * @fileref TagLib reference to the file.
     * @return TagLib::ByteVector that contains the tag's render, if possible, and empty otherwise.
     */
    static const QString readEmbeddedUniqueId( const TagLib::FileRef &fileref );

    /**
     * Get the render() of a tag from TagLib, if possible, to assist in AFT UID generation.
     * @fileref TagLib reference to the file.
     * @return TagLib::ByteVector that contains the tag's render, if possible, and empty otherwise.
     */
    static const TagLib::ByteVector generatedUniqueIdHelper( const TagLib::FileRef &fileref );

    /**
     * Create a random AFT UID, to prevent ever returning an empty one (since much of A2 uses unique
     * IDs of the various Meta types).
     * @param md5 Contains the QCryptographicHash to update
     * @return QString containing the uniqueID
     */
    static const QString randomUniqueId( QCryptographicHash &md5 );

    /**
     * Get the render() of a tag from TagLib, if possible, to assist in AFT UID generation.
     * @path Path of the file.
     * @return QString contains the uniqueID, an MD5 hash (but not a hash of the entire file!).
     */
    static const QString readUniqueId( const QString &path );

    /**
     * Read metadata tags of a given file.
     * @track Track for the file.
     * @return QMap containing tags, or empty QMap on failure.
     */
    AttributeHash readTags( const QString &path, TagLib::AudioProperties::ReadStyle readStyle = TagLib::AudioProperties::Fast );

    /**
     * Helper method for writing XML elements to stdout.
     * @name Name of the element.
     * @attributes Key/value map of attributes.
     */
    void writeElement( const QString& name, const AttributeHash& attributes );

    /**
     * @return the LOWERCASE file extension without the preceding '.', r "" if there is none
     */
    inline QString extension( const QString &fileName )
    {
        return fileName.contains( '.' ) ? fileName.mid( fileName.lastIndexOf( '.' ) + 1 ).toLower() : "";
    }

    /**
     * @return the last directory in @param fileName
     */
    inline QString directory( const QString &fileName )
    {
        return fileName.section( '/', 0, -2 );
    }

    QString escape( const QString &plain );
    void readArgs();
    void displayHelp();

    QString       m_collectionId;
    QString       m_amarokPid;
    bool          m_batch;
    bool          m_importPlaylists;
    QStringList   m_folders;
    QDateTime     m_batchFolderTime;
    bool          m_recursively;
    bool          m_incremental;
    bool          m_restart;
    QString       m_saveLocation;
    QString       m_logfile;
    QString       m_rpath;
    QStringList   m_scannedFolders;
    QDBusInterface *m_amarokCollectionInterface;

    // Disable copy constructor and assignment
    CollectionScanner( const CollectionScanner& );
    CollectionScanner& operator= ( const CollectionScanner& );

};


#endif // COLLECTIONSCANNER_H

