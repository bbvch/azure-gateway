#pragma once

#include "QStringMap.h"

#include <azureiot/iothub_client_ll.h>

#include <QObject>


namespace azure
{


class Connection :
        public QObject
{
    Q_OBJECT

public:
    struct Parameter
    {
        QString connectionString;
        QString X509Certificate;
        QString X509privateKey;
    };

    enum class ConnectionError {
        CanNotCreateConnection,
        CanNotConnect,
    };
    Q_ENUM(ConnectionError)

    enum class SendError {
        NoConnection,
        MessageCreationError,
        RequestFailed,
        ConfirmationFailed,
    };
    Q_ENUM(SendError)


    explicit Connection(const Parameter &parameter, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol, QObject *parent = nullptr);
    ~Connection();

    void onConnectStatusChanged(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason);
    IOTHUBMESSAGE_DISPOSITION_RESULT onReceive(IOTHUB_MESSAGE_HANDLE message);
    void onSendConfirmed(IOTHUB_CLIENT_CONFIRMATION_RESULT result);
    void onSendReportedState(int status_code);

signals:
    void connected();
    void connectingError(azure::Connection::ConnectionError, int);

    void sent();
    void sendError(azure::Connection::SendError, int);

    void received(const QString &data);

public slots:
    void sendMessage(const QString &data, const QString &contentType, const QString &contentEncoding, const QStringMap &headers);
    void sendDeviceTwin(const QString& data);

private slots:
    void tick();

private:
    const IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

};


}
