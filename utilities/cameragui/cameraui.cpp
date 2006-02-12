/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-16
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

// C Ansi includes.

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

// Qt includes.

#include <qvgroupbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qiconview.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qpopupmenu.h>
#include <qsplitter.h>
#include <qpixmap.h>
#include <qframe.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qprogressbar.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qfile.h>
#include <qfileinfo.h>

// KDE includes.

#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <khelpmenu.h>
#include <kdebug.h>
#include <kcalendarsystem.h>
#include <kurllabel.h>

// Local includes.

#include "imagepropertiessidebarcamgui.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "album.h"
#include "albumselectdialog.h"
#include "renamecustomizer.h"
#include "animwidget.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "cameracontroller.h"
#include "cameraui.h"
#include "scanlib.h"

namespace Digikam
{

class CameraUIPriv
{
public:

    CameraUIPriv()
    {
        showAdvanced = false;
        busy         = false;
        helpBtn      = 0;
        closeBtn     = 0;
        downloadBtn  = 0;
        deleteBtn    = 0;
        advBtn       = 0;
        downloadMenu = 0;
        deleteMenu   = 0;
        cancelBtn    = 0;
        splitter     = 0;
        rightSidebar = 0;
    }

    bool                          showAdvanced;
    bool                          busy;

    QStringList                   foldersToScan;

    QPushButton*                  helpBtn;
    QPushButton*                  closeBtn;
    QPushButton*                  downloadBtn;
    QPushButton*                  deleteBtn;
    QPushButton*                  advBtn;

    QPopupMenu*                   downloadMenu;
    QPopupMenu*                   deleteMenu;

    QToolButton*                  cancelBtn;

    QVBox*                        advBox;

    QCheckBox*                    autoRotateCheck;
    QCheckBox*                    autoAlbumCheck;

    QLabel*                       status;

    QProgressBar*                 progress;

    QSplitter                    *splitter;

    KURL                          lastDestURL;

    CameraController*             controller;

    CameraIconView*               view;

    RenameCustomizer*             renameCustomizer;

    AnimWidget*                   anim;

