/***************************************************************************
 * copyright     : (C) 2007 Dan Meltzer <parallelgrapefruit@gmail.com>   *
 *               : (C) 2008 Soren Harward <stharward@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 of     *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "RepeatTrackNavigator.h"

#include "playlist/PlaylistModel.h"

Playlist::RepeatTrackNavigator::RepeatTrackNavigator()
{
    m_trackid = Model::instance()->activeId();

    connect( Model::instance(), SIGNAL( activeTrackChanged( const quint64 ) ), this, SLOT( recvActiveTrackChanged( const quint64 ) ) );
}
