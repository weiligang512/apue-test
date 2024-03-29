/***************************************************************************
begin                : 2004/03/12
copyright            : (C) Mark Kretschmann
email                : markey@web.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AMAROK_PLUGINMANAGER_H
#define AMAROK_PLUGINMANAGER_H

#include <KService>
#include <KServiceTypeTrader>

#include <vector>

namespace Amarok { class Plugin; }
class KLibrary;

class PluginManager
{
    public:
        /** Bump this number whenever the plugin framework gets incompatible with older versions */
        static const int FrameworkVersion = 43;

        /**
         * It will return a list of services that match your
         * specifications.  The only required parameter is the service
         * type.  This is something like 'text/plain' or 'text/html'.  The
         * constraint parameter is used to limit the possible choices
         * returned based on the constraints you give it.
         *
         * The @p constraint language is rather full.  The most common
         * keywords are AND, OR, NOT, IN, and EXIST, all used in an
         * almost spoken-word form.  An example is:
         * \code
         * (Type == 'Service') and (('KParts/ReadOnlyPart' in ServiceTypes) or (exist Exec))
         * \endcode
         *
         * The keys used in the query (Type, ServiceType, Exec) are all
         * fields found in the .desktop files.
         *
         * @param constraint  A constraint to limit the choices returned, QString::null to
         *                    get all services of the given @p servicetype
         *
         * @return            A list of services that satisfy the query
         * @see               http://developer.kde.org/documentation/library/kdeqt/tradersyntax.html
         */
        static KService::List query( const QString& constraint = QString() );

        /**
         * Load and instantiate plugin from query
         * @param constraint  A constraint to limit the choices returned, QString::null to
         *                    get all services of the given @p servicetype
         * @return            Pointer to Plugin, or NULL if error
         * @see               http://developer.kde.org/documentation/library/kdeqt/tradersyntax.html
         */
        static Amarok::Plugin* createFromQuery( const QString& constraint = QString() );

        /**
         * Load and instantiate plugin from service
         * @param service     Pointer to KService
         * @return            Pointer to Plugin, or NULL if error
         */
        static Amarok::Plugin* createFromService( const KService::Ptr service );

        /**
         * Remove library and delete plugin
         * @param plugin      Pointer to plugin
         */
        static void unload( Amarok::Plugin* plugin );

        /**
         * Look up service for loaded plugin from store
         * @param pointer     Pointer to plugin
         * @return            KService, or 0 if not found
         */
        static KService::Ptr getService( const Amarok::Plugin* plugin );

        /**
         * Dump properties from a service to stdout for debugging
         * @param service     Pointer to KService
         */
        static void dump( const KService::Ptr service );

       /**
         * Show modal info dialog about plugin
         * @param constraint  A constraint to limit the choices returned
         */
        static void showAbout( const QString& constraint );

    private:
        struct StoreItem {
            Amarok::Plugin* plugin;
            KLibrary* library;
            KService::Ptr service;
        };

        static std::vector<StoreItem>::iterator lookupPlugin( const Amarok::Plugin* plugin );

    //attributes:
        static std::vector<StoreItem> m_store;
};


#endif /* AMAROK_PLUGINMANAGER_H */