    ImagePropertiesSideBarCamGui *rightSidebar;
};    

CameraUI::CameraUI(QWidget* parent, const QString& title,
                   const QString& model, const QString& port,
                   const QString& path)
        : KDialog(parent, 0, false, WDestructiveClose)
{
    d = new CameraUIPriv;

    // -- setup view -----------------------------------------
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QGroupBox* viewBox = new QGroupBox(0, Qt::Horizontal, title, this);

    QGridLayout* viewBoxLayout = new QGridLayout(viewBox->layout(), 1, 3, 
                                     KDialog::spacingHint());
    viewBoxLayout->setColStretch( 0, 0 );
    viewBoxLayout->setColStretch( 1, 3 );
    viewBoxLayout->setColStretch( 2, 1 );
    viewBoxLayout->setColStretch( 3, 0 );

    QWidget* widget   = new QWidget(this);
    QHBoxLayout *hlay = new QHBoxLayout(widget);
    d->splitter       = new QSplitter(widget);
    d->view           = new CameraIconView(this, d->splitter);
    d->rightSidebar   = new ImagePropertiesSideBarCamGui(widget, "CameraGui Sidebar Right", d->splitter,
                                                         Digikam::Sidebar::Right);
    hlay->addWidget(d->splitter);
    hlay->addWidget(d->rightSidebar);
    
    viewBoxLayout->addMultiCellWidget(widget, 0, 0, 0, 3);
    d->splitter->setOpaqueResize(false);
    d->rightSidebar->loadViewState();
        
    // -------------------------------------------------------------------------

    d->cancelBtn = new QToolButton(viewBox);
    QIconSet iconSet = kapp->iconLoader()->loadIconSet("stop", KIcon::Toolbar, 22);
    d->cancelBtn->setText(i18n("Cancel"));
    d->cancelBtn->setIconSet(iconSet);
    d->cancelBtn->setEnabled(false);
    viewBoxLayout->addMultiCellWidget(d->cancelBtn, 1, 1, 0, 0);
    
    d->status = new QLabel(viewBox);
    viewBoxLayout->addMultiCellWidget(d->status, 1, 1, 1, 1);
    d->progress = new QProgressBar(viewBox);
    viewBoxLayout->addMultiCellWidget(d->progress, 1, 1, 2, 2);
    d->progress->hide();

    QFrame *frame = new QFrame(viewBox);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( frame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );

    KURLLabel *pixmapLogo = new KURLLabel( frame );
    pixmapLogo->setText(QString::null);
    pixmapLogo->setURL("http://www.digikam.org");
    pixmapLogo->setScaledContents( false );
    pixmapLogo->setPaletteBackgroundColor( QColor(201, 208, 255) );
    QToolTip::add(pixmapLogo, i18n("Visit digiKam project website"));
    layout->addWidget( pixmapLogo );
    KGlobal::dirs()->addResourceType("digikamlogo", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("digikamlogo", "digikamlogo.png");
    pixmapLogo->setPixmap( QPixmap( directory + "digikamlogo.png" ) );
    
    d->anim = new AnimWidget(frame);
    layout->addWidget( d->anim );

    viewBoxLayout->addMultiCellWidget(frame, 1, 1, 3, 3);
    
    mainLayout->addWidget(viewBox);

    // -------------------------------------------------------------------------
    
    QHBoxLayout* btnLayout = new QHBoxLayout(0, KDialog::marginHint(), KDialog::spacingHint());

    QPushButton* helpBtn;
    QPushButton* selectBtn;
    
    d->advBtn      = new QPushButton(i18n("&Advanced %1").arg(">>"), this);
    helpBtn        = new QPushButton(i18n("&Help"), this);
    selectBtn      = new QPushButton(i18n("&Select"), this);
    d->downloadBtn = new QPushButton(i18n("&Download"), this);
    d->deleteBtn   = new QPushButton(i18n("D&elete"), this);
    d->closeBtn    = new QPushButton(i18n("&Close"), this);
    
    btnLayout->addWidget(d->advBtn);
    btnLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
    btnLayout->addWidget(helpBtn);
    btnLayout->addWidget(selectBtn);
    btnLayout->addWidget(d->downloadBtn);
    btnLayout->addWidget(d->deleteBtn);
    btnLayout->addWidget(d->closeBtn);

    mainLayout->addLayout(btnLayout);
    
    // -------------------------------------------------------------------------

    d->advBox = new QVBox(this);
    d->advBox->setSpacing(KDialog::spacingHint());
    d->showAdvanced = false;
    d->advBox->hide();

    d->renameCustomizer = new RenameCustomizer(d->advBox);
    d->view->setRenameCustomizer(d->renameCustomizer);

    QVGroupBox* exifBox = new QVGroupBox(i18n("Use Camera-provided Information (EXIF)"),
                                         d->advBox);
    d->autoRotateCheck = new QCheckBox(i18n("Automatically rotate/flip using "
                                           "camera-provided information (EXIF)"),
                                      exifBox);
    d->autoAlbumCheck = new QCheckBox(i18n("Download photos into automatically "
                                          "created date-based sub-albums of "
                                          "destination album"),
                                      exifBox);
    
    mainLayout->addWidget(d->advBox);

    // -------------------------------------------------------------------------
    // About popupmenu button using a slot for calling the camera interface
    // anchor in Digikam handbook.
    
    KHelpMenu* helpMenu = new KHelpMenu(this, KApplication::kApplication()->aboutData(),
                                        false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Digikam Handbook"),
                                 this, SLOT(slotHelp()), 0, -1, 0);
    helpBtn->setPopup( helpMenu->menu() );

    // -------------------------------------------------------------------------
    
    QPopupMenu* selectMenu = new QPopupMenu(this);
    selectMenu->insertItem(i18n("Select &All"),
                           d->view, SLOT(slotSelectAll()));
    selectMenu->insertItem(i18n("Select N&one"),
                           d->view, SLOT(slotSelectNone()));
    selectMenu->insertItem(i18n("&Invert Selection"),
                           d->view, SLOT(slotSelectInvert()));
    selectMenu->insertSeparator();
    selectMenu->insertItem(i18n("Select &New Items"),
                           d->view, SLOT(slotSelectNew()));
    selectBtn->setPopup(selectMenu);    

    // -------------------------------------------------------------------------
        
    d->downloadMenu = new QPopupMenu(this);
    d->downloadMenu->insertItem(i18n("Download Selected"),
                               this, SLOT(slotDownloadSelected()),
                               0, 0);
    d->downloadMenu->insertItem(i18n("Download All"),
                               this, SLOT(slotDownloadAll()),
                               0, 1);
    d->downloadMenu->setItemEnabled(0, false);
    d->downloadBtn->setPopup(d->downloadMenu);

    // -------------------------------------------------------------------------
    
    d->deleteMenu = new QPopupMenu(this);
    d->deleteMenu->insertItem(i18n("Delete Selected"),
                             this, SLOT(slotDeleteSelected()),
                             0, 0);
    d->deleteMenu->insertItem(i18n("Delete All"),
                             this, SLOT(slotDeleteAll()),
                             0, 1);
    d->deleteMenu->setItemEnabled(0, false);
    d->deleteBtn->setPopup(d->deleteMenu);

    // -------------------------------------------------------------------------
    
    connect(d->closeBtn, SIGNAL(clicked()),
            this, SLOT(close()));

    connect(d->advBtn, SIGNAL(clicked()),
            this, SLOT(slotToggleAdvanced()));

    connect(pixmapLogo, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(slotProcessURL(const QString&)));     

    // -------------------------------------------------------------------------

    connect(d->view, SIGNAL(signalSelected(CameraIconViewItem*, bool)),
            this, SLOT(slotItemsSelected(CameraIconViewItem*, bool)));

    connect(d->view, SIGNAL(signalFileView(CameraIconViewItem*)),
            this, SLOT(slotFileView(CameraIconViewItem*)));

    connect(d->view, SIGNAL(signalDownload()),
            this, SLOT(slotDownloadSelected()));

    connect(d->view, SIGNAL(signalDelete()),
            this, SLOT(slotDeleteSelected()));

    // -------------------------------------------------------------------------
    
    connect(d->rightSidebar, SIGNAL(signalFirstItem()),
            this, SLOT(slotFirstItem()));
    
    connect(d->rightSidebar, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));
                
    connect(d->rightSidebar, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));                
    
