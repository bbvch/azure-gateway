/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "QStringMap.h"
#include "MessageAdapter.h"
#include "ClientAdapterHandler.h"

#include <azureiot/iothub_client_ll.h>

#include <QObject>
#include <memory>


namespace azure::iotsdk
{


class ClientAdapter
{
public:
    ClientAdapter(const std::string &connectionString, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol, ClientAdapterHandler &handler);
    virtual ~ClientAdapter() = default;

    bool setMessageTimeout(const std::chrono::milliseconds &);
    bool setRetryPolicy(IOTHUB_CLIENT_RETRY_POLICY, std::size_t retryTimeoutLimitInSeconds);
    bool setX509Certificate(const std::string &);
    bool setX509PrivateKey(const std::string &);

    bool sendMessage(const MessageAdapter &);
    bool sendDeviceTwin(const std::string &);

    void doWork();

private:
    std::unique_ptr<IOTHUB_CLIENT_LL_HANDLE_DATA_TAG, std::function<void(IOTHUB_CLIENT_LL_HANDLE_DATA_TAG*)>> handle;
    ClientAdapterHandler &handler;

};


}
