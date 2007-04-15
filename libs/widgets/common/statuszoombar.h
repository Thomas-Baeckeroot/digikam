/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-04-15
 * Description : a zoom bar used in status bar.
 *
 * Copyright 2007 by Gilles Caulier
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

#ifndef STATUSSTATUSBAR_H
#define STATUSSTATUSBAR_H

// KDE includes.

#include <qhbox.h>
#include <qstring.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class StatusZoomBarPriv;

class DIGIKAM_EXPORT StatusZoomBar : public QHBox
{

Q_OBJECT

public:

    StatusZoomBar( QWidget *parent=0 );
    ~StatusZoomBar();

    void setEnableZoomPlus(bool e);
    void setEnableZoomMinus(bool e);

    void setZoomSliderValue(int v);
    void setZoomTrackerText(const QString& text);

signals:

    void signalZoomPlusClicked();
    void signalZoomMinusClicked();
    void signalZoomSliderChanged(int);

private:

    StatusZoomBarPriv* d;
};

}  // namespace Digikam

#endif /* STATUSSTATUSBAR_H */
