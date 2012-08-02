#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QTreeView>

#include "playlist/playlists/playlist.h"

class PlaylistView : public QTreeView
{
    Q_OBJECT
public:
    PlaylistView(QWidget *parent);
    
private:
    Playlist *playlist() { return qobject_cast<Playlist*>(model()); }
    
private slots:
    void slotPopulateBackMenu() const;
    void slotPlayFromBackMenu(QAction *) const;
};

#endif//PLAYLISTVIEW_H