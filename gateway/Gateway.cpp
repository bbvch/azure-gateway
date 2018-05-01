#include "Gateway.h"

#include "logger.h"

namespace gateway
{


Gateway::Gateway(QObject *parent) :
    QObject{parent}
{
}

void Gateway::fromQueue(const QString &data, const QString &contentType, const QString &contentEncoding, const QStringMap &headers)
{
    const auto type = headers["gateway:type"];

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
        sendCloudMessage(data, contentType, contentEncoding, headers);
    }
}


}
