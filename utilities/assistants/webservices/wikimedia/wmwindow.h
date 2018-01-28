/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-11
 * Description : a tool to export images to WikiMedia web service
 *
 * Copyright (C) 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Parthasarathy Gopavarapu <gparthasarathy93 at gmail dot com>
 * Copyright (C) 2013      by Peter Potrowl <peter dot potrowl at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef WM_WINDOW_H
#define WM_WINDOW_H

// Local includes

#include "tooldialog.h"
#include "dinfointerface.h"

class QCloseEvent;

class KJob;

namespace Digikam
{

class WMWindow : public ToolDialog
{
    Q_OBJECT

public:

    explicit WMWindow(DInfoInterface* const iface, QWidget* const parent);
    ~WMWindow();

    void reactivate();
    bool prepareImageForUpload(const QString& imgPath);

private Q_SLOTS:

    void slotFinished();
    void slotProgressCanceled();
    void slotStartTransfer();
    void slotChangeUserClicked();
    void slotDoLogin(const QString& login, const QString& pass, const QString& wikiName, const QUrl& wikiUrl);
    void slotEndUpload();
    int  slotLoginHandle(KJob* loginJob);

private:

    bool eventFilter(QObject* obj, QEvent* event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;
    void readSettings();
    void saveSettings();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // WM_WINDOW_H