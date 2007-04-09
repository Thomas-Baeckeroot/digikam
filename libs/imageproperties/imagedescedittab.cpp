/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at gmail dot com>
 *          Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date   : 2003-03-09
 * Description : Comments, Tags, and Rating properties editor
 *
 * Copyright 2003-2005 by Renchi Raju & Gilles Caulier
 * Copyright 2006-2007 by Gilles Caulier & Marcel Wiesweg
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

// Qt includes.

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qiconset.h>
#include <qwhatsthis.h>
#include <qtooltip.h>

// KDE includes.

#include <kabc/stdaddressbook.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include <kurl.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ktextedit.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kdialogbase.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "kdatetimeedit.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"
#include "tagcreatedlg.h"
#include "navigatebarwidget.h"
#include "ratingwidget.h"
#include "talbumlistview.h"
#include "tagfilterview.h"
#include "imageinfo.h"
#include "imageattributeswatch.h"
#include "metadatahub.h"
#include "statusprogressbar.h"
#include "imagedescedittab.h"
#include "imagedescedittab.moc"

namespace Digikam
{

class ImageDescEditTabPriv
{

public:

    ImageDescEditTabPriv()
    {
        modified                   = false;
        ignoreImageAttributesWatch = false;
        recentTagsBtn              = 0;
        tagsSearchClearBtn         = 0;
        commentsEdit               = 0;
        tagsSearchEdit             = 0;
        dateTimeEdit               = 0;
        tagsView                   = 0;
        ratingWidget               = 0;
        ABCMenu                    = 0;
        assignedTagsBtn            = 0;
        applyBtn                   = 0;
        revertBtn                  = 0;
        toggleAutoTags             = TagFilterView::NoToggleAuto;
    }

    bool                           modified;
    bool                           ignoreImageAttributesWatch;

    QToolButton                   *recentTagsBtn;
    QToolButton                   *tagsSearchClearBtn;
    QToolButton                   *assignedTagsBtn;
    QToolButton                   *revertBtn;

    QPopupMenu                    *ABCMenu;

    QPushButton                   *applyBtn;

    QPushButton                   *moreButton;
    QPopupMenu                    *moreMenu;

    KTextEdit                     *commentsEdit;

    KLineEdit                     *tagsSearchEdit;

    KDateTimeEdit                 *dateTimeEdit;

    QPtrList<ImageInfo>            currInfos;

    TAlbumListView                *tagsView;

    RatingWidget                  *ratingWidget;

    TagFilterView::ToggleAutoTags  toggleAutoTags;

