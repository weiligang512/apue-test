/***************************************************************************
 * copyright     : (C) 2008 Jeff Mitchell <mitchell@kde.org>               *
 **************************************************************************/

 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FIRSTRUNTUTORIAL_H
#define FIRSTRUNTUTORIAL_H

#include "amarokconfig.h"
#include "FirstRunTutorialPage.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHash>
#include <QTimeLine>
#include <QWidget>

class FirstRunTutorial : public QObject
{
    Q_OBJECT

    public:
        FirstRunTutorial( QWidget *parent );
        ~FirstRunTutorial();

    public slots:
        void initOverlay();

    private slots:
        void fadeShowTimerFrameChanged( int frame );
        void fadeShowTimerFinished();
        void fadeHideTimerFrameChanged( int frame );
        void fadeHideTimerFinished();
        void nextPage();

        void slotPage1();

    protected:
        virtual bool eventFilter( QObject* watched, QEvent* event );

    private:
        QWidget* m_parent;
        QGraphicsScene *m_scene;
        QGraphicsView *m_view;
        QTimeLine m_fadeShowTimer;
        QTimeLine m_fadeHideTimer;
        int m_framesMax;
        QSet<QGraphicsItem*> m_itemSet;
        QHash<int, FirstRunTutorialPage*> m_pages;
        int m_pageNum;
};

#endif
