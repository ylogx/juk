/***************************************************************************
                          actioncollection.cpp
                             -------------------
    begin                : Fri Feb 27 2004
    copyright            : (C) 2004 by Scott Wheeler
    email                : wheeler@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kactioncollection.h>

#include "actioncollection.h"

namespace ActionCollection
{
    KActionCollection *actions()
    {
        static KActionCollection *a =
            new KActionCollection(static_cast<QWidget *>(0), "JuK Action Collection");
        return a;
    }

    KAction *action(const char *key)
    {
        return actions()->action(key);
    }
}
