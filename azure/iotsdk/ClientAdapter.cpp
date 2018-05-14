/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ClientAdapter.h"

#include "logger.h"

#include <azureiot/iothub_client_options.h>
#include <azureiot/iothubtransporthttp.h>
#include <azureiot/iothub_client.h>

#include <memory>


namespace azure::iotsdk
{
namespace
{

static IOTHUBMESSAGE_DISPOSITION_RESULT receiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    ClientAdapterHandler *handler = static_cast<ClientAdapterHandler*>(userContextCallback);
    return handler->messageReceived(message);
}

static void connectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback)
{
    ClientAdapterHandler *handler = static_cast<ClientAdapterHandler*>(userContextCallback);
    handler->connectionStatusChanged(result, reason);
}

void sendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    ClientAdapterHandler *handler = static_cast<ClientAdapterHandler*>(userContextCallback);
    handler->messageSendResult(result);
}

static void sendReportedStateCallback(int status_code, void* userContextCallback)
{
    ClientAdapterHandler *handler = static_cast<ClientAdapterHandler*>(userContextCallback);
    handler->deviceTwinSendResult(status_code);
}

}



ClientAdapter::ClientAdapter(const std::string &connectionString, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol, ClientAdapterHandler &handler_) :
    handle{IoTHubClient_LL_CreateFromConnectionString(connectionString.c_str(), protocol), IoTHubClient_LL_Destroy},
    handler{handler_}
{
    if (!handle)
    {
        throw std::runtime_error("can not create connection handle");
    }

    IOTHUB_CLIENT_RESULT result = IoTHubClient_LL_SetMessageCallback(handle.get(), receiveMessageCallback, &handler);
    if (result != IOTHUB_CLIENT_OK)
    {
        throw std::runtime_error("can not register receive callback");
    }

    result = IoTHubClient_LL_SetConnectionStatusCallback(handle.get(), connectionStatusCallback, &handler);
    if (result != IOTHUB_CLIENT_OK)
    {
        throw std::runtime_error("can not register connection status callback");
    }
}

bool ClientAdapter::setMessageTimeout(const std::chrono::milliseconds &value)
{
    const unsigned long long IOTHUB_LL_TIMEOUT = value.count(); //TODO test
    const auto result = IoTHubClient_LL_SetOption(handle.get(), OPTION_MESSAGE_TIMEOUT, &IOTHUB_LL_TIMEOUT);
    return result == IOTHUB_CLIENT_OK;
}

bool ClientAdapter::setRetryPolicy(IOTHUB_CLIENT_RETRY_POLICY policy, std::size_t retryTimeoutLimitInSeconds)
{
    const auto result = IoTHubClient_LL_SetRetryPolicy(handle.get(), policy, retryTimeoutLimitInSeconds);
    return result == IOTHUB_CLIENT_OK;
}

bool ClientAdapter::setX509Certificate(const std::string &value)
{
    const auto result = IoTHubClient_LL_SetOption(handle.get(), OPTION_X509_CERT, value.c_str());
    return result == IOTHUB_CLIENT_OK;
}

bool ClientAdapter::setX509PrivateKey(const std::string &value)
{
    const auto result = IoTHubClient_LL_SetOption(handle.get(), OPTION_X509_PRIVATE_KEY, value.c_str());
    return result == IOTHUB_CLIENT_OK;
}

bool ClientAdapter::sendMessage(const MessageAdapter &value)
{
    const auto result = IoTHubClient_LL_SendEventAsync(handle.get(), value.getHandle(), sendConfirmationCallback, &handler);
    return result == IOTHUB_CLIENT_OK;
}

bool ClientAdapter::sendDeviceTwin(const std::string &value)
{
    const auto result = IoTHubClient_LL_SendReportedState(handle.get(), reinterpret_cast<const unsigned char*>(value.c_str()), value.size(), sendReportedStateCallback, &handler);
    return result == IOTHUB_CLIENT_OK;
}

void ClientAdapter::doWork()
{
    IoTHubClient_LL_DoWork(handle.get());
}


}