    MetadataHub                    hub;
};

ImageDescEditTab::ImageDescEditTab(QWidget *parent, bool navBar)
                : NavigateBarTab(parent)
{
    d = new ImageDescEditTabPriv;

    setupNavigateBar(navBar);
    QWidget *settingsArea = new QWidget(this);

    m_navigateBarLayout->addWidget(settingsArea);

    QGridLayout *settingsLayout = new QGridLayout(settingsArea, 5, 1, 
                                      KDialog::spacingHint(), KDialog::spacingHint());

    // Comments/Date/Rating view -----------------------------------
    
    QVBox *commentsBox = new QVBox(settingsArea);
    new QLabel(i18n("Comments:"), commentsBox);
    d->commentsEdit = new KTextEdit(commentsBox);
    d->commentsEdit->setTextFormat(QTextEdit::PlainText);
    d->commentsEdit->setCheckSpellingEnabled(true);

    QHBox *dateBox  = new QHBox(settingsArea);
    new QLabel(i18n("Date:"), dateBox);
    d->dateTimeEdit = new KDateTimeEdit(dateBox, "datepicker");

    QHBox *ratingBox = new QHBox(settingsArea);
    new QLabel(i18n("Rating:"), ratingBox);
    d->ratingWidget  = new RatingWidget(ratingBox);

    // Tags view ---------------------------------------------------

    QHBox *tagsSearch = new QHBox(settingsArea);
    tagsSearch->setSpacing(KDialog::spacingHint());

    d->tagsSearchClearBtn = new QToolButton(tagsSearch);
    d->tagsSearchClearBtn->setAutoRaise(true);
    d->tagsSearchClearBtn->setIconSet(kapp->iconLoader()->loadIcon("locationbar_erase",
                                      KIcon::Toolbar, KIcon::SizeSmall));

    new QLabel(i18n("Search:"), tagsSearch);
    d->tagsSearchEdit = new KLineEdit(tagsSearch);
    d->tagsSearchEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    d->assignedTagsBtn = new QToolButton(tagsSearch);
    QToolTip::add(d->assignedTagsBtn, i18n("Already assigned tags"));
    d->assignedTagsBtn->setIconSet(kapp->iconLoader()->loadIcon("tag-assigned",
                                   KIcon::NoGroup, KIcon::SizeSmall, 
                                   KIcon::DefaultState, 0, true));
    d->assignedTagsBtn->setToggleButton(true);

    d->recentTagsBtn      = new QToolButton(tagsSearch);
    QPopupMenu *popupMenu = new QPopupMenu(d->recentTagsBtn);
    QToolTip::add(d->recentTagsBtn, i18n("Recent Tags"));
    d->recentTagsBtn->setIconSet(kapp->iconLoader()->loadIcon("tag-recents", 
                                 KIcon::NoGroup, KIcon::SizeSmall, 
                                 KIcon::DefaultState, 0, true));
    d->recentTagsBtn->setUsesBigPixmap(false);
    d->recentTagsBtn->setPopup(popupMenu);
    d->recentTagsBtn->setPopupDelay(1);

    d->tagsView = new TAlbumListView(settingsArea);

    // Buttons -----------------------------------------

    QHBox *buttonsBox = new QHBox(settingsArea);
    buttonsBox->setSpacing(KDialog::spacingHint());

    d->revertBtn = new QToolButton(buttonsBox);
    d->revertBtn->setIconSet(SmallIcon("reload_page"));
    QToolTip::add(d->revertBtn, i18n("Revert all changes"));
    d->revertBtn->setEnabled(false);
    
    d->applyBtn = new QPushButton(i18n("Apply"), buttonsBox);
    d->applyBtn->setIconSet(SmallIcon("button_ok"));
    d->applyBtn->setEnabled(false);
    QToolTip::add(d->applyBtn, i18n("Apply all changes to pictures"));
    buttonsBox->setStretchFactor(d->applyBtn, 10);

    d->moreButton = new QPushButton(i18n("More"), buttonsBox);
    d->moreMenu = new QPopupMenu(this);
    d->moreButton->setPopup(d->moreMenu);

    // --------------------------------------------------

    settingsLayout->addMultiCellWidget(commentsBox, 0, 0, 0, 1);
    settingsLayout->addMultiCellWidget(dateBox, 1, 1, 0, 1);
    settingsLayout->addMultiCellWidget(ratingBox, 2, 2, 0, 1);
    settingsLayout->addMultiCellWidget(d->tagsView, 3, 3, 0, 1);
    settingsLayout->addMultiCellWidget(tagsSearch, 4, 4, 0, 1);
    settingsLayout->addMultiCellWidget(buttonsBox, 5, 5, 0, 1);
    settingsLayout->setRowStretch(3, 10);

    // --------------------------------------------------

    connect(d->tagsView, SIGNAL(signalProgressBarMode(int, const QString&)),
            this, SIGNAL(signalProgressBarMode(int, const QString&)));

    connect(d->tagsView, SIGNAL(signalProgressValue(int)),
            this, SIGNAL(signalProgressValue(int)));

    connect(popupMenu, SIGNAL(activated(int)),
            this, SLOT(slotRecentTagsMenuActivated(int)));

    connect(d->tagsView, SIGNAL(signalItemStateChanged(TAlbumCheckListItem *)),
            this, SLOT(slotItemStateChanged(TAlbumCheckListItem *)));
    
    connect(d->commentsEdit, SIGNAL(textChanged()),
            this, SLOT(slotCommentChanged()));
    
    connect(d->dateTimeEdit, SIGNAL(dateTimeChanged(const QDateTime& )),
            this, SLOT(slotDateTimeChanged(const QDateTime&)));

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotRatingChanged(int)));
     
    connect(d->tagsView, SIGNAL(rightButtonClicked(QListViewItem*, const QPoint &, int)),
            this, SLOT(slotRightButtonClicked(QListViewItem*, const QPoint&, int)));
            
    connect(d->tagsSearchClearBtn, SIGNAL(clicked()),
            d->tagsSearchEdit, SLOT(clear()));
            
    connect(d->tagsSearchEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTagsSearchChanged()));

    connect(d->assignedTagsBtn, SIGNAL(toggled(bool)),
            this, SLOT(slotAssignedTagsToggled(bool)));

    connect(d->applyBtn, SIGNAL(clicked()),
            this, SLOT(slotApplyAllChanges()));

    connect(d->revertBtn, SIGNAL(clicked()),
            this, SLOT(slotRevertAllChanges()));

    connect(d->moreMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotMoreMenu()));

    // Initialize ---------------------------------------------

    d->commentsEdit->installEventFilter(this);
    d->dateTimeEdit->installEventFilter(this);
    d->ratingWidget->installEventFilter(this);
    d->tagsView->installEventFilter(this);
    updateRecentTags();

    // Connect to album manager -----------------------------

    AlbumManager* man = AlbumManager::instance();
    
    connect(man, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));
    
    connect(man, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));
    
    connect(man, SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));
    
    connect(man, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));
    
    connect(man, SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotAlbumIconChanged(Album*)));

    connect(man, SIGNAL(signalTAlbumMoved(TAlbum*, TAlbum*)),
            this, SLOT(slotAlbumMoved(TAlbum*, TAlbum*)));

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
            this, SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));

    connect(loader, SIGNAL(signalFailed(Album *)),
            this, SLOT(slotThumbnailLost(Album *)));

    connect(loader, SIGNAL(signalReloadThumbnails()),
            this, SLOT(slotReloadThumbnails()));

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalImageTagsChanged(Q_LLONG)),
            this, SLOT(slotImageTagsChanged(Q_LLONG)));

    connect(watch, SIGNAL(signalImagesChanged(int)),
            this, SLOT(slotImagesChanged(int)));

    connect(watch, SIGNAL(signalImageRatingChanged(Q_LLONG)),
            this, SLOT(slotImageRatingChanged(Q_LLONG)));

    connect(watch, SIGNAL(signalImageDateChanged(Q_LLONG)),
            this, SLOT(slotImageDateChanged(Q_LLONG)));

    connect(watch, SIGNAL(signalImageCaptionChanged(Q_LLONG)),
            this, SLOT(slotImageCaptionChanged(Q_LLONG)));

    // -- read config ---------------------------------------------------------

    KConfig* config = kapp->config();
    config->setGroup("Tag List View");
    d->toggleAutoTags = (TagFilterView::ToggleAutoTags)(config->readNumEntry("Toggle Auto Tags", 
                                                       TagFilterView::NoToggleAuto));
}

ImageDescEditTab::~ImageDescEditTab()
{
    slotChangingItems();

    /*
    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        (*it)->removeExtraData(this);
    }
    */

    KConfig* config = kapp->config();
    config->setGroup("Tag List View");
    config->writeEntry("Toggle Auto Tags", (int)(d->toggleAutoTags));
    config->sync();

    delete d;
}

bool ImageDescEditTab::singleSelection() const
{
    return (d->currInfos.count() == 1);
}

