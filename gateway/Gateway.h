/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <Message.h>
#include <QStringMap.h>
#include <QObject>

namespace gateway
{


class Gateway :
        public QObject
{
    Q_OBJECT
public:
    explicit Gateway(QObject *parent = nullptr);

public slots:
    void fromQueue(const QString &data, const QString &contentType, const QString &contentEncoding, const QStringMap &headers);

signals:
    void sendCloudMessage(const Message &);
    void sendCloudDeviceTwin(const QString &data);

};


}
