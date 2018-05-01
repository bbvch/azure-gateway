#pragma once

#include <QStringMap.h>
#include <QObject>

namespace gateway
{


class Gateway :
        public QObject
{
    Q_OBJECT
public:
    explicit Gateway(QObject *parent = nullptr);

public slots:
    void fromQueue(const QString &data, const QString &contentType, const QString &contentEncoding, const QStringMap &headers);

signals:
    void sendCloudMessage(const QString &data, const QString &contentType, const QString &contentEncoding, const QStringMap &headers);

};


}