void ImageDescEditTab::slotChangingItems()
{
    if (!d->modified)
        return;

    if (d->currInfos.isEmpty())
        return;

    if (!AlbumSettings::instance()->getApplySidebarChangesDirectly())
    {
        KDialogBase *dialog = new KDialogBase(i18n("Apply changes?"),
                                              KDialogBase::Yes | KDialogBase::No,
                                              KDialogBase::Yes, KDialogBase::No,
                                              this, "applyChanges",
                                              true, true,
                                              KStdGuiItem::yes(), KStdGuiItem::discard());

        int changedFields = 0;
        if (d->hub.commentChanged())
            changedFields++;
        if (d->hub.dateTimeChanged())
            changedFields++;
        if (d->hub.ratingChanged())
            changedFields++;
        if (d->hub.tagsChanged())
            changedFields++;

        QString text;
        if (changedFields == 1)
        {
            if (d->hub.commentChanged())
                text = i18n("<qt><p>You have edited the comment of the picture. ",
                            "<qt><p>You have edited the comment of %n pictures. ",
                            d->currInfos.count());
            else if (d->hub.dateTimeChanged())
                text = i18n("<qt><p>You have edited the date of the picture. ",
                            "<qt><p>You have edited the date of %n pictures. ",
                            d->currInfos.count());
            else if (d->hub.ratingChanged())
                text = i18n("<qt><p>You have edited the rating of the picture. ",
                            "<qt><p>You have edited the rating of %n pictures. ",
                            d->currInfos.count());
            else if (d->hub.tagsChanged())
                text = i18n("<qt><p>You have edited the tags of the picture. ",
                            "<qt><p>You have edited the tags of %n pictures. ",
                            d->currInfos.count());

            text += i18n("Do you want to apply your changes?</p></qt>");
        }
        else
        {
            text = i18n("<qt><p>You have edited the metadata of the picture: </p><ul>",
                        "<qt><p>You have edited the metadata of %n pictures: </p><ul>",
                        d->currInfos.count());

            if (d->hub.commentChanged())
                text += i18n("<li>the comment</li>");
            if (d->hub.dateTimeChanged())
                text += i18n("<li>the date</li>");
            if (d->hub.ratingChanged())
                text += i18n("<li>the rating</li>");
            if (d->hub.tagsChanged())
                text += i18n("<li>the tags</li>");

            text += "</ul><p>";

            text += i18n("Do you want to apply your changes?</p></qt>");
        }

        bool alwaysApply = false;
        int returnCode = KMessageBox::createKMessageBox
                         (dialog, QMessageBox::Information,
                          text, QStringList(),
                          i18n("Always apply changes without confirmation"),
                          &alwaysApply, KMessageBox::Notify);

        if (alwaysApply)
            AlbumSettings::instance()->setApplySidebarChangesDirectly(true);

        if (returnCode == KDialogBase::User1)
            return;
        // otherwise apply
    }

    slotApplyAllChanges();
}

void ImageDescEditTab::slotApplyAllChanges()
{
    if (!d->modified)
        return;

    if (d->currInfos.isEmpty())
        return;

    bool progressInfo = (d->currInfos.count() > 1);
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                               i18n("Applying changes to pictures. Please wait..."));
    MetadataWriteSettings writeSettings = MetadataHub::defaultWriteSettings();

    // debugging - use this to indicate reentry from event loop (kapp->processEvents)
    // remove before final release
    if (d->ignoreImageAttributesWatch)
    {
        DWarning() << "ImageDescEditTab::slotApplyAllChanges(): re-entering from event loop!" << endl;
    }

    // we are now changing attributes ourselves
    d->ignoreImageAttributesWatch = true;
    AlbumManager::instance()->albumDB()->beginTransaction();
    int i=0;
    for (ImageInfo *info = d->currInfos.first(); info; info = d->currInfos.next())
    {
        // apply to database
        d->hub.write(info);
        // apply to file metadata
        d->hub.write(info->filePath(), MetadataHub::FullWrite, writeSettings);

        emit signalProgressValue((int)((i++/(float)d->currInfos.count())*100.0));
        if (progressInfo)
            kapp->processEvents();
    }
    AlbumManager::instance()->albumDB()->commitTransaction();
    d->ignoreImageAttributesWatch = false;

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());

    d->modified = false;
    d->hub.resetChanged();
    d->applyBtn->setEnabled(false);
    d->revertBtn->setEnabled(false);

    updateRecentTags();
}

void ImageDescEditTab::slotRevertAllChanges()
{
    if (!d->modified)
        return;

    if (d->currInfos.isEmpty())
        return;

    setInfos(d->currInfos);
}

void ImageDescEditTab::setItem(ImageInfo *info)
{
    slotChangingItems();
    QPtrList<ImageInfo> list;
    if (info)
        list.append(info);
    setInfos(list);
}

void ImageDescEditTab::setItems(QPtrList<ImageInfo> infos)
{
    slotChangingItems();
    setInfos(infos);
}

void ImageDescEditTab::setInfos(QPtrList<ImageInfo> infos)
{
    if (infos.isEmpty())
    {
        d->hub = MetadataHub();
        d->commentsEdit->blockSignals(true);
        d->commentsEdit->clear();
        d->commentsEdit->blockSignals(false);
        d->currInfos.clear();
        setEnabled(false);
        return;
    }

    setEnabled(true);
    d->currInfos = infos;
    d->modified  = false;
    d->hub = MetadataHub();
    d->applyBtn->setEnabled(false);
    d->revertBtn->setEnabled(false);

    for (ImageInfo *info = d->currInfos.first(); info; info = d->currInfos.next())
    {
        d->hub.load(info);
    }

    updateComments();
    updateRating();
    updateDate();
    updateTagsView();
}

void ImageDescEditTab::slotReadFromFileMetadataToDatabase()
{
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                               i18n("Reading metadata from files. Please wait..."));

    d->ignoreImageAttributesWatch = true;
    AlbumManager::instance()->albumDB()->beginTransaction();
    int i=0;
    for (ImageInfo *info = d->currInfos.first(); info; info = d->currInfos.next())
    {
        // A batch operation: a hub for each single file, not the common hub
        MetadataHub fileHub(MetadataHub::NewTagsImport);
        // read in from DMetadata
        fileHub.load(info->filePath());
        // write out to database
        fileHub.write(info);

        emit signalProgressValue((int)((i++/(float)d->currInfos.count())*100.0));
        kapp->processEvents();
    }
    AlbumManager::instance()->albumDB()->commitTransaction();
    d->ignoreImageAttributesWatch = false;

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());

    // reload everything
    setInfos(d->currInfos);
}

