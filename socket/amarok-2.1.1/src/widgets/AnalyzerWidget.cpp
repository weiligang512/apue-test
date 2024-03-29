/***************************************************************************
 * copyright     : (C) 2004 Mark Kretschmann <markey@web.de>               *
                   (C) 2007 Dan Meltzer <parallelgrapefruit@gmail.com>   *
 **************************************************************************/

 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//

#include "AnalyzerWidget.h"

#include "Amarok.h"
#include "amarokconfig.h"
#include "Debug.h"
#include "analyzerbase.h"
#include "socketserver.h"

#include <KIcon>
#include <KLocale>
#include <KMenu>

AnalyzerWidget::AnalyzerWidget( QWidget *parent )
    : QWidget( parent )
    , m_child( 0 )
{
    setObjectName(  "AnalyzerWidget" );
    setToolTip( i18n( "Click for more analyzers" ) );
    setContentsMargins(0,0,0,0);
    changeAnalyzer();
}

void
AnalyzerWidget::resizeEvent( QResizeEvent *)
{
    m_child->resize( size() );
}

void AnalyzerWidget::changeAnalyzer()
{
    delete m_child;
    m_child = Analyzer::Factory::createPlaylistAnalyzer( this );
    m_child->setObjectName( "ToolBarAnalyzer" );
    m_child->resize( size() );
    m_child->show();
}

void
AnalyzerWidget::mousePressEvent( QMouseEvent *e)
{
    if( e->button() == Qt::LeftButton ) {
        AmarokConfig::setCurrentAnalyzer( AmarokConfig::currentAnalyzer() + 1 );
        changeAnalyzer();
    }
}

void
AnalyzerWidget::contextMenuEvent( QContextMenuEvent *e)
{
#if defined HAVE_LIBVISUAL
    KMenu menu;
    menu.addAction( KIcon( "view-media-visualization-amarok" ), i18n("&Visualizations"),
                           Vis::Selector::instance(), SLOT(show()) );

    menu.exec( mapToGlobal( e->pos() ) );
#else
    Q_UNUSED(e);
#endif
}

#include "AnalyzerWidget.moc"
