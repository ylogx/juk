/***************************************************************************
                          musicbrainzitem.h
                             -------------------
    begin                : Thur Sep 04 2003
    copyright            : (C) 2003 by Adam Treat
    email                : manyoso@yahoo.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MUSICBRAINZITEM_H
#define MUSICBRAINZITEM_H

#include <config.h>

#if HAVE_MUSICBRAINZ

#include <klistview.h>

#include "musicbrainzquery.h"

/**
 * Items for the MusicBrainz queries.
 */

class MusicBrainzItem : public KListViewItem
{
public:
    MusicBrainzItem(KListView *parent, const MusicBrainzQuery::Track &track,
                    const QString &name, const QString &artist,
                    const QString &album);

    virtual ~MusicBrainzItem();

    MusicBrainzQuery::Track track() const { return m_track; }

private:
    MusicBrainzQuery::Track m_track;
};

#endif // HAVE_MUSICBRAINZ

#endif