void ImageDescEditTab::slotWriteToFileMetadataFromDatabase()
{
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode, 
                               i18n("Writing metadata to files. Please wait..."));
    MetadataWriteSettings writeSettings = MetadataHub::defaultWriteSettings();

    int i=0;
    for (ImageInfo *info = d->currInfos.first(); info; info = d->currInfos.next())
    {
        MetadataHub fileHub;
        // read in from database
        fileHub.load(info);
        // write out to file DMetadata
        fileHub.write(info->filePath());

        emit signalProgressValue((int)((i++/(float)d->currInfos.count())*100.0));
        kapp->processEvents();
    }

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
}

bool ImageDescEditTab::eventFilter(QObject *, QEvent *e)
{
    if ( e->type() == QEvent::KeyPress )
    {
        QKeyEvent *k = (QKeyEvent *)e;
        if (k->state() == Qt::ControlButton &&
            (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            emit signalNextItem();
            return true;
        }
        else if (k->state() == Qt::ShiftButton &&
                 (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            emit signalPrevItem();
            return true;
        }

        return false;
    }
    
    return false;
}

void ImageDescEditTab::populateTags()
{
    d->tagsView->clear();

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum *tag = (TAlbum*)(*it);
        slotAlbumAdded(tag);
    }
}

void ImageDescEditTab::slotItemStateChanged(TAlbumCheckListItem *item)
{
    TagFilterView::ToggleAutoTags oldAutoTags = d->toggleAutoTags;

    switch(d->toggleAutoTags)
    {
        case TagFilterView::Children:
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(item->m_album, item->isOn());
            d->toggleAutoTags = oldAutoTags;
            break;
        case TagFilterView::Parents:
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleParentTags(item->m_album, item->isOn());
            d->toggleAutoTags = oldAutoTags;
            break;
        case TagFilterView::ChildrenAndParents:
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(item->m_album, item->isOn());
            toggleParentTags(item->m_album, item->isOn());
            d->toggleAutoTags = oldAutoTags;
            break;
        default:
            break;
    }

    d->hub.setTag(item->m_album, item->isOn());

    d->tagsView->blockSignals(true);
    item->setStatus(d->hub.tagStatus(item->m_album));
    d->tagsView->blockSignals(false);

    slotModified();
}

void ImageDescEditTab::slotCommentChanged()
{
    // we cannot trust that the text actually changed
    // (there are bogus signals caused by spell checking, see bug 141663)
    // so we have to check before marking the metadata as modified
    if (d->hub.comment() == d->commentsEdit->text())
        return;

    d->hub.setComment(d->commentsEdit->text());
    setMetadataWidgetStatus(d->hub.commentStatus(), d->commentsEdit);
    slotModified();
}

void ImageDescEditTab::slotDateTimeChanged(const QDateTime& dateTime)
{
    d->hub.setDateTime(dateTime);
    setMetadataWidgetStatus(d->hub.dateTimeStatus(), d->dateTimeEdit);
    slotModified();
}

void ImageDescEditTab::slotRatingChanged(int rating)
{
    d->hub.setRating(rating);
    // no handling for MetadataDisjoint needed for rating,
    // we set it to 0 when disjoint, see below
    slotModified();
}

void ImageDescEditTab::slotModified()
{
    d->modified = true;
    d->applyBtn->setEnabled(true);
    d->revertBtn->setEnabled(true);
}

void ImageDescEditTab::assignRating(int rating)
{
    d->ratingWidget->setRating(rating);
}

void ImageDescEditTab::updateTagsView()
{
    d->tagsView->blockSignals(true);

    QListViewItemIterator it( d->tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* tItem = dynamic_cast<TAlbumCheckListItem*>(it.current());
        if (tItem)
            tItem->setStatus(d->hub.tagStatus(tItem->m_album));
        ++it;
    }

    // The condition is a temporary fix not to destroy name filtering on image change.
    // See comments in these methods.
    if (d->assignedTagsBtn->isOn())
        slotAssignedTagsToggled(d->assignedTagsBtn->isOn());

    d->tagsView->blockSignals(false);
}

void ImageDescEditTab::updateComments()
{
    d->commentsEdit->blockSignals(true);
    d->commentsEdit->setText(d->hub.comment());
    setMetadataWidgetStatus(d->hub.commentStatus(), d->commentsEdit);
    d->commentsEdit->blockSignals(false);
}

void ImageDescEditTab::updateRating()
{
    d->ratingWidget->blockSignals(true);
    if (d->hub.ratingStatus() == MetadataHub::MetadataDisjoint)
        d->ratingWidget->setRating(0);
    else
        d->ratingWidget->setRating(d->hub.rating());
    d->ratingWidget->blockSignals(false);
}

void ImageDescEditTab::updateDate()
{
    d->dateTimeEdit->blockSignals(true);
    d->dateTimeEdit->setDateTime(d->hub.dateTime());
    setMetadataWidgetStatus(d->hub.dateTimeStatus(), d->dateTimeEdit);
    d->dateTimeEdit->blockSignals(false);
}

void ImageDescEditTab::setMetadataWidgetStatus(int status, QWidget *widget)
{
    if (status == MetadataHub::MetadataDisjoint)
    {
        // For text widgets: Set text color to color of disabled text
        QPalette palette = widget->palette();
        palette.setColor(QColorGroup::Text, palette.color(QPalette::Disabled, QColorGroup::Text));
        widget->setPalette(palette);
    }
    else
    {
        widget->unsetPalette();
    }
}

void ImageDescEditTab::slotRightButtonClicked(QListViewItem *item, const QPoint &, int )
{
    TAlbum *album;

    if (!item)
    {
        album = AlbumManager::instance()->findTAlbum(0);
    }
    else
    {
        TAlbumCheckListItem* viewItem = dynamic_cast<TAlbumCheckListItem*>(item);

        if(!viewItem)
            album = AlbumManager::instance()->findTAlbum(0);
        else
            album = viewItem->m_album;
    }

    if(!album)
        return;

    d->ABCMenu = new QPopupMenu;
    
    connect(d->ABCMenu, SIGNAL( aboutToShow() ),
            this, SLOT( slotABCContextMenu() ));

    KPopupMenu popmenu(this);
    popmenu.insertTitle(SmallIcon("digikam"), i18n("Tags"));
    popmenu.insertItem(SmallIcon("tag-new"),  i18n("New Tag..."), 10);
    popmenu.insertItem(SmallIcon("tag-addressbook"), i18n("Create Tag From AddressBook"), d->ABCMenu);

    if (!album->isRoot())
    {
        popmenu.insertItem(SmallIcon("tag-properties"), i18n("Edit Tag Properties..."), 11);
        popmenu.insertItem(SmallIcon("tag-reset"),      i18n("Reset Tag Icon"),         13);        
        popmenu.insertSeparator(-1);
        popmenu.insertItem(SmallIcon("tag-delete"),     i18n("Delete Tag"),             12);
    }

    popmenu.insertSeparator(-1);

    QPopupMenu selectTagsMenu;
    selectTagsMenu.insertItem(i18n("All Tags"),   14);
    if (!album->isRoot())
    {
        selectTagsMenu.insertSeparator(-1);
        selectTagsMenu.insertItem(i18n("Childs"),     17);
        selectTagsMenu.insertItem(i18n("Parents"),    19);
    }
    popmenu.insertItem(i18n("Select"), &selectTagsMenu);

    QPopupMenu deselectTagsMenu;
    deselectTagsMenu.insertItem(i18n("All Tags"), 15);
    if (!album->isRoot())
    {
        deselectTagsMenu.insertSeparator(-1);
        deselectTagsMenu.insertItem(i18n("Childs"),   18);
        deselectTagsMenu.insertItem(i18n("Parents"),  20);
    }
    popmenu.insertItem(i18n("Deselect"), &deselectTagsMenu);

    popmenu.insertItem(i18n("Invert Selection"),  16);
    popmenu.insertSeparator(-1);

    QPopupMenu toggleAutoMenu;
    toggleAutoMenu.setCheckable(true);
    toggleAutoMenu.insertItem(i18n("None"),    21);
    toggleAutoMenu.insertSeparator(-1);
    toggleAutoMenu.insertItem(i18n("Childs"),  22);
    toggleAutoMenu.insertItem(i18n("Parents"), 23);
    toggleAutoMenu.insertItem(i18n("Both"),    24);
    toggleAutoMenu.setItemChecked(21 + d->toggleAutoTags, true);
    popmenu.insertItem(i18n("Toogle Auto"), &toggleAutoMenu);

    TagFilterView::ToggleAutoTags oldAutoTags = d->toggleAutoTags;            

    int choice = popmenu.exec((QCursor::pos()));
    switch( choice )
    {
        case 10:   // New Tag.
        {
            tagNew(album);
            break;
        }
        case 11:   // Edit Tag Properties.
        {
            if (!album->isRoot())
                tagEdit(album);
            break;
        }
        case 12:   // Delete Tag.
        {
            if (!album->isRoot())
                tagDelete(album);
            break;
        }
        case 13:   // Reset Tag Icon.
        {
            QString errMsg;
            AlbumManager::instance()->updateTAlbumIcon(album, QString("tag"), 0, errMsg);
            break;
        }
        case 14:   // Select All Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            QListViewItemIterator it(d->tagsView, QListViewItemIterator::NotChecked);
            while (it.current())
            {
                TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
                if (item->isVisible())
                    item->setOn(true);
                ++it;
            }
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 15:   // Deselect All Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            QListViewItemIterator it(d->tagsView, QListViewItemIterator::Checked);
            while (it.current())
            {
                TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
                if (item->isVisible())
                    item->setOn(false);
                ++it;
            }
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 16:   // Invert All Tags Selection.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            QListViewItemIterator it(d->tagsView);
            while (it.current())
            {
                TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
                if (item->isVisible())
                    item->setOn(!item->isOn());
                ++it;
            }
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 17:   // Select Child Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(album, true);
            TAlbumCheckListItem *item = (TAlbumCheckListItem*)album->extraData(this);
            item->setOn(true);            
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 18:   // Deselect Child Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleChildTags(album, false);
            TAlbumCheckListItem *item = (TAlbumCheckListItem*)album->extraData(this);
            item->setOn(false);            
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 19:   // Select Parent Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleParentTags(album, true);
            TAlbumCheckListItem *item = (TAlbumCheckListItem*)album->extraData(this);
            item->setOn(true);            
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 20:   // Deselect Parent Tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            toggleParentTags(album, false);
            TAlbumCheckListItem *item = (TAlbumCheckListItem*)album->extraData(this);
            item->setOn(false);            
            d->toggleAutoTags = oldAutoTags;
            break;
        }
        case 21:   // No toggle auto tags.
        {
            d->toggleAutoTags = TagFilterView::NoToggleAuto;
            break;
        }
        case 22:   // Toggle auto Childs tags.
        {
            d->toggleAutoTags = TagFilterView::Children;
            break;
        }
        case 23:   // Toggle auto Parents tags.
        {
            d->toggleAutoTags = TagFilterView::Parents;
            break;
        }
        case 24:   // Toggle auto Childs and Parents tags.
        {
            d->toggleAutoTags = TagFilterView::ChildrenAndParents;
            break;
        }
        default:
            break;
    }

    if ( choice > 100 )
    {
        tagNew(album, d->ABCMenu->text( choice ), "tag-people" );
    }

    delete d->ABCMenu;
    d->ABCMenu = 0;
}

