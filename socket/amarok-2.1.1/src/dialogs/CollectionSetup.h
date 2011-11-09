/***************************************************************************
   begin                : Tue Feb 4 2003
   copyright            : (C) 2003 Scott Wheeler <wheeler@kde.org>
                        : (C) 2004 Max Howell <max.howell@methylblue.com>
                        : (C) 2004-2008 Mark Kretschmann <kretschmann@kde.org>
                        : (C) 2008 Seb Ruiz <ruiz@kde.org>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AMAROK_COLLECTIONSETUP_H
#define AMAROK_COLLECTIONSETUP_H

#include <KVBox>      //baseclass

#include <QCheckBox>
#include <QFileSystemModel>
#include <QTreeWidgetItem>

#include "Debug.h"

class QTreeView;

namespace CollectionFolder { class Model; }

class CollectionSetup : public KVBox
{
    Q_OBJECT

    friend class CollectionFolder::Model;

    public:
        static CollectionSetup* instance() { return s_instance; }

        CollectionSetup( QWidget* );
        
        void writeConfig();
        bool hasChanged() const;
         
        QStringList dirs() const { return m_dirs; }
        bool recursive() const { return m_recursive && m_recursive->isChecked(); }
        bool monitor() const { return m_monitor && m_monitor->isChecked(); }

    signals:
        void changed();

    private slots:
        void importCollection();

    private:
        static CollectionSetup* s_instance;

        QTreeView *m_view;
        CollectionFolder::Model *m_model;
        QStringList m_dirs;
        QCheckBox *m_recursive;
        QCheckBox *m_monitor;
};


namespace CollectionFolder //just to keep it out of the global namespace
{
    class Model : public QFileSystemModel
    {
        public:
            Model();
        
            virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
            QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
            bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

            void setDirectories( QStringList &dirs ); // will clear m_checked before inserting new directories
            QStringList directories() const;

            virtual int columnCount( const QModelIndex& ) const { return 1; }

        private:
            bool ancestorChecked( const QString &path ) const;
            bool descendantChecked( const QString &path ) const;
            bool isForbiddenPath( const QString &path ) const;
            bool recursive() const { return CollectionSetup::instance() && CollectionSetup::instance()->recursive(); } // simply for convenience
            QSet<QString> m_checked;
    };

} // end namespace CollectionFolder

#endif

