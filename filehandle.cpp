/***************************************************************************
                          filehandle.cpp
                             -------------------
    begin                : Sun Feb 29 2004
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

#include <limits.h>
#include <stdlib.h>

#include <kdebug.h>

#include <qfileinfo.h>

#include "filehandle.h"
#include "tag.h"
#include "cache.h"

static QString resolveSymLinks(const QFileInfo &file) // static
{
    char real[PATH_MAX];

    if(file.exists() && realpath(QFile::encodeName(file.absFilePath()).data(), real))
	return QFile::decodeName(real);
    else
	return file.filePath();
}


/**
 * A simple reference counter -- pasted from TagLib.
 */

class RefCounter
{
public:
    RefCounter() : refCount(1) {}
    void ref() { refCount++; }
    bool deref() { return ! --refCount ; }
    int count() const { return refCount; }
private:
    uint refCount;
};

class FileHandle::FileHandlePrivate : public RefCounter
{
public:
    FileHandlePrivate() :
        tag(0) {}

    mutable Tag *tag;
    mutable QString absFilePath;
    QFileInfo fileInfo;
    QDateTime modificationTime;
    QDateTime lastModified;
};

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

FileHandle::FileHandle()
{
    static FileHandlePrivate *nullPrivate = new FileHandlePrivate;
    d = nullPrivate;
    d->ref();
}

FileHandle::FileHandle(const FileHandle &f) :
    d(f.d)
{
    d->ref();
}

FileHandle::FileHandle(const QFileInfo &info, const QString &path) :
    d(0)
{
    setup(info, path);
}

FileHandle::FileHandle(const QString &path) :
    d(0)
{
    setup(QFileInfo(path), path);
}

FileHandle::~FileHandle()
{
    if(d->deref())
        delete d;
}

void FileHandle::refresh()
{
    d->fileInfo.refresh();
    delete d->tag;
    d->tag = new Tag(d->absFilePath);
}

void FileHandle::setFile(const QString &path)
{
    if(!d || isNull())
        setup(QFileInfo(path), path);
    else {
        d->absFilePath = path;
        d->fileInfo.setFile(path);
    }
}

Tag *FileHandle::tag() const
{
    if(!d->tag)
        d->tag = new Tag(d->absFilePath);

    return d->tag;
}

QString FileHandle::absFilePath() const
{
    if(d->absFilePath.isNull())
        d->absFilePath = resolveSymLinks(d->fileInfo.absFilePath());
    return d->absFilePath;
}

const QFileInfo &FileHandle::fileInfo() const
{
    return d->fileInfo;
}

bool FileHandle::isNull() const
{
    return *this == null();
}

bool FileHandle::current() const
{
    return(d->modificationTime.isValid() &&
           lastModified().isValid() &&
           d->modificationTime >= lastModified());
}

const QDateTime &FileHandle::lastModified() const
{
    if(d->lastModified.isNull())
        d->lastModified = d->fileInfo.lastModified();

    return d->lastModified;
}

void FileHandle::read(CacheDataStream &s)
{
    switch(s.cacheVersion()) {
    case 1:
    default:
        if(!d->tag)
            d->tag = new Tag(d->absFilePath, true);

        s >> *(d->tag);
        s >> d->modificationTime;
        break;
    }
}

FileHandle &FileHandle::operator=(const FileHandle &f)
{
    if(&f == this)
        return *this;

    if(d->deref())
        delete d;

    d = f.d;
    d->ref();

    return *this;
}

bool FileHandle::operator==(const FileHandle &f) const
{
    return d == f.d;
}

bool FileHandle::operator!=(const FileHandle &f) const
{
    return d != f.d;
}

const FileHandle &FileHandle::null() // static
{
    static FileHandle f;
    return f;
}

////////////////////////////////////////////////////////////////////////////////
// private methods
////////////////////////////////////////////////////////////////////////////////

void FileHandle::setup(const QFileInfo &info, const QString &path)
{
    if(d && !isNull())
        return;

    QString fileName = path.isNull() ? info.absFilePath() : path;

    FileHandle cached = Cache::instance()->value(fileName);

    if(cached != null()) {
        d = cached.d;
        d->ref();
    }
    else {
        d = new FileHandlePrivate;
        d->fileInfo = info;
        d->absFilePath = fileName;
	Cache::instance()->insert(*this);
    }
}

////////////////////////////////////////////////////////////////////////////////
// related functions
////////////////////////////////////////////////////////////////////////////////

QDataStream &operator<<(QDataStream &s, const FileHandle &f)
{
    s << *(f.tag())
      << f.lastModified();

    return s;
}

CacheDataStream &operator>>(CacheDataStream &s, FileHandle &f)
{
    f.read(s);
    return s;
}
