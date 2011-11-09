/***************************************************************************
 * copyright     : (C) 2004 Mark Kretschmann <markey@web.de>               *
                   (C) 2007 Dan Meltzer <parallelgrapefruit@gmail.com>     *
                   (C) 2009 Kevin Funk <krf@electrostorm.net               *
 **************************************************************************/

 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VOLUMEWIDGET_H
#define VOLUMEWIDGET_H

#include "EngineObserver.h"
#include "SliderWidget.h"
#include "ToolBar.h"

#include <KAction>

#include <QLabel>
#include <QPointer>
#include <QStringList>

/**
* A custom widget that serves as our volume slider within Amarok.
*/
class VolumeWidget : public Amarok::ToolBar, public EngineObserver
{
    Q_OBJECT
public:
    VolumeWidget( QWidget * );
    Amarok::VolumeSlider* slider() const { return m_slider; }

private slots:
    void engineVolumeChanged( int value );
    void engineMuteStateChanged( bool mute );

private:
    QPointer<Amarok::VolumeSlider> m_slider;
    KAction *m_action;
    QLabel *m_label;
    QStringList m_icons;
};

#endif
