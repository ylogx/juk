/***************************************************************************
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

#ifndef PLAYLIST_COLLECTION_H
#define PLAYLIST_COLLECTION_H

#include "playlistinterface.h"
#include "stringhash.h"

#include <kshortcut.h>
#include <kdirwatch.h>
#include <kconfig.h>
#include <klocale.h>

#include <qobject.h>
#include <qstringlist.h>
#include <qsignal.h>

class QWidgetStack;
class KAction;
class Playlist;
class PlaylistItem;

typedef QValueList<PlaylistItem *> PlaylistItemList;

class PlaylistCollection : public PlaylistInterface
{
    friend class Playlist;
    friend class CollectionList;

public:
    PlaylistCollection(QWidgetStack *playlistStack);
    virtual ~PlaylistCollection();

    virtual QString name() const;
    virtual FileHandle currentFile() const;
    virtual int count() const;
    virtual int time() const;
    virtual void playNext();
    virtual void playPrevious();
    virtual void stop();

    void open(const QStringList &files = QStringList());
    void addFolder();
    void rename();
    void duplicate();
    void save();
    void saveAs();
    void remove();
    void reload();
    void editSearch();

    void removeItems();
    void refreshItems();
    void renameItems();

    PlaylistItemList selectedItems();

    void scanFolders();

    void createPlaylist();
    void createSearchPlaylist();
    void createFolderPlaylist();

    void guessTagFromFile();
    void guessTagFromInternet();

    void setSearchEnabled(bool enable);

    QObject *object() const;

    class ActionHandler;

protected:
    virtual Playlist *currentPlaylist() const;
    virtual QWidgetStack *playlistStack() const;
    virtual void raise(Playlist *playlist);
    virtual void setupPlaylist(Playlist *playlist, const QString &iconName);

    bool importPlaylists() const;

    QString uniquePlaylistName(const QString &suggest = i18n("Playlist"));

private:
    void readConfig();
    void saveConfig();

    QWidgetStack  *m_playlistStack;
    ActionHandler *m_actionHandler;

    KDirWatch   m_dirWatch;
    QStringList m_playlistNames;
    StringHash  m_playlistFiles;
    QStringList m_folderList;
    bool        m_importPlaylists;
    bool        m_restore;
    bool        m_searchEnabled;
};

/**
 * This class is just used as a proxy to handle the signals coming from action
 * activations without requiring PlaylistCollection to be a QObject.
 */

class PlaylistCollection::ActionHandler : public QObject
{
    Q_OBJECT
public:
    ActionHandler(PlaylistCollection *collection);

private:
    KAction *createAction(const QString &text,
                          const char *slot,
                          const char *name,
                          const QString &icon = QString::null,
                          const KShortcut &shortcut = KShortcut());
private slots:
    void slotOpen()         { m_collection->open(); }
    void slotAddFolder()    { m_collection->addFolder(); }
    void slotRename()       { m_collection->rename(); }
    void slotDuplicate()    { m_collection->duplicate(); }
    void slotSave()         { m_collection->save(); }
    void slotSaveAs()       { m_collection->saveAs(); }
    void slotReload()       { m_collection->reload(); }
    void slotRemove()       { m_collection->remove(); }
    void slotEditSearch()   { m_collection->editSearch(); }

    void slotRemoveItems()  { m_collection->removeItems(); }
    void slotRefreshItems() { m_collection->refreshItems(); }
    void slotRenameItems()  { m_collection->renameItems(); }

    void slotScanFolders()  { m_collection->scanFolders(); }

    void slotCreatePlaylist()       { m_collection->createPlaylist(); }
    void slotCreateSearchPlaylist() { m_collection->createSearchPlaylist(); }
    void slotCreateFolderPlaylist() { m_collection->createFolderPlaylist(); }

    void slotGuessTagFromFile()     { m_collection->guessTagFromFile(); }
    void slotGuessTagFromInternet() { m_collection->guessTagFromInternet(); }

    void slotSetSearchEnabled(bool enable) { m_collection->setSearchEnabled(enable); }

signals:
    void signalSelectedItemsChanged();

private:
    PlaylistCollection *m_collection;
};

#endif