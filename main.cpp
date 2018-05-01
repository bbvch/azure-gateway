#include "azure/azure_Client.h"
#include "amqp/amqp_Client.h"
#include "configuration.h"

#include <cute-adapter-production/linux/SignalHandler.h>

#include <azureiot/iothubtransportamqp_websockets.h>

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

namespace
{


void connectInfo(azure::Client &client)
{
    QObject::connect(&client, &azure::Client::connected, [](){
        qInfo() << "connected to cloud";
    });
    QObject::connect(&client, &azure::Client::disconnected, [](){
        qInfo() << "disconnected from cloud";
    });
    QObject::connect(&client, &azure::Client::connectingError, [](azure::Connection::ConnectionError error, int code){
        qCritical() << error << "code" << code;
    });
    QObject::connect(&client, &azure::Client::sent, []{
        qDebug() << "message sent";
    });

    QObject::connect(&client, &azure::Client::received, [](const QString &){
        qWarning() << "received message, did not expect one";
    });
}


}


int main(int argc, char *argv[])
{
    qRegisterMetaType<azure::Connection::ConnectionError>();
    qRegisterMetaType<azure::Connection::SendError>();

    QCoreApplication app(argc, argv);

    const Options options = parseArguments(app.arguments());
    printInfo(options);

    azure::Client azureClient{loadParameter(options), AMQP_Protocol_over_WebSocketsTls};
    amqp::Client queue{options.queue};
    cute_adapter_production::linux::SignalHandler signalHandler{};

    QTimer receivedTick{};
    receivedTick.setInterval(10);
    receivedTick.setSingleShot(false);

    connectInfo(azureClient);

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