void ImageDescEditTab::slotABCContextMenu()
{
    d->ABCMenu->clear();

    int counter = 100;
    KABC::AddressBook* ab = KABC::StdAddressBook::self();
    QStringList names;
    for ( KABC::AddressBook::Iterator it = ab->begin(); it != ab->end(); ++it )
    {
        names.push_back(it->formattedName());
    }

    qHeapSort(names);

    for ( QStringList::Iterator it = names.begin(); it != names.end(); ++it )
    {
        QString name = *it;
        if ( !name.isNull() )
            d->ABCMenu->insertItem( name, ++counter );
    }

    if (counter == 100)
    {
        d->ABCMenu->insertItem( i18n("No AddressBook Entries Found"), ++counter );
        d->ABCMenu->setItemEnabled( counter, false );
    }
}

void ImageDescEditTab::slotMoreMenu()
{
    d->moreMenu->clear();

    if (singleSelection())
    {
        d->moreMenu->insertItem(i18n("Read metadata from file to database"), this, SLOT(slotReadFromFileMetadataToDatabase()));
        int writeActionId = d->moreMenu->insertItem(i18n("Write metadata to each file"), this, SLOT(slotWriteToFileMetadataFromDatabase()));
        // we do not need a "Write to file" action here because the apply button will do just that
        // if selection is a single file.
        // Adding the option will confuse users: Does the apply button not write to file?
        // Removing the option will confuse users: There is not option to write to file! (not visible in single selection)
        // Disabling will confuse users: Why is it disabled?
        d->moreMenu->setItemEnabled(writeActionId, false);
    }
    else
    {
        // We need to make clear that this action is different from the Apply button,
        // which saves the same changes to all files. These batch operations operate on each single file.
        d->moreMenu->insertItem(i18n("Read metadata from each file to database"), this, SLOT(slotReadFromFileMetadataToDatabase()));
        d->moreMenu->insertItem(i18n("Write metadata to each file"), this, SLOT(slotWriteToFileMetadataFromDatabase()));
    }
}

