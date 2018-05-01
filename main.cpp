#include "azure/azure_Client.h"
#include "amqp/amqp_Client.h"
#include "configuration/configuration.h"

#include <cute-adapter-production/linux/SignalHandler.h>

#include <azureiot/iothubtransportamqp_websockets.h>

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>


int main(int argc, char *argv[])
{
    qRegisterMetaType<azure::Connection::ConnectionError>();
    qRegisterMetaType<azure::Connection::SendError>();

    QCoreApplication app(argc, argv);

    const auto options = configuration::parseArguments(app.arguments());
    printInfo(options);

    azure::Client azureClient{loadParameter(options), AMQP_Protocol_over_WebSocketsTls};
    amqp::Client queue{options.queue};
    cute_adapter_production::linux::SignalHandler signalHandler{};

    QTimer receivedTick{};
    receivedTick.setInterval(10);
    receivedTick.setSingleShot(false);

    QTimer::singleShot(0, &azureClient, &azure::Client::connect);
    QObject::connect(&azureClient, SIGNAL(connected()), &receivedTick, SLOT(start()));
    QObject::connect(&receivedTick, SIGNAL(timeout()), &queue, SLOT(tick()));

    QObject::connect(&signalHandler, &cute_adapter::linux::SignalHandler::received, &azureClient, &azure::Client::disconnect);
    QObject::connect(&azureClient, &azure::Client::connectingError, &app, &QCoreApplication::quit);
    QObject::connect(&azureClient, &azure::Client::disconnected, &app, &QCoreApplication::quit);

    QObject::connect(&queue, SIGNAL(received(QString, QString, QString, QStringMap)), &azureClient, SLOT(send(QString, QString, QString, QStringMap)));
    QObject::connect(&azureClient, &azure::Client::sent, &queue, &amqp::Client::ackLastMessage);

    return app.exec();
}
