#include "Connection.h"

#include "logger.h"

#include <azureiot/iothub_client_options.h>
#include <azureiot/iothubtransporthttp.h>
#include <azureiot/iothub_client.h>

#include <QThread>
#include <QUuid>
#include <QTimer>

#include <memory>


namespace azure
{

namespace
{

static IOTHUBMESSAGE_DISPOSITION_RESULT receiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    Connection *client = static_cast<Connection*>(userContextCallback);
    return client->onReceive(message);
}

static void connectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback)
{
    Connection *client = static_cast<Connection*>(userContextCallback);
    client->onConnectStatusChanged(result, reason);
}

void sendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    Connection *client = static_cast<Connection*>(userContextCallback);
    client->onSendConfirmed(result);
}

}


Connection::Connection(const Parameter &parameter, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol, QObject *parent) :
    QObject(parent),
    iotHubClientHandle(IoTHubClient_LL_CreateFromConnectionString(parameter.connectionString.toUtf8().data(), protocol))
{
    if (!iotHubClientHandle)
    {
        throw std::runtime_error("can not create connection handle");
    }

    IOTHUB_CLIENT_RESULT result = IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, this);
    if (result != IOTHUB_CLIENT_OK)
    {
        throw std::runtime_error("can not register receive callback");
    }

    result = IoTHubClient_LL_SetConnectionStatusCallback(iotHubClientHandle, connectionStatusCallback, this);
    if (result != IOTHUB_CLIENT_OK)
    {
        throw std::runtime_error("can not register connection status callback");
    }

    const size_t retryTimeoutLimitInSeconds = 0; // no retry limit
    result = IoTHubClient_LL_SetRetryPolicy(iotHubClientHandle, IOTHUB_CLIENT_RETRY_LINEAR_BACKOFF, retryTimeoutLimitInSeconds);
    if (result != IOTHUB_CLIENT_OK)
    {
        throw std::runtime_error("can not set retry policy");
    }

    const unsigned long long IOTHUB_LL_TIMEOUT = 3 * 60 * 1000;
    result = IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_MESSAGE_TIMEOUT, &IOTHUB_LL_TIMEOUT);
    if (result != IOTHUB_CLIENT_OK)
    {
        throw std::runtime_error("can not set message timeout");
    }

    result = IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_X509_CERT, parameter.X509Certificate.toUtf8().data());
    if (result != IOTHUB_CLIENT_OK)
    {
        throw std::runtime_error("can not set X509 certificate");
    }

    result = IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_X509_PRIVATE_KEY, parameter.X509privateKey.toUtf8().data());
    if (result != IOTHUB_CLIENT_OK)
    {
        throw std::runtime_error("can not set X509 private key");
    }

    tick();
}

Connection::~Connection()
{
    IoTHubClient_LL_Destroy(iotHubClientHandle);
}

void Connection::tick()
{
    IoTHubClient_LL_DoWork(iotHubClientHandle);
    QTimer::singleShot(10, this, SLOT(tick()));
}

void Connection::send(const QString &data, const QString &contentType, const QString &contentEncoding, const QStringMap &headers)
{
    qCDebug(logger) << "send message with header" << headers;

    std::unique_ptr<IOTHUB_MESSAGE_HANDLE_DATA_TAG, std::function<void(IOTHUB_MESSAGE_HANDLE_DATA_TAG*)>>
    messageHandle(
        IoTHubMessage_CreateFromByteArray(reinterpret_cast<const unsigned char*>(data.toUtf8().data()), data.length()),
        IoTHubMessage_Destroy
    );


    if (!messageHandle)
    {
        sendError(SendError::MessageCreationError, 0);
        return;
    }

    IOTHUB_MESSAGE_RESULT msgResult = IoTHubMessage_SetMessageId(messageHandle.get(), QUuid::createUuid().toString().toStdString().c_str());
    if (msgResult != IOTHUB_MESSAGE_OK)
    {
        sendError(SendError::MessageCreationError, msgResult);
        return;
    }

    msgResult = IoTHubMessage_SetContentTypeSystemProperty(messageHandle.get(), contentType.toStdString().c_str());
    if (msgResult != IOTHUB_MESSAGE_OK)
    {
        sendError(SendError::MessageCreationError, msgResult);
        return;
    }

    msgResult = IoTHubMessage_SetContentEncodingSystemProperty(messageHandle.get(), contentEncoding.toStdString().c_str());
    if (msgResult != IOTHUB_MESSAGE_OK)
    {
        sendError(SendError::MessageCreationError, msgResult);
        return;
    }

    {
        const auto properties = IoTHubMessage_Properties(messageHandle.get());

        for (const auto &key : headers.keys()) {
            const auto &value = headers[key];
            const auto mapResult = Map_Add(properties, key.toStdString().c_str(), value.toStdString().c_str());
            if (mapResult != MAP_OK)
            {
                sendError(SendError::MessageCreationError, mapResult); //TODO use different error
                return;
            }
        }
    }

    IOTHUB_CLIENT_RESULT sendResult = IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle.get(), sendConfirmationCallback, this);
    if (sendResult != 0)
    {
        sendError(SendError::RequestFailed, sendResult);
        return;
    }
}

void Connection::onConnectStatusChanged(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason)
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

IOTHUBMESSAGE_DISPOSITION_RESULT Connection::onReceive(IOTHUB_MESSAGE_HANDLE message)
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

void Connection::onSendConfirmed(IOTHUB_CLIENT_CONFIRMATION_RESULT result)
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


}