    connect(d->rightSidebar, SIGNAL(signalLastItem()),
            this, SLOT(slotLastItem()));                

    // -- Read settings --------------------------------------------------

    readSettings();
    
    // -- camera controller -----------------------------------------------
    
    d->controller = new CameraController(this, model, port, path);

    connect(d->controller, SIGNAL(signalConnected(bool)),
            this, SLOT(slotConnected(bool)));

    connect(d->controller, SIGNAL(signalInfoMsg(const QString&)),
            d->status, SLOT(setText(const QString&)));

    connect(d->controller, SIGNAL(signalErrorMsg(const QString&)),
            this, SLOT(slotErrorMsg(const QString&)));

    connect(d->controller, SIGNAL(signalBusy(bool)),
            this, SLOT(slotBusy(bool)));

    connect(d->controller, SIGNAL(signalFolderList(const QStringList&)),
            this, SLOT(slotFolderList(const QStringList&)));

    connect(d->controller, SIGNAL(signalFileList(const GPItemInfoList&)),
            this, SLOT(slotFileList(const GPItemInfoList&)));

    connect(d->controller, SIGNAL(signalThumbnail(const QString&, const QString&, const QImage&)),
            this, SLOT(slotThumbnail(const QString&, const QString&, const QImage&)));

    connect(d->controller, SIGNAL(signalDownloaded(const QString&, const QString&)),
            this, SLOT(slotDownloaded(const QString&, const QString&)));

