#include "Gateway.h"

#include "logger.h"

namespace gateway
{


Gateway::Gateway(QObject *parent) :
    QObject{parent}
{
}

QStringMap removeAllWithPrefix(const QString &prefix, const QStringMap &value)
{
    QStringMap result{};

    for (const auto &key : value.keys())
    {
        if (!key.startsWith(prefix))
        {
            result[key] = value[key];
        }
    }

    return result;
}

void Gateway::fromQueue(const QString &data, const QString &contentType, const QString &contentEncoding, const QStringMap &headers)
{
    const auto type = headers["gateway:type"];

    const QStringMap cloudHeader = removeAllWithPrefix("gateway:", headers);

    if (type == "properties")
    {
        sendCloudDeviceTwin(data);
    }
    else
    {
        if (type != "message")
        {
            qCWarning(logger) << "send undefined or unknown message type as message" << type;
        }
        const Message message{cloudHeader, contentEncoding, contentType, data};
        sendCloudMessage(message);
    }
}


}
