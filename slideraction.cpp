/***************************************************************************
                          slideraction.cpp  -  description
                             -------------------
    begin                : Wed Feb 6 2002
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

#include <ktoolbar.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <qtooltip.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qslider.h>

#include "slideraction.h"

////////////////////////////////////////////////////////////////////////////////
// convenience class
////////////////////////////////////////////////////////////////////////////////

/**
 * This "custom" slider reverses the left and middle buttons.  Typically the
 * middle button "instantly" seeks rather than moving the slider towards the
 * click position in fixed intervals.  This behavior has now been mapped on
 * to the left mouse button.
 */

class TrackPositionSlider : public QSlider
{
public:
    TrackPositionSlider(QWidget *parent, const char *name) : QSlider(parent, name) {}

protected:
    virtual void mousePressEvent(QMouseEvent *e) {
        if(e->button() == LeftButton) {
            QMouseEvent reverse(QEvent::MouseButtonPress, e->pos(), MidButton, e->state());
            QSlider::mousePressEvent(&reverse);
            emit sliderPressed();
        }
        else if(e->button() == MidButton) {
            QMouseEvent reverse(QEvent::MouseButtonPress, e->pos(), LeftButton, e->state());
            QSlider::mousePressEvent(&reverse);
        }
    }
    virtual void focusInEvent(QFocusEvent *) { clearFocus(); }
};

class VolumeSlider : public QSlider
{
public:
    VolumeSlider(QWidget *parent, const char *name) : QSlider(parent, name) {}

protected:
    virtual void wheelEvent(QWheelEvent *e) {
        QWheelEvent transposed(e->pos(), -(e->delta()), e->state(), e->orientation());

        QSlider::wheelEvent(&transposed);
    }
    virtual void focusInEvent(QFocusEvent *) { clearFocus(); }
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

SliderAction::SliderAction(const QString &text, QObject *parent, const char *name)
    : CustomAction(text, parent, name)
{

}

SliderAction::~SliderAction()
{

}

////////////////////////////////////////////////////////////////////////////////
// public slots
////////////////////////////////////////////////////////////////////////////////

void SliderAction::slotUpdateOrientation(QDockWindow *dockWindow)
{
    // if the toolbar is not null and either the dockWindow not defined or is the toolbar
    if(customWidget && toolbar && (!dockWindow || dockWindow == dynamic_cast<QDockWindow *>(toolbar))) {
        if(toolbar->barPos() == KToolBar::Right || toolbar->barPos() == KToolBar::Left) {
            m_trackPositionSlider->setOrientation(Qt::Vertical);
            m_volumeSlider->setOrientation(Qt::Vertical);
            m_layout->setDirection(QBoxLayout::TopToBottom);
        }
        else {
            m_trackPositionSlider->setOrientation(Qt::Horizontal);
            m_volumeSlider->setOrientation(Qt::Horizontal);
            m_layout->setDirection(QBoxLayout::LeftToRight);
        }
    }
    slotUpdateSize();
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

QWidget *SliderAction::createWidget(QWidget *parent) // virtual -- used by base class
{
    if(parent) {
        QWidget *base = new QWidget(parent);
        base->setBackgroundMode(parent->backgroundMode());
        base->setName("kde toolbar widget");

        QToolBar *toolbar = dynamic_cast<QToolBar *>(parent);
        if(toolbar)
            toolbar->setStretchableWidget(base);

        m_layout = new QBoxLayout(base, QBoxLayout::TopToBottom, 5, 5);

        m_layout->addItem(new QSpacerItem(20, 1));

        QLabel *trackPositionLabel = new QLabel(base);
        trackPositionLabel->setName("kde toolbar widget");
        trackPositionLabel->setPixmap(SmallIcon("juk_time"));
        QToolTip::add(trackPositionLabel, i18n("Track position"));
        m_layout->addWidget(trackPositionLabel);

        m_trackPositionSlider = new TrackPositionSlider(base, "trackPositionSlider");
        m_trackPositionSlider->setMaxValue(1000);
        QToolTip::add(m_trackPositionSlider, i18n("Track position"));
        m_layout->addWidget(m_trackPositionSlider);

        m_layout->addItem(new QSpacerItem(10, 1));

        QLabel *volumeLabel = new QLabel(base);
        volumeLabel->setName("kde toolbar widget");
        volumeLabel->setPixmap(SmallIcon("juk_volume"));
        QToolTip::add(volumeLabel, i18n("Volume"));
        m_layout->addWidget(volumeLabel);

        m_volumeSlider = new VolumeSlider(base, "volumeSlider");
        m_volumeSlider->setMaxValue(100);
        QToolTip::add(m_volumeSlider, i18n("Volume"));
        m_layout->addWidget(m_volumeSlider);

        m_volumeSlider->setName("kde toolbar widget");
        m_trackPositionSlider->setName("kde toolbar widget");

        m_layout->setStretchFactor(m_trackPositionSlider, 4);
        m_layout->setStretchFactor(m_volumeSlider, 1);

        connect(parent, SIGNAL(modechange()), this, SLOT(slotUpdateSize()));
        return base;
    }
    else
        return 0;
}

////////////////////////////////////////////////////////////////////////////////
// private slots
////////////////////////////////////////////////////////////////////////////////

void SliderAction::slotUpdateSize()
{
    static const int offset = 3;
    static const int absoluteMax = 10000;

    if(customWidget && toolbar) {
        if(toolbar->barPos() == KToolBar::Right || toolbar->barPos() == KToolBar::Left) {
            m_volumeSlider->setMaximumWidth(toolbar->iconSize() - offset);
            m_volumeSlider->setMaximumHeight(volumeMax);

            m_trackPositionSlider->setMaximumWidth(toolbar->iconSize() - offset);
            m_trackPositionSlider->setMaximumHeight(absoluteMax);
        }
        else {
            m_volumeSlider->setMaximumHeight(toolbar->iconSize() - offset);
            m_volumeSlider->setMaximumWidth(volumeMax);

            m_trackPositionSlider->setMaximumHeight(toolbar->iconSize() - offset);
            m_trackPositionSlider->setMaximumWidth(absoluteMax);
        }
    }

}

#include "slideraction.moc"
