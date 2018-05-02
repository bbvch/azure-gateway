#pragma once

#include <azureiot/iothub_client_ll.h>


namespace azure::iotsdk
{


class ClientAdapterHandler
{
public:
    virtual ~ClientAdapterHandler() = default;

    virtual void connectionStatusChanged(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason) = 0;
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT messageReceived(IOTHUB_MESSAGE_HANDLE message) = 0;
    virtual void messageSendResult(IOTHUB_CLIENT_CONFIRMATION_RESULT result) = 0;
    virtual void deviceTwinSendResult(int statusCode) = 0;
};


}
