/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "QStringMap.h"
#include "Message.h"
#include "iotsdk/ClientAdapter.h"

#include <azureiot/iothub_client_ll.h>

#include <QObject>
#include <memory>


namespace azure
{


class Connection :
        public QObject,
        private iotsdk::ClientAdapterHandler
{
    Q_OBJECT

public:
    struct Parameter
    {
        QString connectionString;
        QString X509Certificate;
        QString X509privateKey;
    };

    enum class ConnectionError {
        CanNotCreateConnection,
        CanNotConnect,
    };
    Q_ENUM(ConnectionError)

    enum class SendError {
        NoConnection,
        MessageCreationError,
        RequestFailed,
        ConfirmationFailed,
    };
    Q_ENUM(SendError)


    explicit Connection(const Parameter &parameter, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol, QObject *parent = nullptr);

signals:
    void connected();
    void connectingError(azure::Connection::ConnectionError, int);

    void sent();
    void sendError(azure::Connection::SendError, int);

    void received(const QString &data);

public slots:
    void sendMessage(const Message &);
    void sendDeviceTwin(const QString& data);

private slots:
    void tick();

private:
    iotsdk::ClientAdapter client;

    void connectionStatusChanged(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason) override;
    IOTHUBMESSAGE_DISPOSITION_RESULT messageReceived(IOTHUB_MESSAGE_HANDLE message) override;
    void messageSendResult(IOTHUB_CLIENT_CONFIRMATION_RESULT result) override;
    void deviceTwinSendResult(int statusCode) override;

};


}
