/***************************************************************************
                          searchplaylist.h
                             -------------------
    begin                : Mon May 5 2003
    copyright            : (C) 2003 by Scott Wheeler
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

#include <kdebug.h>

#include <qptrdict.h>

#include "searchplaylist.h"
#include "playlistitem.h"
#include "collectionlist.h"

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

SearchPlaylist::SearchPlaylist(QWidget *parent, const PlaylistSearch &search, const QString &name) :
    DynamicPlaylist(search.playlists(), parent, name),
    m_search(search)
{

}

////////////////////////////////////////////////////////////////////////////////
// protected methods
////////////////////////////////////////////////////////////////////////////////

void SearchPlaylist::updateItems()
{
    // Here we don't simply use "clear" since that would involve a call to
    // items() which would in turn call this method...

    PlaylistItemList l = Playlist::items();

    QPtrDict<PlaylistItem> oldItems(503);

    for(PlaylistItemList::ConstIterator it = l.begin(); it != l.end(); ++it)
        oldItems.insert((*it)->collectionItem(), *it);

    m_search.search();
    PlaylistItemList matched = m_search.matchedItems();
    PlaylistItemList newItems;

    for(PlaylistItemList::ConstIterator it = matched.begin(); it != matched.end(); ++it) {
        if(!oldItems.remove((*it)->collectionItem()))
            newItems.append((*it)->collectionItem());
    }

    // kdDebug(65432) << k_funcinfo << "newItems.size() == " << newItems.size() << endl;

    for(QPtrDictIterator<PlaylistItem> it(oldItems); it.current(); ++it)
        clearItem(it.current(), false);

    if(!oldItems.isEmpty() && newItems.isEmpty())
        emit signalCountChanged(this);

    createItems(newItems);
}


////////////////////////////////////////////////////////////////////////////////
// helper functions
////////////////////////////////////////////////////////////////////////////////

QDataStream &operator<<(QDataStream &s, const SearchPlaylist &p)
{
    s << p.name()
      << p.playlistSearch();

    return s;
}

QDataStream &operator>>(QDataStream &s, SearchPlaylist &p)
{
    QString name;
    PlaylistSearch search;

    s >> name
      >> search;

    p.setName(name);
    p.setPlaylistSearch(search);

    return s;
}

#include "searchplaylist.moc"
