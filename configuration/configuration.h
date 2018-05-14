/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "azure/azure_Client.h"

#include <QString>

namespace configuration
{


struct Options
{
    QString queue;
    QString hostname;
    QString deviceId;
    QString keyName;
};

Options parseArguments(const QStringList &);
void printInfo(const Options &);
azure::Connection::Parameter loadParameter(const Options &);


}
