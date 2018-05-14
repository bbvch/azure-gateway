/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "MessageAdapter.h"


namespace azure::iotsdk
{


MessageAdapter::MessageAdapter(const std::string &content) :
    handle{IoTHubMessage_CreateFromByteArray(reinterpret_cast<const unsigned char*>(content.c_str()), content.size()), IoTHubMessage_Destroy}
{
}

bool MessageAdapter::isValid() const
{
    return handle != nullptr;
}

bool MessageAdapter::setId(const std::string &value)
{
    const auto result = IoTHubMessage_SetMessageId(handle.get(), value.c_str());
    return result == IOTHUB_MESSAGE_OK;
}

bool MessageAdapter::setContentType(const std::string &value)
{
    const auto result = IoTHubMessage_SetContentTypeSystemProperty(handle.get(), value.c_str());
    return result == IOTHUB_MESSAGE_OK;
}

bool MessageAdapter::setContentEncoding(const std::string &value)
{
    const auto result = IoTHubMessage_SetContentEncodingSystemProperty(handle.get(), value.c_str());
    return result == IOTHUB_MESSAGE_OK;
}

bool MessageAdapter::addHeader(const std::map<std::string, std::string> &value)
{
    const auto properties = IoTHubMessage_Properties(handle.get());

    for (const auto &item : value) {
            const auto result = Map_Add(properties, item.first.c_str(), item.second.c_str());
            if (result != MAP_OK)
                {
                    return false;
                }
        }

    return true;
}

IOTHUB_MESSAGE_HANDLE MessageAdapter::getHandle() const
{
    return handle.get();
}


}
