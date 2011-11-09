/*
    Copyright (c) 2008 Daniel Jones <danielcjones@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DYNAMICCATEGORY_H
#define DYNAMICCATEGORY_H

#include <QCheckBox>
#include <QListView>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

#include <KComboBox>

namespace PlaylistBrowserNS {

    class DynamicBiasModel;
    class DynamicBiasDelegate;

    /**
    */
    class DynamicCategory : public QWidget
    {
        Q_OBJECT
        public:
            DynamicCategory( QWidget* parent );
            ~DynamicCategory();

        protected slots:
            void newPalette( const QPalette & palette );

        private slots:
            void OnOff(bool);
            void enableDynamicMode( bool enable );
            void playlistSelectionChanged( int index );
            void save();
            void playlistCleared();
            
        private:
            void On();
            void Off();

            QVBoxLayout* m_vLayout;

            QCheckBox *m_onOffCheckbox;
            QPushButton *m_repopulateButton;

            KComboBox *m_presetComboBox;

            QToolButton *m_saveButton;
            QToolButton *m_deleteButton;
            QListView   *m_biasListView;
            DynamicBiasModel *m_biasModel;
            DynamicBiasDelegate *m_biasDelegate;
    };

}

#endif
