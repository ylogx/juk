/***************************************************************************
    begin                : Sun Feb 17 2002
    copyright            : (C) 2002 - 2004 by Scott Wheeler
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

#include <config.h>

#include <kdebug.h>
#include <kaction.h>
#include <kiconloader.h>

#include "playlistitem.h"
#include "collectionlist.h"
#include "musicbrainzquery.h"
#include "tag.h"
#include "actioncollection.h"
#include "ktrm.h"
#include "coverinfo.h"
#include "tagtransactionmanager.h"

static void startMusicBrainzQuery(const FileHandle &file)
{
#if HAVE_MUSICBRAINZ
    new MusicBrainzLookup(file);
#else
    Q_UNUSED(file)
#endif
}

////////////////////////////////////////////////////////////////////////////////
// PlaylistItem public methods
////////////////////////////////////////////////////////////////////////////////

PlaylistItem::~PlaylistItem()
{
    // Although this isn't the most efficient way to accomplish the task of
    // stopping playback when deleting the item being played, it has the
    // stark advantage of working reliably.  I'll tell anyone who tries to
    // optimize this, the timing issues can be *hard*. -- mpyne

    if(playlist()->playingItem() == this)
	playlist()->setPlaying(0);

    m_collectionItem->removeChildItem(this);
}

void PlaylistItem::setFile(const FileHandle &file)
{
    m_collectionItem->updateCollectionDict(d->fileHandle.absFilePath(), file.absFilePath());
    d->fileHandle = file;
    refresh();
}

void PlaylistItem::setFile(const QString &file)
{
    QString oldPath = d->fileHandle.absFilePath();
    d->fileHandle.setFile(file);
    m_collectionItem->updateCollectionDict(oldPath, d->fileHandle.absFilePath());
    refresh();
}

FileHandle PlaylistItem::file() const
{
    return d->fileHandle;
}

QPixmap *PlaylistItem::pixmap(int column) const
{
    int offset = playlist()->columnOffset();

    if ((column - offset) == CoverColumn && d->fileHandle.coverInfo()->hasCover())
        return new QPixmap(SmallIcon("image"));

    return 0;
}

QString PlaylistItem::text(int column) const
{
    if(!d->fileHandle.tag())
	return QString::null;

    int offset = playlist()->columnOffset();

    switch(column - offset) {
    case TrackColumn:
	return d->fileHandle.tag()->title();
    case ArtistColumn:
	return d->fileHandle.tag()->artist();
    case AlbumColumn:
	return d->fileHandle.tag()->album();
    case CoverColumn:
	return QString::null;
    case TrackNumberColumn:
	return d->fileHandle.tag()->track() > 0
	    ? QString::number(d->fileHandle.tag()->track())
	    : QString::null;
    case GenreColumn:
	return d->fileHandle.tag()->genre();
    case YearColumn:
	return d->fileHandle.tag()->year() > 0 
	    ? QString::number(d->fileHandle.tag()->year())
	    : QString::null;
    case LengthColumn:
	return d->fileHandle.tag()->lengthString();
    case BitrateColumn:
	return QString::number(d->fileHandle.tag()->bitrate());
    case CommentColumn:
	return d->fileHandle.tag()->comment();
    case FileNameColumn:
	return d->fileHandle.fileInfo().fileName();
    case FullPathColumn:
	return d->fileHandle.fileInfo().absFilePath();
    default:
	return KListViewItem::text(column);
    }
}

void PlaylistItem::setText(int column, const QString &text)
{
    int offset = playlist()->columnOffset();
    if(column - offset >= 0 && column + offset <= lastColumn()) {
	KListViewItem::setText(column, QString::null);
	return;
    }

    KListViewItem::setText(column, text);
    playlist()->slotWeightDirty(column);
}

void PlaylistItem::setSelected(bool selected)
{
    playlist()->markItemSelected(this, selected);
    KListViewItem::setSelected(selected);
}

void PlaylistItem::guessTagInfo(TagGuesser::Type type)
{
    switch(type) {
    case TagGuesser::FileName:
    {
	TagGuesser guesser(d->fileHandle.absFilePath());
	Tag *tag = TagTransactionManager::duplicateTag(d->fileHandle.tag());

	if(!guesser.title().isNull())
	    tag->setTitle(guesser.title());
	if(!guesser.artist().isNull())
	    tag->setArtist(guesser.artist());
	if(!guesser.album().isNull())
	    tag->setAlbum(guesser.album());
	if(!guesser.track().isNull())
	    tag->setTrack(guesser.track().toInt());
	if(!guesser.comment().isNull())
	    tag->setComment(guesser.comment());

	TagTransactionManager::instance()->changeTagOnItem(this, tag);
	break;
    }
    case TagGuesser::MusicBrainz:
	startMusicBrainzQuery(d->fileHandle);
	break;
    }
}

Playlist *PlaylistItem::playlist() const
{
    return static_cast<Playlist *>(listView());
}

QValueVector<int> PlaylistItem::cachedWidths() const
{
    return d->cachedWidths;
}

void PlaylistItem::refresh()
{
    m_collectionItem->refresh();
}

void PlaylistItem::refreshFromDisk()
{
    d->fileHandle.refresh();
    refresh();
}

void PlaylistItem::clear()
{
    playlist()->clearItem(this);
}

////////////////////////////////////////////////////////////////////////////////
// PlaylistItem protected methods
////////////////////////////////////////////////////////////////////////////////

PlaylistItem::PlaylistItem(CollectionListItem *item, Playlist *parent) :
    KListViewItem(parent),
    d(0),
    m_playing(false)
{
    setup(item);
}

PlaylistItem::PlaylistItem(CollectionListItem *item, Playlist *parent, QListViewItem *after) :
    KListViewItem(parent, after),
    d(0),
    m_playing(false)
{
    setup(item);
}


// This constructor should only be used by the CollectionList subclass.

PlaylistItem::PlaylistItem(CollectionList *parent) :
    KListViewItem(parent),
    m_playing(false)
{
    d = new Data;
    m_collectionItem = static_cast<CollectionListItem *>(this);
    setDragEnabled(true);
}

void PlaylistItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align)
{
    if(!m_playing)
	return KListViewItem::paintCell(p, cg, column, width, align);

    QColorGroup colorGroup = cg;

    QColor base = colorGroup.base();
    QColor selection = colorGroup.highlight();

    int r = (base.red() + selection.red()) / 2;
    int b = (base.blue() + selection.blue()) / 2;
    int g = (base.green() + selection.green()) / 2;

    QColor c(r, g, b);

    colorGroup.setColor(QColorGroup::Base, c);
    QListViewItem::paintCell(p, colorGroup, column, width, align);
}

int PlaylistItem::compare(QListViewItem *item, int column, bool ascending) const
{
    // reimplemented from QListViewItem

    int offset = playlist()->columnOffset();

    if(!item)
	return 0;

    PlaylistItem *playlistItem = static_cast<PlaylistItem *>(item);

    // The following statments first check to see if you can sort based on the
    // specified column.  If the values for the two PlaylistItems are the same
    // in that column it then trys to sort based on columns 1, 2, 3 and 0,
    // (artist, album, track number, track name) in that order.

    int c = compare(this, playlistItem, column, ascending);

    if(c != 0)
	return c;
    else {
	// Loop through the columns doing comparisons until something is differnt.
	// If all else is the same, compare the track name.

	int last = playlist()->isColumnVisible(AlbumColumn + offset) ? TrackNumberColumn : ArtistColumn;

	for(int i = ArtistColumn; i <= last; i++) {
	    if(playlist()->isColumnVisible(i + offset)) {
		c = compare(this, playlistItem, i, ascending);
		if(c != 0)
		    return c;
	    }
	}
	return compare(this, playlistItem, TrackColumn + offset, ascending);
    }
}

int PlaylistItem::compare(const PlaylistItem *firstItem, const PlaylistItem *secondItem, int column, bool) const
{
    int offset = playlist()->columnOffset();

    if(column < 0 || column > lastColumn() + offset)
	return 0;

    if(column < offset) {
	QString first = firstItem->text(column).lower();
	QString second = secondItem->text(column).lower();
	return first.localeAwareCompare(second);
    }

    switch(column - offset) {
    case TrackNumberColumn:
        if(firstItem->d->fileHandle.tag()->track() > secondItem->d->fileHandle.tag()->track())
            return 1;
        else if(firstItem->d->fileHandle.tag()->track() < secondItem->d->fileHandle.tag()->track())
            return -1;
        else
            return 0;
	break;
    case LengthColumn:
        if(firstItem->d->fileHandle.tag()->seconds() > secondItem->d->fileHandle.tag()->seconds())
            return 1;
        else if(firstItem->d->fileHandle.tag()->seconds() < secondItem->d->fileHandle.tag()->seconds())
            return -1;
        else
            return 0;
	break;
    case BitrateColumn:
        if(firstItem->d->fileHandle.tag()->bitrate() > secondItem->d->fileHandle.tag()->bitrate())
            return 1;
        else if(firstItem->d->fileHandle.tag()->bitrate() < secondItem->d->fileHandle.tag()->bitrate())
            return -1;
        else
            return 0;
	break;
    default:
	return strcoll(firstItem->d->local8Bit[column - offset],
		       secondItem->d->local8Bit[column - offset]);
    }
}

bool PlaylistItem::isValid() const
{
    return bool(d->fileHandle.tag());
}

////////////////////////////////////////////////////////////////////////////////
// PlaylistItem private methods
////////////////////////////////////////////////////////////////////////////////

void PlaylistItem::setup(CollectionListItem *item)
{
    m_collectionItem = item;

    d = item->d;
    item->addChildItem(this);
    setDragEnabled(true);
}
