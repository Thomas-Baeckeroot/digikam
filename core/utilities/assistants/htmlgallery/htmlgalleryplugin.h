/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to generate HTML image galleries.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_HTML_GALLERY_PLUGIN_H
#define DIGIKAM_HTML_GALLERY_PLUGIN_H

// Local includes

#include "dplugin.h"

namespace Digikam
{

class HtmlGalleryPlugin : public DPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.digikam.HtmlGallery")
    Q_INTERFACES(Digikam::DPlugin)

public:

    explicit HtmlGalleryPlugin(QObject* const parent = 0);

    QString name()                 const override;
    QString id()                   const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;

    void setup(QObject* const);

private Q_SLOTS:

    void slotHtmlGallery();
};

} // namespace Digikam

#endif // DIGIKAM_HTML_GALLERY_PLUGIN_H
