/*  This file is part of the KDE project
    Copyright (C) 2009 Patrick Spendrin <ps_ml@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef MEDIADEVICECOLLECTION_EXPORT_H
#define MEDIADEVICECOLLECTION_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef MEDIADEVICECOLLECTION_EXPORT
# if defined(MAKE_MEDIADEVICELIB_LIB)
   /* We are building this library */ 
#  define MEDIADEVICECOLLECTION_EXPORT KDE_EXPORT
# else
   /* We are using this library */ 
#  define MEDIADEVICECOLLECTION_EXPORT KDE_IMPORT
# endif
#endif

#endif // MEDIADEVICECOLLECTION_EXPORT_H
