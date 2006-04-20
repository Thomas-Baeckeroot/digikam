/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-08
 * Description : A tab to display camera item informations
 *
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

// Qt includes.
 
#include <qlayout.h>
#include <qfile.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kfileitem.h>
#include <kdebug.h>
#include <kseparator.h>
#include <ksqueezedtextlabel.h>

// Local includes.

#include "gpiteminfo.h"
#include "navigatebarwidget.h"
#include "cameraitempropertiestab.h"

namespace Digikam
{

class CameraItemPropertiesTabPriv
{
public:

    CameraItemPropertiesTabPriv()
    {
        navigateBar            = 0;
        labelNewFileName       = 0;
        labelFolder            = 0;
        labelFileIsReadable    = 0;
        labelFileIsWritable    = 0;
        labelFileDate          = 0;
        labelFileSize          = 0;
        labelFileMime          = 0;
        labelImageDimensions   = 0;
        labelAlreadyDownloaded = 0;
    }

    KSqueezedTextLabel *labelNewFileName;
    KSqueezedTextLabel *labelFolder;
    KSqueezedTextLabel *labelFileIsReadable;
    KSqueezedTextLabel *labelFileIsWritable;
    KSqueezedTextLabel *labelFileDate;
    KSqueezedTextLabel *labelFileSize;
    KSqueezedTextLabel *labelFileMime;
    KSqueezedTextLabel *labelImageDimensions;
    KSqueezedTextLabel *labelAlreadyDownloaded;
    
    NavigateBarWidget  *navigateBar;
};

CameraItemPropertiesTab::CameraItemPropertiesTab(QWidget* parent, bool navBar)
                       : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new CameraItemPropertiesTabPriv;
    QGridLayout *topLayout = new QGridLayout(this, 14, 1, KDialog::marginHint(), KDialog::spacingHint());

    d->navigateBar = new NavigateBarWidget(this, navBar);
    topLayout->addMultiCellWidget(d->navigateBar, 0, 0, 0, 1);

    QLabel *title       = new QLabel(i18n("<u><i>Camera File Properties:</i></u>"), this);
    QLabel *folder      = new QLabel(i18n("<b>Folder</b>:"), this);
    QLabel *isReadable  = new QLabel(i18n("<b>Readable</b>:"), this);
    QLabel *isWritable  = new QLabel(i18n("<b>Writable</b>:"), this);
    QLabel *date        = new QLabel(i18n("<b>Date</b>:"), this);
    QLabel *size        = new QLabel(i18n("<b>Size</b>:"), this);
    QLabel *mime        = new QLabel(i18n("<b>Mime</b>:"), this);
    QLabel *dimensions  = new QLabel(i18n("<b>Dimensions</b>:"), this);
    KSeparator *line    = new KSeparator (Horizontal, this);
    QLabel *title2      = new QLabel(i18n("<u><i>Download Status:</i></u>"), this);
    QLabel *newFileName = new QLabel(i18n("<b>New Name</b>:"), this);
    QLabel *downloaded  = new QLabel(i18n("<b>Downloaded</b>:"), this);
                            
    d->labelFolder            = new KSqueezedTextLabel(0, this);
    d->labelFileIsReadable    = new KSqueezedTextLabel(0, this);
    d->labelFileIsWritable    = new KSqueezedTextLabel(0, this);
    d->labelFileDate          = new KSqueezedTextLabel(0, this);
    d->labelFileSize          = new KSqueezedTextLabel(0, this);
    d->labelFileMime          = new KSqueezedTextLabel(0, this);
    d->labelImageDimensions   = new KSqueezedTextLabel(0, this);
    d->labelNewFileName       = new KSqueezedTextLabel(0, this);
    d->labelAlreadyDownloaded = new KSqueezedTextLabel(0, this);

