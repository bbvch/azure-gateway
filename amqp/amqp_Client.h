/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "utility.h"
#include "QStringMap.h"

#include <QObject>
#include <amqp.h>
#include <memory>
#include <functional>

namespace amqp
{


class Client :
        public QObject
{
    Q_OBJECT

public:
    explicit Client(const QString &queuename, QObject *parent = nullptr);

signals:
    void received(const QString &data, const QString &contentType, const QString &contentEncoding, const QStringMap &headers);
    void acked();

public slots:
    void tick();
    void ackLastMessage();

private:
    const std::unique_ptr<amqp_connection_state_t_, decltype(&destroy_connection)> conn;
    const std::unique_ptr<void, std::function<void(void*)>> open_connection;
    const std::unique_ptr<const amqp_channel_t, std::function<void(const amqp_channel_t*)>> channel;

    QList<uint64_t> unansweredMessages{};

};


}
