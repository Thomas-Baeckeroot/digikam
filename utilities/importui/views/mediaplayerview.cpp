/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-08-22
 * Description : A view to embed Phonon media player in import interface.
 *
 * Copyright (C) 2012 Islam Wazery <wazery at ubuntu dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "mediaplayerview.moc"

// Qt includes

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QFrame>
#include <QGridLayout>
#include <QToolBar>
#include <QEvent>
#include <QMouseEvent>

// KDE includes

#include <kapplication.h>
#include <kaction.h>
#include <kdialog.h>
#include <klocale.h>
#include <phonon/seekslider.h>
#include <phonon/videoplayer.h>
#include <phonon/videowidget.h>

// Local includes

#include "importstackedview.h"
#include "thememanager.h"

namespace Digikam
{

ImportMediaPlayerMouseClickFilter::ImportMediaPlayerMouseClickFilter(QObject* const parent)
    : QObject(parent), m_parent(parent)
{
}

bool ImportMediaPlayerMouseClickFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

        if (mouseEvent && mouseEvent->button() == Qt::LeftButton)
        {
            if (m_parent)
            {
                ImportMediaPlayerView* mplayer = dynamic_cast<ImportMediaPlayerView*>(m_parent);

                if (mplayer)
                {
                    mplayer->slotEscapePressed();
                }
            }

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return QObject::eventFilter(obj, event);
    }
}

// --------------------------------------------------------

class ImportMediaPlayerView::Private
{

public:

    enum ImportMediaPlayerViewMode
    {
        ErrorView=0,
        PlayerView
    };

public:

    Private() :
        errorView(0),
        playerView(0),
        escapePreviewAction(0),
        prevAction(0),
        nextAction(0),
        toolBar(0),
        grid(0),
        player(0),
        slider(0)
    {
    }

    QFrame*              errorView;
    QFrame*              playerView;

    QAction*             escapePreviewAction;
    QAction*             prevAction;
    QAction*             nextAction;

    QToolBar*            toolBar;

    QGridLayout*         grid;

    Phonon::VideoPlayer* player;
    Phonon::SeekSlider*  slider;

    KUrl                 currentItem;
};

ImportMediaPlayerView::ImportMediaPlayerView(ImportStackedView* const parent)
    : QStackedWidget(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    d->escapePreviewAction = new QAction(SmallIcon("folder-image"), i18n("Escape preview"),                this);
    d->prevAction          = new QAction(SmallIcon("go-previous"),  i18nc("go to previous image", "Back"), this);
    d->nextAction          = new QAction(SmallIcon("go-next"),      i18nc("go to next image", "Forward"),  this);

    d->errorView           = new QFrame(this);
    QLabel* errorMsg       = new QLabel(i18n("An error has occurred with the media player...."), this);

    errorMsg->setAlignment(Qt::AlignCenter);
    d->errorView->setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    d->errorView->setLineWidth(1);

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(errorMsg, 1, 0, 1, 3 );
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(0, 10);
    grid->setRowStretch(2, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());
    d->errorView->setLayout(grid);

    insertWidget(Private::ErrorView, d->errorView);

    // --------------------------------------------------------------------------

    d->playerView = new QFrame(this);
    d->player     = new Phonon::VideoPlayer(Phonon::VideoCategory, this);
    d->slider     = new Phonon::SeekSlider(this);
    d->slider->setMediaObject(d->player->mediaObject());
    d->player->mediaObject()->setTickInterval(100);
    d->player->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->playerView->setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    d->playerView->setLineWidth(1);

    d->grid = new QGridLayout;
    d->grid->addWidget(d->player->videoWidget(), 0, 0, 1, 3);
    d->grid->addWidget(d->slider,                1, 0, 1, 3);
    d->grid->setColumnStretch(0, 10);
    d->grid->setColumnStretch(2, 10);
    d->grid->setRowStretch(0, 10);
    d->grid->setMargin(KDialog::spacingHint());
    d->grid->setSpacing(KDialog::spacingHint());
    d->playerView->setLayout(d->grid);

    insertWidget(Private::PlayerView, d->playerView);

    d->toolBar = new QToolBar(this);
    d->toolBar->addAction(d->prevAction);
    d->toolBar->addAction(d->nextAction);
    d->toolBar->addAction(d->escapePreviewAction);

    setPreviewMode(Private::PlayerView);

    d->errorView->installEventFilter(new ImportMediaPlayerMouseClickFilter(this));
    d->player->videoWidget()->installEventFilter(new ImportMediaPlayerMouseClickFilter(this));

    // --------------------------------------------------------------------------

    connect(d->player->mediaObject(), SIGNAL(finished()),
            this, SLOT(slotPlayerFinished()));

    connect(d->player->mediaObject(), SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(slotPlayerstateChanged(Phonon::State,Phonon::State)));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(signalPrevItem()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(signalNextItem()));

    connect(d->escapePreviewAction, SIGNAL(triggered()),
            parent, SIGNAL(signalBack2FilesList()));
}

ImportMediaPlayerView::~ImportMediaPlayerView()
{
    d->player->stop();
    delete d->player;
    delete d;
}

void ImportMediaPlayerView::slotPlayerFinished()
{
    if (d->player->mediaObject()->errorType() == Phonon::FatalError)
    {
        setPreviewMode(Private::ErrorView);
    }
}

void ImportMediaPlayerView::slotPlayerstateChanged(Phonon::State newState, Phonon::State /*oldState*/)
{
    if (newState == Phonon::ErrorState)
    {
        setPreviewMode(Private::ErrorView);
    }
}

void ImportMediaPlayerView::escapePreview()
{
    d->player->stop();
}

void ImportMediaPlayerView::slotThemeChanged()
{
    QPalette palette;
    palette.setColor(d->errorView->backgroundRole(), kapp->palette().color(QPalette::Base));
    d->errorView->setPalette(palette);

    QPalette palette2;
    palette2.setColor(d->playerView->backgroundRole(), kapp->palette().color(QPalette::Base));
    d->playerView->setPalette(palette2);
}

void ImportMediaPlayerView::slotEscapePressed()
{
    escapePreview();
    emit signalEscapePreview();
}

int ImportMediaPlayerView::previewMode()
{
    return indexOf(currentWidget());
}

void ImportMediaPlayerView::setPreviewMode(int mode)
{
    if (mode != Private::ErrorView && mode != Private::PlayerView)
    {
        return;
    }

    setCurrentIndex(mode);
    d->toolBar->raise();
}

void ImportMediaPlayerView::setCurrentItem(const KUrl& url, bool asPrevious, bool asNext)
{
    d->prevAction->setEnabled(asPrevious);
    d->nextAction->setEnabled(asNext);

    if (url.isEmpty())
    {
        d->currentItem = url;
        d->player->stop();
        return;
    }

    if (d->currentItem == url &&
        (d->player->isPlaying() || d->player->isPaused()))
    {
        return;
    }

    d->currentItem = url;

    d->player->play(d->currentItem);
    setPreviewMode(Private::PlayerView);
}

}  // namespace Digikam
