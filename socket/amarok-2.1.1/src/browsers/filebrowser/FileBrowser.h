/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2007 Mirko Stocker <me@misto.ch>
   Copyright (C) 2007 Ian Monroe <ian@monroe.nu>
   Copyright (C) 2008-2009 Mark Kretschmann <kretschmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <KUrl>
#include <KVBox>

class KActionCollection;
class KActionSelector;
class KBookmarkHandler;
class KConfigBase;
class KFilePlacesModel;
class KHistoryComboBox;
class KToolBar;
class KUrlNavigator;

class QAbstractItemView;
class QEvent;
class QFocusEvent;
class QToolButton;

class MyDirOperator;

/*
    The Amarok Files browser presents a directory view, in which the default action is
    to open the activated file.
    Additionally, a toolbar for managing the kdiroperator widget + sync that to
    the directory of the current file is available, as well as a filter widget
    allowing to filter the displayed files using a name filter.
*/

namespace FileBrowser
{

class Widget : public KVBox
{
    Q_OBJECT

public:
    explicit Widget( const char * name, QWidget *parent );
    ~Widget();

    void setupToolbar();
    MyDirOperator *dirOperator() const { return m_dirOperator; }
    KActionCollection *actionCollection() const { return m_actionCollection; }

public Q_SLOTS:
    void slotFilterChange( const QString& );
    void setDir( const KUrl& url );
    void selectorViewChanged( QAbstractItemView * );

private Q_SLOTS:
    void dirUrlEntered( const KUrl& u );
    void filterButtonClicked();

protected:
    void focusInEvent( QFocusEvent * );
    bool eventFilter( QObject *, QEvent * );
    void initialDirChangeHack();

private:
    void readConfig();
    void writeConfig();

    KToolBar          *m_toolbar;
    KActionCollection *m_actionCollection;
    KBookmarkHandler  *m_bookmarkHandler;
    KUrlNavigator     *m_urlNav;
    KFilePlacesModel  *m_filePlacesModel;
    MyDirOperator     *m_dirOperator;
    KHistoryComboBox  *m_filter;
    QToolButton       *m_filterButton;

    QString            m_lastFilter;
};

}

#endif //__KATE_FILESELECTOR_H__