    connect(d->controller, SIGNAL(signalSkipped(const QString&, const QString&)),
            this, SLOT(slotSkipped(const QString&, const QString&)));

    connect(d->controller, SIGNAL(signalDeleted(const QString&, const QString&)),
            this, SLOT(slotDeleted(const QString&, const QString&)));

    connect(d->controller, SIGNAL(signalExifFromFile(const QString&, const QString&)),
            this, SLOT(slotExifFromFile(const QString&, const QString&)));
    
    connect(d->controller, SIGNAL(signalExifData(const QByteArray&)),
            this, SLOT(slotExifFromData(const QByteArray&)));

    connect(d->cancelBtn, SIGNAL(clicked()),
            d->controller, SLOT(slotCancel()));

    d->view->setFocus();
    QTimer::singleShot(0, d->controller, SLOT(slotConnect()));
}

CameraUI::~CameraUI()
{
    delete d->rightSidebar;
    delete d;
}

void CameraUI::slotProcessURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

bool CameraUI::isBusy() const
{
    return d->busy;
}

void CameraUI::closeEvent(QCloseEvent* e)
{
    // When a directory is created, a watch is put on it to spot new files
    // but it can occur that the file is copied there before the watch is
    // completely setup. That is why as an extra safeguard run scanlib
    // over the folders we used. Bug: 119201
    ScanLib sLib;
    for (QStringList::iterator it = d->foldersToScan.begin();
         it != d->foldersToScan.end(); ++it)
    {
        kdDebug() << "Scanning " << (*it) << endl;
        sLib.findMissingItems( (*it) );
    }
    
    //---------------------------------------------------

    if(!d->lastDestURL.isEmpty())
        emit signalLastDestination(d->lastDestURL);

    delete d->controller;
    saveSettings();
    e->accept();
}

void CameraUI::slotHelp()
{
    KApplication::kApplication()->invokeHelp("camerainterface.anchor",
                                             "digikam");
}

void CameraUI::slotBusy(bool val)
{
    if (!val)
    {
        if (!d->busy)
            return;

        d->busy = false;
        d->cancelBtn->setEnabled(false);
        d->downloadBtn->setEnabled(true);
        d->deleteBtn->setEnabled(true);
        d->anim->stop();
        d->status->setText(i18n("Ready"));
        d->progress->hide();
    }
    else
    {
        if (d->busy)
            return;
        
        if (!d->anim->running())
            d->anim->start();
        d->busy = true;
        d->cancelBtn->setEnabled(true);
        d->downloadBtn->setEnabled(false);
        d->deleteBtn->setEnabled(false);
    }
}

void CameraUI::slotConnected(bool val)
{
    if (!val)
    {
      if (KMessageBox::warningYesNo(this,
                                    i18n("Failed to connect to camera. "
                                         "Please make sure its connected "
                                         "properly and turned on. "
                                         "Would you like to try again?"), 
                                    i18n("Connection Failed"),
                                    i18n("Retry"),
                                    i18n("Abort"))
          == KMessageBox::Yes)
          QTimer::singleShot(0, d->controller, SLOT(slotConnect()));
      else
          close();
    }
    else
    {
        d->controller->listFolders();
    }
}

void CameraUI::slotFolderList(const QStringList& folderList)
{
    for (QStringList::const_iterator it = folderList.begin();
         it != folderList.end(); ++it)
    {
        d->controller->listFiles(*it);
    }
}

void CameraUI::slotFileList(const GPItemInfoList& fileList)
{
    for (GPItemInfoList::const_iterator it = fileList.begin();
         it != fileList.end(); ++it)
    {
        d->view->addItem(*it);
        d->controller->getThumbnail((*it).folder, (*it).name);
    }
}

