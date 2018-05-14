/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Connection.h"

#include "logger.h"

#include <azureiot/iothub_client_options.h>
#include <azureiot/iothubtransporthttp.h>
#include <azureiot/iothub_client.h>

#include <QThread>
#include <QTimer>

#include <memory>


namespace azure
{


Connection::Connection(const Parameter &parameter, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol, QObject *parent) :
    QObject(parent),
    client{parameter.connectionString.toUtf8().toStdString(), protocol, *this}
{
    const size_t retryTimeoutLimitInSeconds = 0; // no retry limit
    if (!client.setRetryPolicy(IOTHUB_CLIENT_RETRY_LINEAR_BACKOFF, retryTimeoutLimitInSeconds))
    {
        throw std::runtime_error("can not set retry policy");
    }

    if (!client.setMessageTimeout(std::chrono::minutes(3)))
    {
        throw std::runtime_error("can not set message timeout");
    }

    if (!client.setX509Certificate(parameter.X509Certificate.toUtf8().toStdString()))
    {
        throw std::runtime_error("can not set X509 certificate");
    }

    if (!client.setX509PrivateKey(parameter.X509privateKey.toUtf8().toStdString()))
    {
        throw std::runtime_error("can not set X509 private key");
    }

    tick();
}

void Connection::tick()
{
    client.doWork();
    QTimer::singleShot(10, this, SLOT(tick()));
}



std::map<std::string, std::string> toStd(const QStringMap &value) //TODO move into QStringMap
{
    std::map<std::string, std::string> result{};

    for (const auto &itr : value.toStdMap())
    {
        result[itr.first.toStdString()] = itr.second.toStdString();
    }

    return result;
}

void Connection::sendMessage(const Message &raw)
{
    qCDebug(logger) << "send message with header" << raw.header;

    iotsdk::MessageAdapter message{raw.content.toUtf8().toStdString()};


    if (!message.isValid())
    {
        sendError(SendError::MessageCreationError, 0);
        return;
    }

    if (raw.id != "")
    {
        if (!message.setId(raw.id.toStdString()))
        {
            sendError(SendError::MessageCreationError, 0);
            return;
        }
    }

    if (!message.setContentType(raw.contentType.toStdString()))
    {
        sendError(SendError::MessageCreationError, 0);
        return;
    }

    if (!message.setContentEncoding(raw.contentEncoding.toStdString()))
    {
        sendError(SendError::MessageCreationError, 0);
        return;
    }

    if (!message.addHeader(toStd(raw.header)))
    {
        sendError(SendError::MessageCreationError, 0);
        return;
    }

    if (!client.sendMessage(message))
    {
        sendError(SendError::RequestFailed, 0);
        return;
    }
}

void Connection::sendDeviceTwin(const QString &data)
{
    qCDebug(logger) << "send device twin";

    if (!client.sendDeviceTwin(data.toUtf8().toStdString()))
    {
        sendError(SendError::RequestFailed, 0); //TODO use different error
        return;
    }
}

void Connection::connectionStatusChanged(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason)
{
    qCDebug(logger) << "Connection Status:"
             << ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS, result)
             << ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS_REASON, reason);

    if ((result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED) && (reason == IOTHUB_CLIENT_CONNECTION_OK))
    {
        qCInfo(logger) << "connected";
        connected();
    }
    else
    {
        qCWarning(logger) << "connection error:" << ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS_REASON, reason);
        connectingError(ConnectionError::CanNotConnect, reason);
    }
}

IOTHUBMESSAGE_DISPOSITION_RESULT Connection::messageReceived(IOTHUB_MESSAGE_HANDLE message)
{
    const char* buffer;
    size_t messageSize;
    IOTHUB_MESSAGE_RESULT result = IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &messageSize);

    if (result == IOTHUB_MESSAGE_OK)
    {
        QString incomingMessage(QByteArray(buffer, messageSize));
        qCDebug(logger) << "Incoming message" << incomingMessage;
        received(incomingMessage);
        return IOTHUBMESSAGE_ACCEPTED;
    }
    else
    {
        qCWarning(logger) << "Failure while unpacking incoming message" << ENUM_TO_STRING(IOTHUB_MESSAGE_RESULT, result);
        return IOTHUBMESSAGE_REJECTED;
    }
}

void Connection::messageSendResult(IOTHUB_CLIENT_CONFIRMATION_RESULT result)
{
    qCDebug(logger) << "send confirmation received:" << ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result);

    if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
    {
        sent();
    }
    else
    {
        sendError(SendError::ConfirmationFailed, result);
    }
}

void Connection::deviceTwinSendResult(int statusCode)
{
    qCDebug(logger) << "send reported state received:" << statusCode;

    if (statusCode == 200) //NOTE 200 seems to be the success code
    {
        sent();
    }
    else
    {
        sendError(SendError::ConfirmationFailed, statusCode);
    }
}


}
