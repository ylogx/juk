/***************************************************************************
                          tag.cpp  -  description
                             -------------------
    begin                : Sun Feb 17 2002
    copyright            : (C) 2002 by Scott Wheeler
    email                : scott@slackorama.net
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

#include <qdir.h>
#include <qregexp.h>

#include <id3/misc_support.h>

#include "tag.h"

#include "genrelist.h"
#include "genrelistlist.h"

#define REPLACE true

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

Tag::Tag(const QString &file)
{
    fileName = file;
    tag.Link(file.latin1());

    changed = false;

    // the easy ones -- these are supported in the id3 class

    char *temp;

    temp = ID3_GetArtist(&tag);
    artistName = temp;
    delete [] temp;

    temp = ID3_GetAlbum(&tag);
    albumName = temp;
    delete [] temp;

    temp = ID3_GetTitle(&tag);
    trackName = temp;
    delete [] temp;

    temp = ID3_GetTrack(&tag);
    trackNumberString = temp;
    delete [] temp;

    trackNumberString.replace(QRegExp("^0+"), QString::null);

    trackNumber = ID3_GetTrackNum(&tag);

    temp = ID3_GetComment(&tag);
    comment = temp;
    delete [] temp;

    temp = ID3_GetGenre(&tag);
    genre = temp;
    delete [] temp;

    genre.setID3v1(int(ID3_GetGenreNum(&tag)));

    temp = ID3_GetYear(&tag);
    yearString = temp;
    delete [] temp;

    hasTagBool = (tag.HasV2Tag() || tag.HasV1Tag());

    if(trackName.length() <= 0) {
        trackName = fileName;
        while((trackName.right(4)).lower() == ".mp3") {
            trackName = trackName.left(trackName.length() - 4);
        }
        trackName = trackName.right(trackName.length() - trackName.findRev(QDir::separator(), -1) -1);
    }

    // parse the genre string for (<ID3v1 number>)

    if(genre == "(" + QString::number(genre.getID3v1()) + ")" || genre == QString::null)
        genre = GenreListList::ID3v1List()->name(genre.getID3v1());
    else if(genre.find(QRegExp("\\([0-9]+\\)")) == 0)
        genre = genre.mid(genre.find(")") + 1);

    // convert the year

    year = yearString.toInt();

    if(year <= 0)
        yearString = QString::null;
}


Tag::~Tag()
{
    save();
}


bool Tag::exists()
{
    QFile id3_file(fileName);
    return(id3_file.exists());
}

void Tag::save()
{
    if(changed) {
        if(artistName.length()>0)
            ID3_AddArtist(&tag, artistName.latin1(), REPLACE);
        else
            ID3_RemoveArtists(&tag);

        if(albumName.length()>0)
            ID3_AddAlbum(&tag, albumName.latin1(), REPLACE);
        else
            ID3_RemoveAlbums(&tag);

        if(trackName.length()>0)
            ID3_AddTitle(&tag, trackName.latin1(), REPLACE);
        else
            ID3_RemoveTitles(&tag);

        if(trackNumber>0)
            ID3_AddTrack(&tag,  uchar(trackNumber),  uchar(0),  REPLACE);
        else
            ID3_RemoveTracks(&tag);

        //    ID3_AddGenre(&tag, 1, REPLACE);
        if(genre.getID3v1() >=0 && genre.getID3v1() <  int(GenreListList::ID3v1List()->count())) {
            QString genreString;

            if(genre != GenreListList::ID3v1List()->name(genre.getID3v1()))
                genreString = "(" + QString::number(genre.getID3v1()) + ")" + genre;
            else
                genreString = "(" + QString::number(genre.getID3v1()) + ")";

            ID3_AddGenre(&tag, genreString.latin1(), REPLACE);
        }
        else
            ID3_RemoveGenres(&tag);

        if(year > 0)
            ID3_AddYear(&tag, yearString.latin1(), REPLACE);
        else
            ID3_RemoveYears(&tag);

        ID3_RemoveComments(&tag);
        if(comment.length()>0)
            ID3_AddComment(&tag, comment.latin1(), REPLACE);

        tag.Update();
        changed = false;
    }
}

////////////////////////////////////////////////
// functions to gather information
////////////////////////////////////////////////

QString Tag::getFileName() const 
{ 
    return(fileName);
}

QString Tag::getTrack() const  // The song's name, not it's track number
{ 
    return(trackName); 
}              

QString Tag::getArtist() const 
{ 
    return(artistName);
}

QString Tag::getAlbum() const
{ 
    return(albumName);
}

Genre Tag::getGenre() const 
{ 
    return(genre);
}

int Tag::getTrackNumber() const 
{ 
    return(trackNumber);
}

QString Tag::getTrackNumberString() const
{ 
    return(trackNumberString);
}

int Tag::getYear() const
{ 
    return(year);
}

QString Tag::getYearString() const
{ 
    return(yearString);
}

QString Tag::getComment() const
{ 
    return(comment);
}
bool Tag::hasTag() const
{ 
    return(hasTagBool);
}

/////////////////////////////////////////////////////
// functions that set information
/////////////////////////////////////////////////////


void Tag::setTrack(QString value)
{
    changed = true;
    trackName = value;
};

void Tag::setArtist(QString value)
{
    changed = true;
    artistName = value;
};

void Tag::setAlbum(QString value)
{
    changed = true;
    albumName = value;
};

void Tag::setGenre(Genre value)
{
    changed = true;
    genre = value;
};

void Tag::setTrackNumber(int value)
{
    changed = true;
    trackNumber = value;
    if(trackNumber > 0)
        trackNumberString.setNum(value);
    else
        trackNumberString = QString::null;
};

void Tag::setYear(int value)
{
    changed = true;
    year = value;
    if(year > 0)
        yearString.setNum(value);
    else
        yearString = QString::null;
};

void Tag::setComment(QString value)
{
    changed = true;
    comment = value;
};