void CameraUI::slotThumbnail(const QString& folder, const QString& file,
                             const QImage& thumbnail)
{
    d->view->setThumbnail(folder, file, thumbnail);
}

void CameraUI::slotToggleAdvanced()
{
    d->showAdvanced = !d->showAdvanced;
    if (d->showAdvanced)
    {
        d->advBox->show();
        d->advBtn->setText(i18n("&Simple %1").arg("<<"));
    }
    else
    {
        d->advBox->hide();
        d->advBtn->setText(i18n("&Advanced %1").arg(">>"));
    }
}

void CameraUI::slotErrorMsg(const QString& msg)
{
    KMessageBox::error(this, msg);    
}

void CameraUI::slotDownloadSelected()
{
    slotDownload(true);
}

void CameraUI::slotDownloadAll()
{
    slotDownload(false);
}

void CameraUI::slotDownload(bool onlySelected)
{
    AlbumManager* man = AlbumManager::instance();

    Album* album = man->currentAlbum();
    if (album && album->type() != Album::PHYSICAL)
        album = 0;

    QString header(i18n("Select Destination Album for "
                        "Importing Camera Images"));

    QString newDirName;
    IconItem* firstItem = d->view->firstItem();
    if (firstItem)
    {
        CameraIconViewItem* iconItem =
            static_cast<CameraIconViewItem*>(firstItem);
        
        QDateTime date;
        date.setTime_t(iconItem->itemInfo()->mtime);
        newDirName = QString("%1, %2, %3")
                     .arg(KGlobal::locale()->calendar()->year(date.date()))
                     .arg(KGlobal::locale()->calendar()->monthName(date.date()))
                     .arg(KGlobal::locale()->calendar()->day(date.date()));
    }


    album = AlbumSelectDialog::selectAlbum(this,
                                           (PAlbum*)album,
                                           header,
                                           newDirName,
                                           d->autoAlbumCheck->isChecked());

    if (!album)
        return;

    KURL url;
    url.setPath(((PAlbum*)album)->folderPath());
    
    d->controller->downloadPrep();

    QString downloadName;
    QString name;
    QString folder;
    time_t  mtime;
    bool    autoRotate;
    bool    autoAlbum;

    autoRotate = d->autoRotateCheck->isChecked();
    autoAlbum  = d->autoAlbumCheck->isChecked();

    int total = 0;
    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        if (onlySelected && !(item->isSelected()))
            continue;

        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        folder = iconItem->m_itemInfo->folder;
        name   = iconItem->m_itemInfo->name;
        downloadName = iconItem->getDownloadName();
        mtime        = iconItem->m_itemInfo->mtime;
        
        KURL u(url);
        if (autoAlbum)
        {
            QDateTime date;
            date.setTime_t(mtime);
            QString dirName(date.toString("yyyy-MM-dd"));
            QString errMsg;
            if (!createAutoAlbum(url, dirName, date.date(), errMsg))
            {
                KMessageBox::error(this, errMsg);
                return;
            }

            u.addPath(dirName);
            d->foldersToScan.append(u.path());
            u.addPath(downloadName.isEmpty() ? name : downloadName);
        }
        else
        {
            d->foldersToScan.append(u.path());
            u.addPath(downloadName.isEmpty() ? name : downloadName);
        }
        d->controller->download(folder, name, u.path(), autoRotate);
        addFileExtension(QFileInfo(u.path()).extension(false));
        total++;
    }

    if (total <= 0)
        return;
    
    d->lastDestURL = url;
    d->progress->setProgress(0);
    d->progress->setTotalSteps(total);
    d->progress->show();
}

