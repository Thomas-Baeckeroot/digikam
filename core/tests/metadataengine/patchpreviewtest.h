/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-10-25
 * Description : Extract preview and patch with DMetadata.
 *               This stage is used by Export tools.
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_PATCH_PREVIEW_TEST_H
#define DIGIKAM_PATCH_PREVIEW_TEST_H

// Qt includes

#include <QObject>
#include <QString>

class PatchPreviewTest : public QObject
{
    Q_OBJECT

private:

    void patchPreview(const QString& file, bool rescale, int maxDim, int imageQuality);

private Q_SLOTS:

    void initTestCase();
    void testExtractPreviewAndFixMetadata();
    void cleanupTestCase();
};

#endif // DIGIKAM_PATCH_PREVIEW_TEST_H