void ImageDescEditTab::tagNew(TAlbum* parAlbum, const QString& _title, const QString& _icon)
{
    if (!parAlbum)
        return;

    QString title           = _title;
    QString icon            = _icon;
    AlbumManager *albumMan_ = AlbumManager::instance();

    if (title.isNull())
    {
        if (!TagCreateDlg::tagCreate(kapp->activeWindow(), parAlbum, title, icon))
            return;
    }

    QString errMsg;
    TAlbum* album = albumMan_->createTAlbum(parAlbum, title, icon, errMsg);

    if (!album)
    {
        KMessageBox::error(this, errMsg);
    }
    else
    {
        TAlbumCheckListItem* viewItem = (TAlbumCheckListItem*)album->extraData(this);
        if (viewItem)
        {
            viewItem->setOn(true);
            d->tagsView->setSelected(viewItem, true);
            d->tagsView->ensureItemVisible(viewItem);
        }
    }
}

void ImageDescEditTab::tagDelete(TAlbum *album)
{
    if (!album || album->isRoot())
        return;

    AlbumManager *albumMan = AlbumManager::instance();

    if (album == albumMan->currentAlbum() ||
        album->isAncestorOf(albumMan->currentAlbum()))
    {
        KMessageBox::error(this, i18n("You are currently viewing items in the "
                                      "tag '%1' that you are about to delete. "
                                      "You will need to apply change first "
                                      "if you want to delete the tag." )
                           .arg(album->title()));
        return;
    }

    int result = KMessageBox::warningContinueCancel(this, i18n("Delete '%1' tag?")
                                                    .arg(album->title()),i18n("Delete Tag"),
                                                    KGuiItem(i18n("Delete"), "editdelete"));

    if (result == KMessageBox::Continue)
    {
        QString errMsg;
        if (!albumMan->deleteTAlbum(album, errMsg))
            KMessageBox::error(this, errMsg);
    }
}

void ImageDescEditTab::tagEdit(TAlbum* album)
{
    if (!album || album->isRoot())
        return;

    QString title;
    QString icon;

    if (!TagEditDlg::tagEdit(kapp->activeWindow(), album, title, icon))
        return;

    AlbumManager *albumMan = AlbumManager::instance();
    if (album->title() != title)
    {
        QString errMsg;
        if (!albumMan->renameTAlbum(album, title, errMsg))
        {
            KMessageBox::error(this, errMsg);
            return;
        }
    }

    if (album->icon() != icon)
    {
        QString errMsg;
        if (!albumMan->updateTAlbumIcon(album, icon, 0, errMsg))
        {
            KMessageBox::error(this, errMsg);
        }
    }
}

void ImageDescEditTab::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::TAG)
        return;

    TAlbumCheckListItem* viewItem = 0;

    TAlbum* tag = (TAlbum*)a;

    if (tag->isRoot())
    {
        viewItem = new TAlbumCheckListItem(d->tagsView, tag);
    }
    else
    {
        QCheckListItem* parentItem = (QCheckListItem*)(tag->parent()->extraData(this));

        if (!parentItem)
        {
            DWarning() << k_funcinfo << "Failed to find parent for Tag " << tag->title()
                       << endl;
            return;
        }

        viewItem = new TAlbumCheckListItem(parentItem, tag);
    }

    if (viewItem)
    {
        viewItem->setOpen(true);
        tag->setExtraData(this, viewItem);
        setTagThumbnail(tag);
    }
}

void ImageDescEditTab::slotAlbumDeleted(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    TAlbum* album = (TAlbum*)a;

    QCheckListItem* viewItem = (QCheckListItem*)(album->extraData(this));
    delete viewItem;
    album->removeExtraData(this);
    d->hub.setTag(album, false, MetadataHub::MetadataDisjoint);
}

