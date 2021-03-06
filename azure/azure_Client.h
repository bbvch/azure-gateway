/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "Connection.h"
#include "QStringMap.h"

#include <azureiot/iothub_client_ll.h>
#include <azureiot/iothub_client.h>
#include <azureiot/iothub_message.h>

#include <QObject>


namespace azure
{


class Client :
        public QObject
{
    Q_OBJECT

public:
    explicit Client(const Connection::Parameter &parameter, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol, QObject *parent = nullptr);
    ~Client();

public slots:
    void connect();
    void disconnect();
    void sendMessage(const Message &);
    void sendDeviceTwin(const QString& data);

signals:
    void connected();
    void connectingError(azure::Connection::ConnectionError, int);

    void disconnected();
    void disconnectingError();

    void sent();
    void sendError(azure::Connection::SendError, int);

    void received(const QString &data);

private:
    const Connection::Parameter parameter;    //TODO do not keep in memory since we have the private key here
    const IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;

    Connection* worker;
    QThread* cloudThread;
};


}
