/***************************************************************************
    begin                : Sat Feb 14 2004
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

#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

//Added by qt3to4:
#include <QPixmap>
#include <QObject>
#include "filehandle.h"

class KSelectAction;
class SliderAction;
class StatusLabel;
class PlaylistInterface;

namespace Phonon
{
    class AudioOutput;
    class AudioPath;
    class MediaObject;
    class MediaQueue;
}

/**
 * This class serves as a proxy to the Player interface and handles managing
 * the actions from the top-level mainwindow.
 */

class PlayerManager : public QObject
{
    Q_OBJECT

protected:
    PlayerManager();
    virtual ~PlayerManager();

public:
    static PlayerManager *instance();

    bool playing() const;
    bool paused() const;
    float volume() const;
    int status() const;
    int totalTime() const;
    int currentTime() const;
    //int position() const;

    QStringList trackProperties();
    QString trackProperty(const QString &property) const;
    QPixmap trackCover(const QString &size) const;

    FileHandle playingFile() const;
    QString playingString() const;

    KSelectAction* outputDeviceSelectAction();

    void setPlaylistInterface(PlaylistInterface *interface);
    void setStatusLabel(StatusLabel *label);

    QString randomPlayMode() const;

public slots:

    void play(const FileHandle &file);
    void play(const QString &file);
    void play();
    void pause();
    void stop();
    void setVolume(float volume = 1.0);
    void seek(int seekTime);
    //void seekPosition(int position);
    void seekForward();
    void seekBack();
    void playPause();
    void forward();
    void back();
    void volumeUp();
    void volumeDown();
    void mute();

    void setRandomPlayMode(const QString &randomMode);

signals:
    void signalPlay();
    void signalPause();
    void signalStop();

private:
    void setup();

private slots:
    void slotNeedNextUrl();
    void slotFinished();
    void slotLength(qint64);
    void slotTick(qint64);
    //void slotUpdateTime(int position);

private:
    FileHandle m_file;
    SliderAction *m_sliderAction;
    PlaylistInterface *m_playlistInterface;
    StatusLabel *m_statusLabel;
    bool m_noSeek;
    bool m_muted;
    bool m_setup;
    bool m_ignoreFinished;

    static const int m_pollInterval = 800;

    Phonon::AudioOutput *m_output;
    Phonon::AudioPath *m_audioPath;
    Phonon::MediaQueue *m_mqueue;
    Phonon::MediaObject *m_media;
};

#endif

// vim: set et sw=4 tw=0 sta:
