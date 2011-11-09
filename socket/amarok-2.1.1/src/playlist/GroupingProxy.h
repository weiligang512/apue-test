/***************************************************************************
 * copyright        : (C) 2007-2008 Ian Monroe <ian@monroe.nu>
 *                    (C) 2007 Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>
 *                    (C) 2008 Seb Ruiz <ruiz@kde.org>
 *                    (C) 2008 Soren Harward <stharward@gmail.com>
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

#ifndef GROUPINGPROXY_H
#define GROUPINGPROXY_H

#include "meta/Meta.h"
#include "playlist/navigators/NavigatorFilterProxyModel.h"

#include <QAbstractProxyModel>
#include <QHash>
#include <QModelIndex>

class QMimeData;

/* Ordinarily, we'd just subclass PlaylistModel and tweak a couple of the
 * functions that need to be different, but the singleton nature of the
 * PlaylistModel makes inheritance much more difficult, so we have to use a
 * proxy model instead.  Unfortunately, QAbstractProxyModel doesn't proxy
 * all of its functions automatically, so we have to proxy a lot of
 * them manually.  -- stharward */

namespace Playlist
{
enum GroupDataRoles
{
    GroupRole = 256,
    GroupedTracksRole, // deprecated
    GroupedAlternateRole // deprecated
};

enum GroupMode
{
    None = 1,
    Head,
    Head_Collapsed, // deprecated
    Body,
    Tail,
    Collapsed // deprecated
};

class GroupingProxy : public QAbstractProxyModel
{

    Q_OBJECT

public:
    static GroupingProxy* instance();
    static void destroy();

    // functions from QAbstractProxyModel
    QModelIndex index( int, int c = 0, const QModelIndex& parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex& ) const;
    int rowCount( const QModelIndex& idx = QModelIndex() ) const;
    int columnCount( const QModelIndex& ) const;
    QModelIndex mapToSource( const QModelIndex& ) const;
    QModelIndex mapFromSource( const QModelIndex& ) const;
    QVariant data( const QModelIndex &index, int role ) const;

    // wrapped functions from PlaylistModel
    int activeRow() const;
    void setActiveRow( int ) const;
    Meta::TrackPtr trackAt( int ) const;
    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags( const QModelIndex& ) const;
    QStringList mimeTypes() const;
    QMimeData* mimeData( const QModelIndexList& ) const;
    bool dropMimeData( const QMimeData*, Qt::DropAction, int, int, const QModelIndex& );

    // grouping-related functions
    void setCollapsed( int, bool ) const;
    int firstInGroup( int ) const;
    int lastInGroup( int ) const;

    int tracksInGroup( int row ) const;
    int lengthOfGroup( int row ) const;

    int find( const QString & searchTerm, int fields );
    int findNext( const QString & searchTerm, int selectedRow, int fields  );
    int findPrevious( const QString & searchTerm, int selectedRow, int fields  );

    void clearSearchTerm();

    int totalLength();

    QString currentSearchTerm();
    int currentSearchFields();

signals:
    void rowsInserted( const QModelIndex&, int, int );
    void rowsRemoved( const QModelIndex&, int, int );

private slots:
    void modelDataChanged( const QModelIndex&, const QModelIndex& );
    void modelRowsInserted( const QModelIndex&, int, int );
    void modelRowsRemoved( const QModelIndex&, int, int );
    void regroupAll();

private:
    GroupingProxy();
    ~GroupingProxy();

    void regroupRows( int firstRow, int lastRow );
    QList<GroupMode> m_rowGroupMode;

    // grouping auxiliary functions -- deprecated, but used by GraphicsView
    int groupRowCount( int row ) const;

    /**
     * This function is used to determine if 2 tracks belong in the same group.
     * The current implementation is a bit of a hack, but is what gives the best
     * user experience. 
     * @param track1 The first track
     * @param track2 The second track
     * @return true if track should be grouped together, false otherwise
     */
    bool shouldBeGrouped( Meta::TrackPtr track1, Meta::TrackPtr track2 );

    NavigatorFilterProxyModel *m_model;

    static GroupingProxy* s_instance;
};
} // namespace Playlist

#endif
