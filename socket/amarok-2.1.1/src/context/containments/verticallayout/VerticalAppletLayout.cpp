/***************************************************************************
* copyright            : (C) 2008 Leo Franchi <lfranchi@kde.org>         *
****************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "VerticalAppletLayout.h"

#include "Containment.h"
#include "Debug.h"

#include <plasma/applet.h>

#include <KConfig>

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneResizeEvent>

Context::VerticalAppletLayout::VerticalAppletLayout( QGraphicsItem* parent )
    : QGraphicsWidget( parent )
    , m_showingIndex( -1 )
{
}

Context::VerticalAppletLayout::~VerticalAppletLayout()
{
    
}

void 
Context::VerticalAppletLayout::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
    Q_UNUSED( painter )
    Q_UNUSED( option )
    Q_UNUSED( widget )
}

void
Context::VerticalAppletLayout::resizeEvent( QGraphicsSceneResizeEvent * event )
{
    // update all the applet widths
    foreach( Plasma::Applet* applet, m_appletList )
        applet->resize( event->newSize().width(), applet->size().height() );
    showAtIndex( m_showingIndex );
    
}

void 
Context::VerticalAppletLayout::addApplet( Plasma::Applet* applet, int location )
{
    DEBUG_BLOCK
    debug() << "layout told to add applet at" << location;
    if( m_appletList.size() == 0 )
        emit noApplets( false );
    
    if( location < 0 ) // being told to add at end
    {
        m_appletList << applet;
        showAtIndex( minIndexWithAppletOnScreen ( m_appletList.size() - 1 ) );
        location = m_appletList.size() - 1; // so the signal has the correct location
    } else
    {
        m_appletList.insert( location, applet );
        showAtIndex( minIndexWithAppletOnScreen ( location ) );
    }
    debug() << "emitting addApplet with location" << location;
    emit appletAdded( applet, location );
}


void
Context::VerticalAppletLayout::saveToConfig( KConfigGroup &conf )
{
    DEBUG_BLOCK
    QStringList plugins;

    for( int i = 0; i < m_appletList.size(); i++ )
    {
        Plasma::Applet *applet = m_appletList[ i ];
        if( applet != 0 )
        {
            debug() << "saving applet" << applet->name();
            plugins << applet->pluginName();
        }
        conf.writeEntry( "plugins", plugins );
    }
    conf.writeEntry( "firstShowingApplet", m_showingIndex );
}

void
Context::VerticalAppletLayout::refresh()
{
    showAtIndex( m_showingIndex );
}

QSizeF 
Context::VerticalAppletLayout::totalSize()
{
    QSizeF sizeR( boundingRect().width(), 0 );
    qreal size = 0.0;
    foreach( Plasma::Applet* applet, m_appletList )
        size += applet->effectiveSizeHint( Qt::PreferredSize, QSizeF( boundingRect().width(), -1 ) ).height();
    sizeR.setHeight( size );
    return sizeR;
}


void 
Context::VerticalAppletLayout::showApplet( Plasma::Applet* applet ) // SLOT
{
    showAtIndex( m_appletList.indexOf( applet ) );
}


void 
Context::VerticalAppletLayout::moveApplet( Plasma::Applet* applet, int oldLoc, int newLoc)
{
    DEBUG_BLOCK
    // if oldLoc is -1 we search for the applet to get the real location
    if( oldLoc == -1 )
        oldLoc = m_appletList.indexOf( applet );
    if( oldLoc == -1 )
        debug() << "COULDN'T FIND APPLET IN LIST!";
    
 //   debug() << "moving applet in layout from" << oldLoc << "to" << newLoc;
        
    if( oldLoc <  0 || oldLoc > m_appletList.size() - 1 || newLoc < 0 || newLoc > m_appletList.size() || oldLoc == newLoc )
        return;
    m_appletList.insert( newLoc, m_appletList.takeAt( oldLoc ) );
    showAtIndex( minIndexWithAppletOnScreen( qMin( oldLoc, newLoc ) ) );
}

void 
Context::VerticalAppletLayout::appletRemoved( Plasma::Applet* app )
{
    DEBUG_BLOCK
    int removedIndex = m_appletList.indexOf( app );
    debug() << "removing applet at index:" << removedIndex;
    m_appletList.removeAll( app );
    if( m_showingIndex > removedIndex )
        m_showingIndex--;
    showAtIndex( minIndexWithAppletOnScreen( m_showingIndex ) );

    debug() << "got " << m_appletList.size() << " applets left";
    if( m_appletList.size() == 0 )
        emit noApplets( true );
}

void
Context::VerticalAppletLayout::showAtIndex( int index )
{
    DEBUG_BLOCK
    if( index < 0 || index > m_appletList.size() )
        return;
    
    prepareGeometryChange();
    
    qreal runningHeight = 0.0, currentHeight = 0.0;
    qreal width =  boundingRect().width();
    //debug() << "showing applet at index" << index;
    //debug() << "using applet width of " << width;
    for( int i = index - 1; i >= 0; i-- ) // lay out backwards above the view
    {
        //debug() << "UPWARDS dealing with" << m_appletList[ i ]->name();
        currentHeight = m_appletList[ i ]->effectiveSizeHint( Qt::PreferredSize ).height();
        if( currentHeight < 15 )
            currentHeight = 250; // if it is one of the expanding applets, give it room for its header
        runningHeight -= currentHeight;
        m_appletList[ i ]->setPos( 0, runningHeight );
        //debug() << "UPWARDS putting applet #" << i << " at" << 0 << runningHeight;
        //debug() << "UPWARDS got applet sizehint height:" << currentHeight;
        m_appletList[ i ]->resize( width, currentHeight );
        m_appletList[ i ]->updateConstraints();
        m_appletList[ i ]->hide();
    }
    runningHeight = currentHeight = 0.0;

    /**
      * If an applet has a vertical sizeHint of < 0 (which means effectiveSizeHint < 15  ), then it means it wants to be laid out to maxmize vertical space.
      * Otherwise, give it the space it asks for.
      */
    //debug() << "total of" << m_appletList.size() << "applets";
    int lastShown = m_appletList.size();
    for( int i = index; i < lastShown; i++ ) // now lay out desired item at top and rest below it
    {
        //debug() << "dealing with" << m_appletList[ i ]->name();
        //debug() << "putting applet #" << i << " at" << 0 << runningHeight;
        m_appletList[ i ]->setPos( 0, runningHeight );
        qreal height = m_appletList[ i ]->effectiveSizeHint( Qt::PreferredSize ).height();
        //debug() << "applet has sizeHinte height of:" << height << "preferred  height:" << m_appletList[ i ]->preferredHeight() ;
        if( height < 15 ) // maximise its space
        {
            qreal heightLeft = boundingRect().height() - runningHeight;
            //debug() << "layout has boundingRect FLOWING" << boundingRect() ;
            m_appletList[ i ]->resize( width, heightLeft );
            m_appletList[ i ]->updateConstraints();
            m_appletList[ i ]->show();
            lastShown = i;
        } else
        {
            //debug() << "normal applet, moving on with the next one";
            runningHeight += height;
            m_appletList[ i ]->resize( width, height );
            m_appletList[ i ]->updateConstraints();
            m_appletList[ i ]->show();
            
            //debug() << "next applet will go at:" << runningHeight;
            //debug() << "got applet sizehint height:" << currentHeight;
        }
    }
    // hide the ones that we can't see below
    for( int i = lastShown + 1; i < m_appletList.size(); i++ )
    {
        //debug() << "HIDING NOT VISIBLE APPLET AT INDEX:" << i;
        // hiding an applet does not hide it's children
        // so in order to make sure that the applet is not visible,
        // in the case of misbehaving applets, we also move them out of the way. 
        m_appletList[ i ]->hide();
        m_appletList[ i ]->setPos( 0, boundingRect().height() );
    }
    
    m_showingIndex = index;
}

int 
Context::VerticalAppletLayout::minIndexWithAppletOnScreen( int loc )
{
    DEBUG_BLOCK
    qreal height = 0.0;
    int index = -1;
    if( boundingRect().height() < 30|| 
      ( m_appletList.size() == 0     || loc > m_appletList.size() - 1 ) ) // if we this small a height we are starting up and don't have a real size yet
        return 0;                      // for now just show all the applets 
    for( int i = loc; i >= 0; i-- )
    {    
        index = i;
        //debug() << "height:" << height;
        qreal curHeight = m_appletList[ i ]->effectiveSizeHint( Qt::PreferredSize, QSizeF( boundingRect().width(), -1 ) ).height();
        //debug() << "calculating:" << curHeight << " + " << height << " > " << boundingRect().height();
        if( ( curHeight + height ) > boundingRect().height() )
            break;
            
        height += curHeight;
    }
    return index;
}

#include "VerticalAppletLayout.moc"
