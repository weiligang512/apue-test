/***************************************************************************
 *   Copyright (c) 2008  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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

#ifndef PLAYLISTNAVIGATORFILTERPROXYMODEL_H
#define PLAYLISTNAVIGATORFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

#include "playlist/PlaylistItem.h"

namespace Playlist
{
/**
A proxy model used by navigators to only operate on tracks that match the current paylist search term

This model only forwards the functions needed by the navigators and is not intended to be used for
populating a view. The proxy also provides a number of special functions to deal with cases like when
a search term is active and the currently playing track is not in the subset represented by this proxy.

    @author Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>
*/
class NavigatorFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:

    /**
     * Accessor function for singleton pattern.
     * @return The class instance.
     */
    static NavigatorFilterProxyModel* instance();

    /**
     * Return the currently active row, translated to proxy rows
     * (or -1 if the current row is not represented by this proxy).
     * @return The currently active (playing) row in proxy terms.
     */
    int activeRow() const;

    /**
     * Find the id  of the track at a given row in the proxy model.
     * @param row The row in proxy terms.
     * @return The id of the row.
     */
    quint64 idAt( const int row ) const;

    /**
     * Get the sate of the track at given row in the proxy model.
     * @param row The row in proxy terms.
     * @return The state of the track at the row.
     */
    Item::State stateOfRow( int row ) const;

    /**
     * Get the state of a track by its id.
     * @param id The id of the track.
     * @return The state of the track.
     */
    Item::State stateOfId( quint64 id ) const;

    /**
     * Get the first row below the currently active track that matches the
     * current search term.
     * @return The first matching row.
     */
    int firstMatchAfterActive();

    /**
     * Get the first row above the currently active track that matches the
     * current search term.
     * @return The first matching row.
     */
    int firstMatchBeforeActive();

    /**
     * Notify proxy that the search term of searched fields has changed. Since
     * this calls does not use the parents filter values, this method needs to be
     * called when the values change.
     */
    void filterUpdated();

    /**
     * Toggle acting in pass through mode. When in pass through mode, this proxy
     * is basically completely transparent, and ignores any search terms. It also
     * ignores any calls to filterUpdated() while in pass through mode,.
     * @param passThrough Determines whether pass through mode is enabled.
     */
    void setPassThrough( bool passThrough );

    void setRowQueued( int row );
    void setRowDequeued( int row );

    QVariant data( const QModelIndex& index, int role ) const;

    bool rowExists( int row ) const;
    void setActiveRow( int row );
    Meta::TrackPtr trackAt( int row ) const;

    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags( const QModelIndex& ) const;
    QStringList mimeTypes() const;
    QMimeData* mimeData( const QModelIndexList& ) const;
    bool dropMimeData( const QMimeData*, Qt::DropAction, int, int, const QModelIndex& );

    int find( const QString & searchTerm, int searchFields );
    int findNext( const QString & searchTerm, int selectedRow, int searchFields );
    int findPrevious( const QString & searchTerm, int selectedRow, int searchFields );
    int totalLength() const;

    void clearSearchTerm();
    QString currentSearchTerm();
    int currentSearchFields();

    int rowToSource( int row ) const;
    int rowFromSource( int row ) const;

protected:
    /**
     * Reimplemented from QSortFilterProxyModel. Used internally by the proxy to
     * determine if a given row in the source model should be included in this
     * proxy. When in pass through mode, this always returns true.
     * @param row The row in the source model to check.
     * @param source_parent Ignored.
     * @return True if the row should be included, false otherwise.
     */
    virtual bool filterAcceptsRow ( int row, const QModelIndex & source_parent ) const;

protected slots:
    /**
     * Slot called when the source model has inserted new tracks. Uses filterAcceptsRow
     * to determine if a given id should be included in the list forwarded to any
     * listeners in the insertedIds() signal.
     * @param ids the list of id's added to the source model.
     */
    void slotInsertedIds( const QList<quint64> &ids );

    /**
     * Slot called when the source model has removed tracks. Uses filterAcceptsRow
     * to determine if a given id should be included in the list forwarded to any
     * listeners in the removedIds() signal.
     * @param ids the list of id's removed from the source model.
     */
    void slotRemovedIds( const QList<quint64> &ids );

signals:
    /**
     * Signal forwarded from the source model.
     * @param the list of id's added that are also represented by this proxy.
     */
    void insertedIds( const QList<quint64>& );

    /**
     * Signal forwarded from the source model.
     * @param the list of id's removed that are also represented by this proxy.
     */
    void removedIds( const QList<quint64>& );

    /**
     * Signal emitted when the proxy changes its filtering.
     */
    void filterChanged();

private:
    /**
     * Constructor.
     */
    NavigatorFilterProxyModel();

    /**
     * Destructor.
     */
    ~NavigatorFilterProxyModel();

    bool m_passThrough;

    static NavigatorFilterProxyModel* s_instance;      //! instance variable
};

}

#endif
