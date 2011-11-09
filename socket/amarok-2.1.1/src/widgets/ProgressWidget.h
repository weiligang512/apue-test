/***************************************************************************
 * copyright     : (C) 2007 Dan Meltzer <parallelgrapefruit@gmail.com>   *
 **************************************************************************/

 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AMAROK_PROGRESSWIDGET_H
#define AMAROK_PROGRESSWIDGET_H

#include "EngineObserver.h"

#include <unistd.h>

#include <QHash>
#include <QPainter>
#include <QPolygon>
#include <QWidget>

class TimeLabel;
namespace Amarok { class TimeSlider; }

class ProgressWidget : public QWidget, public EngineObserver
{
    Q_OBJECT

    AMAROK_EXPORT static ProgressWidget *s_instance;

    public:
        ProgressWidget( QWidget* );
        static ProgressWidget *instance() { return s_instance; }

        virtual QSize sizeHint() const;

        void addBookmark( const QString &name, int x );
        void redrawBookmarks();

    public slots:
        void drawTimeDisplay( int position );

    protected:
        virtual void engineTrackPositionChanged( long position, bool /*userSeek*/ );
        virtual void engineStateChanged( Phonon::State state, Phonon::State oldState = Phonon::StoppedState );
        virtual void engineTrackLengthChanged( long seconds );
        virtual void engineNewTrackPlaying();
        virtual void enginePlaybackEnded( int finalPosition, int trackLength, PlaybackEndedReason reason );

    private:
        TimeLabel *m_timeLabelLeft;
        TimeLabel *m_timeLabelRight;
        int m_timeLength;
        Amarok::TimeSlider *m_slider;
        QString m_currentUrlId;
};

#endif

