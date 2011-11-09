/***************************************************************************
*   Copyright (c) 2009  Casey Link <unnamedrambler@gmail.com>             *
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

#ifndef TIMECODEOBSERVER_H
#define TIMECODEOBSERVER_H

#include "EngineController.h"


/**
 * This class handles auto timecoding (position bookmarking) of tracks.
 * After the current track's position has crossed an arbitrary threshold
 * when the user stops playing the track (before the ending) a timecode
 * will be created.
 */
class TimecodeObserver : public EngineObserver
{
public:
    TimecodeObserver();
    virtual ~TimecodeObserver();

    virtual void engineNewTrackPlaying();
    virtual void enginePlaybackEnded ( int finalPosition, int trackLength, EngineObserver::PlaybackEndedReason reason );
    virtual void engineTrackPositionChanged ( long position, bool userSeek );

private:
    bool m_trackTimecodeable; //!< stores if current track has the writetimecode capability
    static const int m_threshold;  //!< the arbitrary minum tracklength threshold in seconds
    Meta::TrackPtr m_currentTrack; //!< The current/just played track
    long m_currPos; //!< the position the current track is at
};

#endif // TIMECODEOBSERVER_H