void ImageDescEditTab::slotAlbumsCleared()
{
    d->tagsView->clear();
}

void ImageDescEditTab::slotAlbumIconChanged(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    setTagThumbnail((TAlbum *)a);
}

void ImageDescEditTab::slotAlbumMoved(TAlbum* tag, TAlbum* newParent)
{
    if (!tag || !newParent)
        return;

    QCheckListItem* item = (QCheckListItem*)tag->extraData(this);
    if (!item)
        return;

    if (item->parent())
    {
        QListViewItem* oldPItem = item->parent();
        oldPItem->takeItem(item);
    }
    else
    {
        d->tagsView->takeItem(item);
    }

    QCheckListItem* newPItem = (QCheckListItem*)newParent->extraData(this);
    if (newPItem)
        newPItem->insertItem(item);
    else
        d->tagsView->insertItem(item);
}

void ImageDescEditTab::slotAlbumRenamed(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    TAlbum* album = (TAlbum*)a;

    QCheckListItem* viewItem = (QCheckListItem*)(album->extraData(this));
    if (!viewItem)
    {
        DWarning() << "Failed to find view item for Tag "
                   << album->title() << endl;
        return;
    }

    viewItem->setText(0, album->title());
}

void ImageDescEditTab::toggleChildTags(TAlbum *album, bool b)
{
    if (!album)
        return;

    AlbumIterator it(album);
    while ( it.current() )
    {
        TAlbum *ta                = (TAlbum*)it.current();
        TAlbumCheckListItem *item = (TAlbumCheckListItem*)ta->extraData(this);
        if (item)
            if (item->isVisible())
                item->setOn(b);
        ++it;
    }
}

void ImageDescEditTab::toggleParentTags(TAlbum *album, bool b)
{
    if (!album)
        return;

    QListViewItemIterator it(d->tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
        if (item->isVisible())
        {
            if (!item->m_album)
                continue;
            if (item->m_album == album->parent())
            {
                item->setOn(b);
                toggleParentTags(item->m_album , b);
            }
        }
        ++it;
    }
}

void ImageDescEditTab::setTagThumbnail(TAlbum *album)
{
    if(!album)
        return;

    QCheckListItem* item = (QCheckListItem*) album->extraData(this);

    if(!item)
        return;

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
    QPixmap icon;
    if (!loader->getTagThumbnail(album, icon))
    {
        if (icon.isNull())
        {
            item->setPixmap(0, loader->getStandardTagIcon(album, AlbumThumbnailLoader::SmallerSize));
        }
        else
        {
            item->setPixmap(0, icon);
        }
    }
}

void ImageDescEditTab::slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail)
{
    if(!album || album->type() != Album::TAG)
        return;

    // update item in tags tree
    QCheckListItem* item = (QCheckListItem*)album->extraData(this);

    if(!item)
        return;

    item->setPixmap(0, thumbnail);

    // update item in recent tags popup menu, if found therein
    QPopupMenu *menu = d->recentTagsBtn->popup();
    if (menu->indexOf(album->id()) != -1)
    {
        menu->changeItem(album->id(), thumbnail, menu->text(album->id()));
    }
}

void ImageDescEditTab::slotThumbnailLost(Album *)
{
    // we already set the standard icon before loading
}

void ImageDescEditTab::slotReloadThumbnails()
{
    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* tag  = (TAlbum*)(*it);
        setTagThumbnail(tag);
    }
}

void ImageDescEditTab::slotImageTagsChanged(Q_LLONG imageId)
{
    // don't lose modifications
    if (d->ignoreImageAttributesWatch || d->modified)
        return;

    reloadForMetadataChange(imageId);
}

void ImageDescEditTab::slotImagesChanged(int albumId)
{
    if (d->ignoreImageAttributesWatch || d->modified)
        return;

    Album *a = AlbumManager::instance()->findAlbum(albumId);
    if (d->currInfos.isEmpty() || !a || a->isRoot() || a->type() != Album::TAG)
        return;

    setInfos(d->currInfos);
}

void ImageDescEditTab::slotImageRatingChanged(Q_LLONG imageId)
{
    if (d->ignoreImageAttributesWatch || d->modified)
        return;

    reloadForMetadataChange(imageId);
}

void ImageDescEditTab::slotImageCaptionChanged(Q_LLONG imageId)
{
    if (d->ignoreImageAttributesWatch || d->modified)
        return;

    reloadForMetadataChange(imageId);
}

void ImageDescEditTab::slotImageDateChanged(Q_LLONG imageId)
{
    if (d->ignoreImageAttributesWatch || d->modified)
        return;

    reloadForMetadataChange(imageId);
}

// private common code for above methods
void ImageDescEditTab::reloadForMetadataChange(Q_LLONG imageId)
{
    if (d->currInfos.isEmpty())
        return;

    if (singleSelection())
    {
        if (d->currInfos.first()->id() == imageId)
            setInfos(d->currInfos);
    }
    else
    {
        // if image id is in our list, update
        for (ImageInfo *info = d->currInfos.first(); info; info = d->currInfos.next())
        {
            if (info->id() == imageId)
            {
                setInfos(d->currInfos);
                return;
            }
        }
    }
}

void ImageDescEditTab::updateRecentTags()
{
    QPopupMenu *menu = d->recentTagsBtn->popup();
    menu->clear();
    
    AlbumManager* albumMan = AlbumManager::instance();
    IntList recentTags     = albumMan->albumDB()->getRecentlyAssignedTags();

    if (recentTags.isEmpty())
    {
        menu->insertItem(i18n("No Recently Assigned Tags"), 0);
        menu->setItemEnabled(0, false);
    }
    else
    {
        for (IntList::const_iterator it = recentTags.begin();
             it != recentTags.end(); ++it)
        {
            TAlbum* album = albumMan->findTAlbum(*it);
            if (album)
            {
                AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
                QPixmap icon;
                if (!loader->getTagThumbnail(album, icon))
                {
                    if (icon.isNull())
                    {
                        icon = loader->getStandardTagIcon(album, AlbumThumbnailLoader::SmallerSize);
                    }
                }
                QString text = album->title() + " (" + ((TAlbum*)album->parent())->prettyURL() + ')';
                menu->insertItem(icon, text, album->id());
            }
        }
    }
}