void CameraUI::slotDeleteSelected()
{
    QStringList folders;
    QStringList files;
    QStringList deleteList;
    
    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        if (iconItem->isSelected())
        {
            QString folder = iconItem->m_itemInfo->folder;
            QString file   = iconItem->m_itemInfo->name;
            folders.append(folder);
            files.append(file);
            deleteList.append(folder + QString("/") + file);
        }
    }

    if (folders.isEmpty())
        return;

    QString warnMsg(i18n("About to delete this image. "
                         "Deleted files are unrecoverable. "
                         "Are you sure?",
                         "About to delete these %n images. "
                         "Deleted files are unrecoverable. "
                         "Are you sure?",
                         deleteList.count()));
    if (KMessageBox::warningContinueCancelList(this, warnMsg,
                                               deleteList,
                                               i18n("Warning"),
                                               i18n("Delete"))
        ==  KMessageBox::Continue) {

        QStringList::iterator itFolder = folders.begin();
        QStringList::iterator itFile   = files.begin();
        for ( ; itFolder != folders.end(); ++itFolder, ++itFile)
        {
            d->controller->deleteFile(*itFolder, *itFile);
        }
    }
}

void CameraUI::slotDeleteAll()
{
    QStringList folders;
    QStringList files;
    QStringList deleteList;
    
    for (IconItem* item = d->view->firstItem(); item;
         item = item->nextItem())
    {
        CameraIconViewItem* iconItem = static_cast<CameraIconViewItem*>(item);
        QString folder = iconItem->m_itemInfo->folder;
        QString file   = iconItem->m_itemInfo->name;
        folders.append(folder);
        files.append(file);
        deleteList.append(folder + QString("/") + file);
    }

    if (folders.isEmpty())
        return;

    QString warnMsg(i18n("About to delete this image. "
                         "Deleted files are unrecoverable. "
                         "Are you sure?",
                         "About to delete these %n images. "
                         "Deleted files are unrecoverable. "
                         "Are you sure?",
                         deleteList.count()));
    if (KMessageBox::warningContinueCancelList(this, warnMsg,
                                               deleteList,
                                               i18n("Warning"),
                                               i18n("Delete"))
        ==  KMessageBox::Continue) {

        QStringList::iterator itFolder = folders.begin();
        QStringList::iterator itFile   = files.begin();
        for ( ; itFolder != folders.end(); ++itFolder, ++itFile)
        {
            d->controller->deleteFile(*itFolder, *itFile);
        }
    }
}

void CameraUI::slotFileView(CameraIconViewItem* item)
{
    d->controller->openFile(item->itemInfo()->folder,
                           item->itemInfo()->name);
}

void CameraUI::slotExifFromFile(const QString& folder, const QString& file)
{
    CameraIconViewItem* item = d->view->findItem(folder, file);
    d->rightSidebar->itemChanged(item->itemInfo(), KURL::KURL(folder + QString("/") + file), QByteArray(), d->view, item);
}

void CameraUI::slotExifFromData(const QByteArray& exifData)
{
    CameraIconViewItem* item = dynamic_cast<CameraIconViewItem*>(d->view->currentItem());
    d->rightSidebar->itemChanged(item->itemInfo(), KURL::KURL(), exifData, d->view, item);
}

void CameraUI::slotItemsSelected(CameraIconViewItem* item, bool selected)
{
    d->downloadMenu->setItemEnabled(0, selected);
    d->deleteMenu->setItemEnabled(0, selected);

    if (selected)
        d->controller->getExif(item->itemInfo()->folder, item->itemInfo()->name);
    else
        d->rightSidebar->slotNoCurrentItem();
}

void CameraUI::slotDownloaded(const QString& folder, const QString& file)
{
    CameraIconViewItem* iconItem = d->view->findItem(folder, file);
    if (iconItem)
    {
        iconItem->setDownloaded();

        if (iconItem->isSelected())
            slotItemsSelected(iconItem, true);
    }
    
    int curr = d->progress->progress();
    d->progress->setProgress(curr+1);
}

