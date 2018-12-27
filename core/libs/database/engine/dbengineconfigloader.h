/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database Engine element configuration loader
 *
 * Copyright (C) 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DB_ENGINE_CONFIG_LOADER_H
#define DIGIKAM_DB_ENGINE_CONFIG_LOADER_H

#include "dbengineconfigsettings.h"

// Qt includes

#include <QString>
#include <QMap>
#include <QDomElement>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DbEngineConfigSettingsLoader
{
public:

    explicit DbEngineConfigSettingsLoader(const QString& filepath, int xmlVersion);

    bool                   readConfig(const QString& filepath,
                                      int xmlVersion);
    DbEngineConfigSettings readDatabase(QDomElement& databaseElement);

    void                   readDBActions(QDomElement& sqlStatementElements,
                                         DbEngineConfigSettings& configElement);

public:

    bool                                  isValid;
    QString                               errorMessage;
    QMap<QString, DbEngineConfigSettings> databaseConfigs;
};

} // namespace Digikam

#endif // DIGIKAM_DB_ENGINE_CONFIG_LOADER_H
