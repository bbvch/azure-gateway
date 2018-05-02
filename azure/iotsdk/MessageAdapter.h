#pragma once

#include <azureiot/iothub_message.h>

#include <string>
#include <map>
#include <memory>
#include <functional>


namespace azure::iotsdk
{


class MessageAdapter
{
public:
    MessageAdapter(const std::string &content);

    bool isValid() const;

    bool setId(const std::string &value);
    bool setContentType(const std::string &value);
    bool setContentEncoding(const std::string &value);
    bool addHeader(const std::map<std::string, std::string> &value);

    IOTHUB_MESSAGE_HANDLE getHandle() const;

private:
    std::unique_ptr<IOTHUB_MESSAGE_HANDLE_DATA_TAG, std::function<void(IOTHUB_MESSAGE_HANDLE_DATA_TAG*)>> handle;
};


}
