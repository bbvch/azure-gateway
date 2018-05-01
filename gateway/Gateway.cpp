#include "Gateway.h"

namespace gateway
{


Gateway::Gateway(QObject *parent) :
    QObject{parent}
{
}

void Gateway::fromQueue(const QString &data, const QString &contentType, const QString &contentEncoding, const QStringMap &headers)
{
    sendCloudMessage(data, contentType, contentEncoding, headers);
}


}