void CameraUI::slotSkipped(const QString&, const QString&)
{
    int curr = d->progress->progress();
    d->progress->setProgress(curr+1);
}

void CameraUI::slotDeleted(const QString& folder, const QString& file)
{
    d->view->removeItem(folder, file);
}

void CameraUI::readSettings()
{
    KConfig* config = kapp->config();

    int w, h;

    config->setGroup("Camera Settings");
    w = config->readNumEntry("Width", 500);
    h = config->readNumEntry("Height", 500);
    d->autoRotateCheck->setChecked(config->readBoolEntry("AutoRotate", true));
    d->autoAlbumCheck->setChecked(config->readBoolEntry("AutoAlbum", false));
    
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    if(config->hasKey("Splitter Sizes"))
        d->splitter->setSizes(config->readIntListEntry("Splitter Sizes"));
    else 
        d->view->setSizePolicy(rightSzPolicy);

    resize(w, h);
}

void CameraUI::saveSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("Camera Settings");
    config->writeEntry("Width", width());
    config->writeEntry("Height", height());
    config->writeEntry("AutoRotate", d->autoRotateCheck->isChecked());
    config->writeEntry("AutoAlbum", d->autoAlbumCheck->isChecked());
    config->writeEntry("Splitter Sizes", d->splitter->sizes());
    config->sync();
}

bool CameraUI::createAutoAlbum(const KURL& parentURL,
                               const QString& name,
                               const QDate& date,
                               QString& errMsg)
{
    KURL u(parentURL);
    u.addPath(name);

    // first stat to see if the album exists
    struct stat buf;
    if (::stat(QFile::encodeName(u.path()), &buf) == 0)
    {
        // now check if its really a directory
        if (S_ISDIR(buf.st_mode))
            return true;
        else
        {
            errMsg = i18n("A file with same name (%1) exists in folder %2")
                     .arg(name)
                     .arg(parentURL.path());
            return false;
        }
    }

    // looks like the directory does not exist, try to create it

    AlbumManager* aman = AlbumManager::instance();
    PAlbum* parent =  aman->findPAlbum(parentURL);
    if (!parent)
    {
        errMsg = i18n("Failed to find Album for path '%1'")
                 .arg(parentURL.path());
        return false;
    }

    return aman->createPAlbum(parent, name, QString(""),
                              date, QString(""), errMsg);
}

void CameraUI::addFileExtension(const QString& ext)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;

    if (settings->getImageFileFilter().contains(ext) ||
        settings->getMovieFileFilter().contains(ext) ||
        settings->getAudioFileFilter().contains(ext) ||
        settings->getRawFileFilter().contains(ext))
        return;

    settings->setImageFileFilter(settings->getImageFileFilter() +
                                 QString(" *.") + ext);
    emit signalAlbumSettingsChanged();
}

void CameraUI::slotFirstItem(void)
{
    CameraIconViewItem *currItem = dynamic_cast<CameraIconViewItem*>(d->view->firstItem());
    if (currItem) 
       d->view->setCurrentItem(currItem);
}

void CameraUI::slotPrevItem(void)
{
    IconItem* prevItem = 0;
    IconItem *currItem = d->view->currentItem();
    if (currItem) 
    {
       prevItem = currItem->prevItem();
       if (prevItem)
           d->view->setCurrentItem(prevItem);
    }
}

void CameraUI::slotNextItem(void)
{
    IconItem* nextItem = 0;
    IconItem *currItem = d->view->currentItem();
    if (currItem) 
    {
       nextItem = currItem->nextItem();
       if (nextItem)
           d->view->setCurrentItem(nextItem);
    }
}

void CameraUI::slotLastItem(void)
{
    CameraIconViewItem *currItem = dynamic_cast<CameraIconViewItem*>(d->view->lastItem());
    if (currItem) 
       d->view->setCurrentItem(currItem);
}

}  // namespace Digikam

#include "cameraui.moc"
