/***************************************************************************
                          juk.cpp  -  description
                             -------------------
    begin                : Mon Feb  4 23:40:41 EST 2002
    copyright            : (C) 2002 by Scott Wheeler
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

#include <klocale.h>
#include <kiconloader.h>
#include <kcmdlineargs.h>
#include <kstatusbar.h>
#include <kdebug.h>

#include <qinputdialog.h>

#include "juk.h"
#include "playlist.h"
#include "playlistsplitter.h"
#include "collectionlist.h"
#include "slideraction.h"
#include "cache.h"
#include "statuslabel.h"

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

JuK::JuK(QWidget *parent, const char *name) : KMainWindow(parent, name, WDestructiveClose)
{
    // Expect segfaults if you change this order.

    readSettings();
    setupLayout();
    setupActions();
    setupPlayer();
    readConfig();
    processArgs();
}

JuK::~JuK()
{
    delete(playTimer);
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void JuK::setupLayout()
{
    splitter = new PlaylistSplitter(this, restore, "playlistSplitter");
    setCentralWidget(splitter);

    // playlist item activation connection
    connect(splitter, SIGNAL(playlistDoubleClicked(QListViewItem *)), this, SLOT(playItem(QListViewItem *)));

    // create status bar
    statusLabel = new StatusLabel(statusBar());
    statusBar()->addWidget(statusLabel, 1);

    splitter->setFocus();
}

void JuK::setupActions()
{
    // file menu
    KStdAction::open(splitter, SLOT(open()), actionCollection());
    new KAction(i18n("Open &Directory..."), "fileopen", 0, splitter, SLOT(openDirectory()), actionCollection(), "openDirectory");
    KStdAction::save(splitter, SLOT(save()), actionCollection());
    new KAction(i18n("Delete"), "editdelete", 0, this, SLOT(remove()), actionCollection(), "remove");
    KStdAction::quit(this, SLOT(quit()), actionCollection());

    // edit menu
    KStdAction::cut(this, SLOT(cut()), actionCollection());
    KStdAction::copy(this, SLOT(copy()), actionCollection());
    KStdAction::paste(this, SLOT(paste()), actionCollection());
    KStdAction::selectAll(splitter, SLOT(selectAll()), actionCollection());

    // view menu
    showEditorAction = new KToggleAction(i18n("Show Tag Editor"), 0, actionCollection(), "showEditor");
    connect(showEditorAction, SIGNAL(toggled(bool)), splitter, SLOT(setEditorVisible(bool)));
    KStdAction::redisplay(splitter, SLOT(refresh()), actionCollection());

    // play menu
    randomPlayAction = new KToggleAction(i18n("Random Play"), 0, actionCollection(), "randomPlay");
    playAction = new KAction(i18n("&Play"), "player_play", 0, this, SLOT(playFile()), actionCollection(), "playFile");
    pauseAction = new KAction(i18n("P&ause"), "player_pause", 0, this, SLOT(pauseFile()), actionCollection(), "pauseFile");
    stopAction = new KAction(i18n("&Stop"), "player_stop", 0, this, SLOT(stopFile()), actionCollection(), "stopFile");
    backAction = new KAction(i18n("Skip &Back"), "player_start", 0, this, SLOT(backFile()), actionCollection(), "backFile");
    forwardAction = new KAction(i18n("Skip &Forward"), "player_end", 0, this, SLOT(forwardFile()), actionCollection(), "forwardFile");

    // playlist menu
    new KAction(i18n("New..."), "filenew", 0, splitter, SLOT(createPlaylist()), actionCollection(), "createPlaylist");
    new KAction(i18n("Open..."), "fileopen", 0, splitter, SLOT(openPlaylist()), actionCollection(), "openPlaylist");

    savePlaylistAction = new KAction(i18n("Save"), "filesave", 0, splitter, SLOT(savePlaylist()), actionCollection(), "savePlaylist");
    saveAsPlaylistAction = new KAction(i18n("Save As..."), "filesaveas", 0, splitter, SLOT(saveAsPlaylist()), 
				       actionCollection(), "saveAsPlaylist");
    renamePlaylistAction = new KAction(i18n("Rename..."), 0, splitter, SLOT(renamePlaylist()), 
				       actionCollection(), "renamePlaylist");
    new KAction(i18n("Duplicate..."), "editcopy", 0, splitter, SLOT(duplicatePlaylist()), actionCollection(), "duplicatePlaylist");
    deleteItemPlaylistAction = new KAction(i18n("Delete"), "editdelete", 0, splitter, SLOT(deleteItemPlaylist()), 
					   actionCollection(), "deleteItemPlaylist");
    
    // settings menu
    restoreOnLoadAction = new KToggleAction(i18n("Restored Playlists on Load"),  0, actionCollection(), "restoreOnLoad"); 

    playlistChanged(0);
    connect(splitter, SIGNAL(playlistChanged(Playlist *)), this, SLOT(playlistChanged(Playlist *)));


    // just in the toolbar
    sliderAction = new SliderAction(i18n("Track Position"), actionCollection(), "trackPositionAction");

    createGUI();

    // set the slider to the proper orientation and make it stay that way
    sliderAction->updateOrientation();
    connect(this, SIGNAL(dockWindowPositionChanged(QDockWindow *)), sliderAction, SLOT(updateOrientation(QDockWindow *)));
}

void JuK::setupPlayer()
{
    playingItem = 0;

    trackPositionDragging = false;
    noSeek = false;
    pauseAction->setEnabled(false);
    stopAction->setEnabled(false);
    backAction->setEnabled(false);
    forwardAction->setEnabled(false);

    playTimer = new QTimer(this);
    connect(playTimer, SIGNAL(timeout()), this, SLOT(pollPlay()));

    if(sliderAction && sliderAction->getTrackPositionSlider() && sliderAction->getVolumeSlider()) {
        connect(sliderAction->getTrackPositionSlider(), SIGNAL(valueChanged(int)), this, SLOT(trackPositionSliderUpdate(int)));
        connect(sliderAction->getTrackPositionSlider(), SIGNAL(sliderPressed()), this, SLOT(trackPositionSliderClick()));
        connect(sliderAction->getTrackPositionSlider(), SIGNAL(sliderReleased()), this, SLOT(trackPositionSliderRelease()));
        sliderAction->getTrackPositionSlider()->setEnabled(false);

        connect(sliderAction->getVolumeSlider(), SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
    }
}


void JuK::processArgs()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    QStringList files;
    
    for(int i = 0; i < args->count(); i++)
	files.append(args->arg(i));

    splitter->open(files);
}

void JuK::readConfig()
{
    // Automagically save and restore many window settings.
    setAutoSaveSettings();

    KConfig *config = KGlobal::config();
    { // player settings
        KConfigGroupSaver saver(config, "Player");
        if(sliderAction && sliderAction->getVolumeSlider()) {
            int volume = config->readNumEntry("Volume", sliderAction->getVolumeSlider()->maxValue());
            sliderAction->getVolumeSlider()->setValue(volume);
        }
	if(randomPlayAction) {
	    bool randomPlay = config->readBoolEntry("RandomPlay", false);
	    randomPlayAction->setChecked(randomPlay);
	}
    }
    { // view settings
        KConfigGroupSaver saver(config, "View");
	bool showEditor = config->readBoolEntry("ShowEditor", true);
	showEditorAction->setChecked(showEditor);
	splitter->setEditorVisible(showEditor);
    }

    if(restoreOnLoadAction)
	restoreOnLoadAction->setChecked(restore);
}

void JuK::readSettings()
{
    KConfig *config = KGlobal::config();
    { // general settings
        KConfigGroupSaver saver(config, "Settings");
	restore = config->readBoolEntry("RestoreOnLoad", true);
    }
}

void JuK::saveConfig()
{
    KConfig *config = KGlobal::config();
    { // player settings
        KConfigGroupSaver saver(config, "Player");
        if(sliderAction && sliderAction->getVolumeSlider())
            config->writeEntry("Volume", sliderAction->getVolumeSlider()->value());
	if(randomPlayAction)
	    config->writeEntry("RandomPlay", randomPlayAction->isChecked());
    }
    { // view settings
        KConfigGroupSaver saver(config, "View");
	config->writeEntry("ShowEditor", showEditorAction->isChecked());
    }
    { // general settings
        KConfigGroupSaver saver(config, "Settings");
	if(restoreOnLoadAction)
	    config->writeEntry("RestoreOnLoad", restoreOnLoadAction->isChecked());
    }
}

bool JuK::queryClose()
{
    Cache::instance()->save();
    saveConfig();
    delete(splitter);
    return(true);
}

////////////////////////////////////////////////////////////////////////////////
// private slot definitions
////////////////////////////////////////////////////////////////////////////////

void JuK::playlistChanged(Playlist *list)
{
    if(!list || list == CollectionList::instance()) {
	savePlaylistAction->setEnabled(false);
	saveAsPlaylistAction->setEnabled(false);
	renamePlaylistAction->setEnabled(false);
	deleteItemPlaylistAction->setEnabled(false);	
    }
    else {
	savePlaylistAction->setEnabled(true);
	saveAsPlaylistAction->setEnabled(true);
	renamePlaylistAction->setEnabled(true);
	deleteItemPlaylistAction->setEnabled(true);
    }
}

////////////////////////////////////////////////////////////////////////////////
// private action slot implementations - file menu
////////////////////////////////////////////////////////////////////////////////

void JuK::remove()
{
    PlaylistItemList items(splitter->playlistSelection());
    PlaylistItem *item = items.first();
    while(item) {
	if(item == playingItem)
	    playingItem = 0;
	item = items.next();
    }
    
    splitter->remove();
}

void JuK::quit()
{
    // delete(this);
    // kapp->quit();
    close();
}

////////////////////////////////////////////////////////////////////////////////
// edit menu
////////////////////////////////////////////////////////////////////////////////

void JuK::cut()
{
    PlaylistItemList items = splitter->playlistSelection();

    PlaylistItem *item = items.first();
    while(item) {
	if(item == playingItem)
	    playingItem = 0;
	item = items.next();
    }

    splitter->clearSelectedItems();
}

////////////////////////////////////////////////////////////////////////////////
// player menu
////////////////////////////////////////////////////////////////////////////////

void JuK::playFile()
{
    if(player.paused()) {
        player.play();

	// Here, before doing anything, we want to make sure that the player did
	// in fact start.

        if(player.playing()) {
            pauseAction->setEnabled(true);
            stopAction->setEnabled(true);
            playTimer->start(pollInterval);
        }
    }
    else if(player.playing())
	player.seekPosition(0);
    else if(splitter) {
        PlaylistItemList items = splitter->playlistSelection();
        if(items.count() > 0)
            playItem(items.at(0));
        else
            playItem(splitter->playlistFirstItem());
    }
}

void JuK::pauseFile()
{
    playTimer->stop();
    player.pause();
    pauseAction->setEnabled(false);
}

void JuK::stopFile()
{
    playTimer->stop();
    player.stop();
    pauseAction->setEnabled(false);
    stopAction->setEnabled(false);
    backAction->setEnabled(false);
    forwardAction->setEnabled(false);
    sliderAction->getTrackPositionSlider()->setValue(0);
    sliderAction->getTrackPositionSlider()->setEnabled(false);
    if(playingItem)
        playingItem->setPixmap(0, 0);
    playingItem = 0;

    statusLabel->clear();
}

void JuK::backFile()
{
    PlaylistItem *i = Playlist::previousItem(playingItem, randomPlayAction->isChecked());
    if(i)
	playItem(i);
}

void JuK::forwardFile()
{
    PlaylistItem *i = Playlist::nextItem(playingItem, randomPlayAction->isChecked());
    playItem(i);
}


////////////////////////////////////////////////////////////////////////////////
// additional player slots
////////////////////////////////////////////////////////////////////////////////

void JuK::trackPositionSliderClick()
{
    trackPositionDragging = true;
}

void JuK::trackPositionSliderRelease()
{
    trackPositionDragging = false;
    player.seekPosition(sliderAction->getTrackPositionSlider()->value());
}

void JuK::trackPositionSliderUpdate(int position)
{
    if(player.playing() && !trackPositionDragging && !noSeek)
        player.seekPosition(position);
}

void JuK::pollPlay()
{
    noSeek = true;
    if(!player.playing()) {

        playTimer->stop();

        if(player.paused())
            pauseFile();
        else if(playingItem) {

	    PlaylistItem *next = Playlist::nextItem(playingItem, randomPlayAction->isChecked());
	    playingItem->setPixmap(0, 0);

	    if(next) {
		playingItem = next;
		
		backAction->setEnabled(true);
		
		sliderAction->getTrackPositionSlider()->setValue(0);
		player.play(playingItem->absFilePath(), player.getVolume());
		if(player.playing()) {
		    playTimer->start(pollInterval);
		    playingItem->setPixmap(0, QPixmap(UserIcon("playing")));
		}
	    }
	    statusLabel->setPlayingItem(playingItem);
	}
	else
	    stopFile();
    }
    else if(!trackPositionDragging) {
        sliderAction->getTrackPositionSlider()->setValue(player.position());
	statusLabel->setItemTotalTime(player.totalTime());
	statusLabel->setItemCurrentTime(player.currentTime());
    }

    // Ok, this is weird stuff, but it works pretty well.  Ordinarily we don't
    // need to check up on our playing time very often, but in the span of the 
    // last interval, we want to check a lot -- to figure out that we've hit the
    // end of the song as soon as possible.

    if(player.playing() && float(player.totalTime() - player.currentTime()) < pollInterval * 2)
        playTimer->changeInterval(50);

    noSeek = false;
}

void JuK::setVolume(int volume)
{
    if(sliderAction && sliderAction->getVolumeSlider() &&
       sliderAction->getVolumeSlider()->maxValue() > 0 &&
       volume >= 0 && sliderAction->getVolumeSlider()->maxValue() >= volume)
    {
        player.setVolume(float(volume) / float(sliderAction->getVolumeSlider()->maxValue()));
    }
}

void JuK::playItem(QListViewItem *item)
{
    playItem(dynamic_cast<PlaylistItem *>(item));
}

void JuK::playItem(PlaylistItem *item)
{
    if(player.playing() || player.paused())
        stopFile();

    if(item) {
        float volume = float(sliderAction->getVolumeSlider()->value()) / float(sliderAction->getVolumeSlider()->maxValue());
        player.play(item->absFilePath(), volume);

	// Make sure that the player actually starts before doing anything.

        if(player.playing()) {
            pauseAction->setEnabled(true);
            stopAction->setEnabled(true);

	    backAction->setEnabled(true);
	    forwardAction->setEnabled(true);

	    playingItem = item;

            sliderAction->getTrackPositionSlider()->setEnabled(true);
            item->setPixmap(0, QPixmap(UserIcon("playing")));
            playTimer->start(pollInterval);

	    statusLabel->setPlayingItem(item);
        }
    }
}

#include "juk.moc"
