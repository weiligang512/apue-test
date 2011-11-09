/***************************************************************************
 *   Plasma applet for showing video from youtube dailymotion and          *
 *   vimeo in the context view.                                            *
 *                                                                         *
 *   Copyright 2009 Simon Esneault <simon.esneault@gmail.com>              *
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

#ifndef VIDEOCLIP_APPLET_H
#define VIDEOCLIP_APPLET_H

#include "context/Applet.h"
#include "context/DataEngine.h"
#include "EngineObserver.h"

#include <Phonon/MediaObject>
#include <Phonon/VideoWidget>

class KratingWidget;
class KratingPainter;
class QGraphicsLinearLayout;
class QGraphicsProxyWidget;
class QGraphicsWidget;
class QHBoxLayout;

//!  Struct VideoInfo, contain all the info vor a video
struct VideoInfo {
    QString url;        // Url for the browser (http://www.youtube.com/watch?v=153d9tc3Oao )
    QString title;      // Name of the video
    QString coverurl;   // url of the cover
    QString duration;   // formatted as a QString(mm:ss)
    QString desc;       // full description
    QPixmap * cover;    // Image data
    QString views;      // number of view of the video
    float rating;       // rating should be beetween 0 to 5
    QString videolink;  // direct video link to the downloadable file
    QString source;     // "youtube" or "dailymotion" or "vimeo" or whatever
    int relevancy;      // used to filter and order the files
    int length;         // length in seconds 
    QString artist;     // The artist just to show it in the artist name
};

 /** VideoclipApplet will display videoclip from internet, relatively to the current playing song
   * If a video is detected in the playlist, it will also play trhe video inside the a VideoWidget.
   */
class VideoclipApplet : public Context::Applet, public EngineObserver
{
        Q_OBJECT

    public:
        VideoclipApplet( QObject* parent, const QVariantList& args );
        ~VideoclipApplet();

        void    init();
        void    paintInterface( QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect );

        void    constraintsEvent( Plasma::Constraints constraints = Plasma::AllConstraints );
        QSizeF  sizeHint( Qt::SizeHint which, const QSizeF & constraint = QSizeF() ) const;
        
        // inherited from EngineObserver
        virtual void engineNewTrackPlaying();
        virtual void enginePlaybackEnded( int finalPosition, int trackLength, PlaybackEndedReason reason );
        
        
    public slots:
        void    dataUpdated( const QString& name, const Plasma::DataEngine::Data& data );
        void    connectSource( const QString &source );
        void    appendVideoClip( );

    private:
        Phonon::MediaObject *m_mediaObject;
        Phonon::VideoWidget *m_videoWidget;

        // The two big container, only one who need a resize
        QGraphicsSimpleTextItem *m_headerText;
        QGraphicsProxyWidget    *m_widget;
        QHBoxLayout             *m_layout;
        QList<QWidget *>m_layoutWidgetList;
        
        int m_height;
        QPixmap     *m_pixYoutube;
        QPixmap     *m_pixDailymotion;
        QPixmap     *m_pixVimeo;

};

Q_DECLARE_METATYPE ( VideoInfo *);
K_EXPORT_AMAROK_APPLET( videoclip, VideoclipApplet )

#endif /* VIDEOCLIP_APPLET_H */

