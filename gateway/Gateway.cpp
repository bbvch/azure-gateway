/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Gateway.h"

#include "logger.h"

namespace gateway
{


Gateway::Gateway(QObject *parent) :
    QObject{parent}
{
}

QStringMap removeAllWithPrefix(const QString &prefix, const QStringMap &value)
{
    QStringMap result{};

    for (const auto &key : value.keys())
    {
        if (!key.startsWith(prefix))
        {
            result[key] = value[key];
        }
    }

    return result;
}

void Gateway::fromQueue(const QString &data, const QString &contentType, const QString &contentEncoding, const QStringMap &headers)
{
    const auto type = headers["gateway:type"];
    const auto id = headers["gateway:message-id"];

    const QStringMap cloudHeader = removeAllWithPrefix("gateway:", headers);

    if (type == "properties")
    {
        sendCloudDeviceTwin(data);
    }
    else
    {
        if (type != "message")
        {
            qCWarning(logger) << "send undefined or unknown message type as message" << type;
        }
        const Message message{cloudHeader, id, contentEncoding, contentType, data};
        sendCloudMessage(message);
    }
}


}
