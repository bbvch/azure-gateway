#include "azure_Client.h"

#include "iotsdk/logger.h"
#include "logger.h"

#include <azure_c_shared_utility/platform.h>

#include <QFile>
#include <QThread>
#include <QUuid>
#include <QCoreApplication>

namespace azure
{


Client::Client(const Connection::Parameter &parameter_, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol_, QObject *parent) :
    QObject(parent),
    parameter(parameter_),
    protocol(protocol_),
    worker(nullptr),
    cloudThread(nullptr)
{
    xlogging_set_log_function(azure::iotsdk::loghandler);
    platform_init();
}

Client::~Client()
{
    platform_deinit();
}

void Client::connect()
{
    try {
        worker = new Connection(parameter, protocol);

        cloudThread = new QThread;
        worker->moveToThread(cloudThread);

        QObject::connect(worker, &Connection::connectingError, cloudThread, &QThread::quit);
        QObject::connect(cloudThread, &QThread::finished, worker, &Connection::deleteLater);
        QObject::connect(worker, &Connection::destroyed, cloudThread, &QThread::deleteLater);
        QObject::connect(cloudThread, &QThread::destroyed, this, &Client::disconnected);

        QObject::connect(worker, &Connection::connected, this, &Client::connected);
        QObject::connect(worker, &Connection::connectingError, this, &Client::connectingError);
        QObject::connect(worker, &Connection::sent, this, &Client::sent);
        QObject::connect(worker, &Connection::sendError, this, &Client::sendError);
        QObject::connect(worker, &Connection::received, this, &Client::received);

        QObject::connect(worker, &Connection::destroyed, []{
            qCDebug(logger) << "cloud worker destroyed";
        });
        QObject::connect(cloudThread, &QThread::destroyed, []{
            qCDebug(logger) << "cloud thread destroyed";
        });
        QObject::connect(cloudThread, &QThread::finished, []{
            qCDebug(logger) << "cloud thread finished";
        });
        QObject::connect(cloudThread, &QThread::started, []{
            qCDebug(logger) << "cloud thread started";
        });

        cloudThread->start(QThread::LowPriority); // we set a low prio to reduce impact on the rest of the sw
    } catch (std::runtime_error e) {
        worker = nullptr;
        cloudThread = nullptr;
        qCCritical(logger) << "could not create connection:" << e.what();
        connectingError(Connection::ConnectionError::CanNotCreateConnection, 0);
    }
}

void Client::disconnect()
{
    if (cloudThread)
    {
        cloudThread->quit();

        cloudThread = nullptr;
        worker = nullptr;
    } else {
        disconnectingError();
    }
}

void Client::send(const QString &data, const QString &contentType, const QString &contentEncoding, const QStringMap &headers)
{
    if (!worker)
    {
        sendError(Connection::SendError::NoConnection, 0);
        return;
    }

    const bool ok = QMetaObject::invokeMethod(worker, "send", Qt::QueuedConnection, Q_ARG(QString, data), Q_ARG(QString, contentType), Q_ARG(QString, contentEncoding), Q_ARG(QStringMap, headers));
    Q_ASSERT(ok);
}


}
