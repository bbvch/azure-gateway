#pragma once

#include "azure/azure_Client.h"

#include <QString>


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