void ImageDescEditTab::slotRecentTagsMenuActivated(int id)
{
    AlbumManager* albumMan = AlbumManager::instance();
    
    if (id > 0)
    {
        TAlbum* album = albumMan->findTAlbum(id);
        if (album)
        {
            TAlbumCheckListItem* viewItem = (TAlbumCheckListItem*)album->extraData(this);
            if (viewItem)
            {
                viewItem->setOn(true);
                d->tagsView->setSelected(viewItem, true);
                d->tagsView->ensureItemVisible(viewItem);
            }
        }
    }
}

void ImageDescEditTab::slotTagsSearchChanged()
{
    //TODO: this will destroy assigned-tags filtering. Unify in one method.
    QString search(d->tagsSearchEdit->text());
    search = search.lower();

    bool atleastOneMatch = false;

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* tag  = (TAlbum*)(*it);

        // don't touch the root Tag
        if (tag->isRoot())
            continue;

        bool match = tag->title().lower().contains(search);
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = tag->parent();
            while (parent && !parent->isRoot())
            {
                if (parent->title().lower().contains(search))
                {
                    match = true;
                    break;
                }

                parent = parent->parent();
            }
        }

        if (!match)
        {
            // check if any of the children match the search
            AlbumIterator it(tag);
            while (it.current())
            {
                if ((*it)->title().lower().contains(search))
                {
                    match = true;
                    break;
                }
                ++it;
            }
        }

        TAlbumCheckListItem* viewItem = (TAlbumCheckListItem*)(tag->extraData(this));

        if (match)
        {
            atleastOneMatch = true;

            if (viewItem)
                viewItem->setVisible(true);
        }
        else
        {
            if (viewItem)
            {
                viewItem->setVisible(false);
            }
        }
    }

    if (search.isEmpty())
    {
        d->tagsSearchEdit->unsetPalette();
        TAlbum* root = AlbumManager::instance()->findTAlbum(0);
        TAlbumCheckListItem* rootItem = (TAlbumCheckListItem*)(root->extraData(this));
        if (rootItem)
            rootItem->setText(0, root->title());
    }
    else
    {
        TAlbum* root = AlbumManager::instance()->findTAlbum(0);
        TAlbumCheckListItem* rootItem = (TAlbumCheckListItem*)(root->extraData(this));
        if (rootItem)
            rootItem->setText(0, i18n("Found Tags"));

        QPalette pal = d->tagsSearchEdit->palette();
        pal.setColor(QPalette::Active, QColorGroup::Base,
                     atleastOneMatch ?  QColor(200,255,200) :
                     QColor(255,200,200));
        d->tagsSearchEdit->setPalette(pal);
    }
}

void ImageDescEditTab::slotAssignedTagsToggled(bool t)
{
    //TODO: this will destroy name filtering. Unify in one method.
    QListViewItemIterator it(d->tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
        TAlbum *tag               = item->m_album;
        if (tag)
        {
            if (!tag->isRoot())
            {
                if (t)
                {
                    MetadataHub::TagStatus status = d->hub.tagStatus(item->m_album);
                    bool tagAssigned = (status == MetadataHub::MetadataAvailable && status.hasTag)
                                        || status == MetadataHub::MetadataDisjoint;
                    item->setVisible(tagAssigned);

                    if (tagAssigned)
                    {
                        Album* parent = tag->parent();
                        while (parent && !parent->isRoot())
                        {
                            QCheckListItem *pitem = (QCheckListItem*)parent->extraData(this);
                            pitem->setVisible(true);
                            parent = parent->parent();
                        }
                    }
                }
                else
                {
                    item->setVisible(true);
                }
            }
        }
        ++it;
    }

    // correct visibilities afterwards:
    // As QListViewItem::setVisible works recursively on all it's children
    // we have to correct this
    if (t)
    {
        it = d->tagsView;
        while (it.current())
        {
            TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
            TAlbum *tag               = item->m_album;
            if (tag)
            {
                if (!tag->isRoot())
                {
                    // only if the current item is not marked as tagged, check all children 
                    MetadataHub::TagStatus status = d->hub.tagStatus(item->m_album);
                    bool tagAssigned = (status == MetadataHub::MetadataAvailable && status.hasTag)
                                        || status == MetadataHub::MetadataDisjoint;
                    if (!tagAssigned)
                    {
                        bool somethingIsSet         = false;
                        QListViewItem* nextSibling  = (*it)->nextSibling();
                        QListViewItemIterator tmpIt = it;
                        ++tmpIt;
                        while (*tmpIt != nextSibling )
                        {
                            TAlbumCheckListItem* tmpItem = dynamic_cast<TAlbumCheckListItem*>(tmpIt.current());
                            MetadataHub::TagStatus tmpStatus = d->hub.tagStatus(tmpItem->m_album);
                            bool tmpTagAssigned = (tmpStatus == MetadataHub::MetadataAvailable && tmpStatus.hasTag)
                                                || tmpStatus == MetadataHub::MetadataDisjoint;
                            if(tmpTagAssigned)
                            {
                                somethingIsSet = true;
                            }
                            ++tmpIt;
                        }
                        if (!somethingIsSet) 
                        {
                            item->setVisible(false);
                        }
                    }
                }
            }
            ++it;
        }
    }

    TAlbum *root                  = AlbumManager::instance()->findTAlbum(0);
    TAlbumCheckListItem *rootItem = (TAlbumCheckListItem*)(root->extraData(this));
    if (rootItem)
    {
        if (t)
            rootItem->setText(0, i18n("Assigned Tags"));
        else
            rootItem->setText(0, root->title());
    }
}

}  // NameSpace Digikam