    topLayout->setRowSpacing( 0, KDialog::spacingHint() );    
    topLayout->addMultiCellWidget(title, 1, 1, 0, 1);
    topLayout->setRowStretch(2, 0);
    topLayout->addMultiCellWidget(folder, 3, 3, 0, 0);
    topLayout->addMultiCellWidget(d->labelFolder, 3, 3, 1, 1);
    topLayout->addMultiCellWidget(isReadable, 4, 4, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileIsReadable, 4, 4, 1, 1);
    topLayout->addMultiCellWidget(isWritable, 5, 5, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileIsWritable, 5, 5, 1, 1);
    topLayout->addMultiCellWidget(date, 6, 6, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileDate, 6, 6, 1, 1);
    topLayout->addMultiCellWidget(size, 7, 7, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileSize, 7, 7, 1, 1);
    topLayout->addMultiCellWidget(mime, 8, 8, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileMime, 8, 8, 1, 1);
    topLayout->addMultiCellWidget(dimensions, 9, 9, 0, 0);
    topLayout->addMultiCellWidget(d->labelImageDimensions, 9, 9, 1, 1);

    topLayout->addMultiCellWidget(line, 10, 10, 0, 1);
    topLayout->addMultiCellWidget(title2, 11, 11, 0, 1);
    topLayout->addMultiCellWidget(newFileName, 12, 12, 0, 0);
    topLayout->addMultiCellWidget(d->labelNewFileName, 12, 12, 1, 1);
    topLayout->addMultiCellWidget(downloaded, 13, 13, 0, 0);
    topLayout->addMultiCellWidget(d->labelAlreadyDownloaded, 13, 13, 1, 1);
    topLayout->setRowStretch(14, 10);
    topLayout->setColStretch(1, 10);
            
    connect(d->navigateBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));

    connect(d->navigateBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->navigateBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->navigateBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
}

CameraItemPropertiesTab::~CameraItemPropertiesTab()
{
    delete d;
}

void CameraItemPropertiesTab::setCurrentItem(const GPItemInfo* itemInfo, int itemType, 
                                             const QString &newFileName)
{
    if (!itemInfo)
    {
        d->navigateBar->setFileName();
        d->labelNewFileName->setText(QString::null);
        d->labelFolder->setText(QString::null);
        d->labelFileIsReadable->setText(QString::null);
        d->labelFileIsWritable->setText(QString::null);
        d->labelFileDate->setText(QString::null);
        d->labelFileSize->setText(QString::null);
        d->labelFileMime->setText(QString::null);
        d->labelImageDimensions->setText(QString::null);
        d->labelAlreadyDownloaded->setText(QString::null);
        setEnabled(false);
        return;
    }
    
    QString str;
    QDateTime date;

    setEnabled(true);

    d->navigateBar->setFileName(itemInfo->name);
    d->navigateBar->setButtonsState(itemType);

    d->labelNewFileName->setText(newFileName);
    d->labelFolder->setText(itemInfo->folder);
    
    if (itemInfo->readPermissions < 0)
        str = i18n("Unknown");
    else if (itemInfo->readPermissions == 0)
        str = i18n("No");
    else
        str = i18n("Yes");

    d->labelFileIsReadable->setText(str);
    
    if (itemInfo->writePermissions < 0)
        str = i18n("Unknown");
    else if (itemInfo->writePermissions == 0)
        str = i18n("No");
    else
        str = i18n("Yes");
    
    d->labelFileIsWritable->setText(str);
    
    date.setTime_t(itemInfo->mtime);
    d->labelFileDate->setText(KGlobal::locale()->formatDateTime(date, true, true));
    
    str = i18n("%1 (%2)").arg(KIO::convertSize(itemInfo->size))
                         .arg(KGlobal::locale()->formatNumber(itemInfo->size, 0));
    d->labelFileSize->setText(str);
    
    d->labelFileMime->setText(itemInfo->mime);

    QString mpixels;
    QSize dims(itemInfo->width, itemInfo->height);
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 1);
    str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)")
          .arg(dims.width()).arg(dims.height()).arg(mpixels);
    d->labelImageDimensions->setText(str);
    
    if (itemInfo->downloaded < 0)
        str = i18n("Unknown");
    else if (itemInfo->downloaded == 0)
        str = i18n("No");
    else
        str = i18n("Yes");
    
    d->labelAlreadyDownloaded->setText(str);
}
    
}  // NameSpace Digikam

#include "cameraitempropertiestab.moc"
