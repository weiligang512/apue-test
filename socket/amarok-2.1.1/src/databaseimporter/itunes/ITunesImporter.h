/*******************************************************************************
* copyright              : (C) 2008 Seb Ruiz <ruiz@kde.org>                    *
* copyright              : (C) 2008 Leo Franchi <lfranchi@kde.org>             *
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/


#ifndef AMAROK_ITUNES_IMPORTER_H
#define AMAROK_ITUNES_IMPORTER_H

#include "databaseimporter/DatabaseImporter.h"
#include "ITunesImporterConfig.h"
#include "ITunesImporterWorker.h"

class ITunesImporter : public DatabaseImporter
{
    Q_OBJECT

    public:
        ITunesImporter( QObject *parent );
        virtual ~ITunesImporter();

        static QString name() { return QString("iTunes"); }
        
        virtual DatabaseImporterConfig *configWidget( QWidget *parent );
        virtual bool canImportArtwork() const { return false; }
    private slots:
        void finishUp();

    protected:
        virtual void import();
        
    private:
        ITunesImporterConfig *m_config;
        ITunesImporterWorker *m_worker;
};

#endif // multiple inclusion guard
