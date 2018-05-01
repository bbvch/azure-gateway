#include "configuration.h"

#include "logger.h"

#include <QFile>
#include <QStandardPaths>
#include <QCommandLineParser>

namespace configuration
{


QString readFile(const QString &filename)
{
    QFile file{filename};
    if (!file.open(QFile::ReadOnly)) {
        qCCritical(logger) << "Could not read file" << filename;
        return {};
    }
    QTextStream stream{&file};
    return stream.readAll();
}

QString locate(const QString &filename)
{
    const QString path = QStandardPaths::locate(QStandardPaths::AppConfigLocation, filename);

    if (path.isEmpty()) {
        qCCritical(logger) << "configuration file" << filename << "not found in" << QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    }

    return path;
}

azure::Connection::Parameter loadParameter(const Options &options)
{
    const QString connectionString = "HostName=" + options.hostname + ";DeviceId=" + options.deviceId + ";x509=true";

    const QString certificateFile = locate(options.keyName + ".certificate");
    const QString privatekeyFile = locate(options.keyName + ".privatekey");

    if (certificateFile.isEmpty() || privatekeyFile.isEmpty()) {
        qFatal("certificate or private key not found");
    }

    qCInfo(logger) << "using certification file" << certificateFile;
    qCInfo(logger) << "using private key file" << privatekeyFile;

    const QString certificate = readFile(certificateFile);
    const QString privatekey = readFile(privatekeyFile);

    if (certificate.isEmpty() || privatekey.isEmpty()) {
        qFatal("error while loading certificate or private key");
    }

    const azure::Connection::Parameter parameter
    {
        connectionString,
        certificate,
        privatekey,
    };

    return parameter;
}

QString readDeviceIdFromFile()
{
    const QString path = locate("device-id");
    if (path.isEmpty()) {
        qFatal("device-id file not found");
    }
    const QString deviceIdRaw = readFile(path);
    const QString deviceId = deviceIdRaw.trimmed();
    return deviceId;
}

Options parseArguments(const QStringList &arguments)
{
    QCommandLineParser parser{};
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption keyname{"key-name", "name of the key to use", "key name", "default"};
    parser.addOption(keyname);

    const QCommandLineOption deviceId{"device-id", "id of the device", "device id"};
    parser.addOption(deviceId);

    parser.addPositionalArgument("queue", "amqp queue to read messages from");
    parser.addPositionalArgument("host", "name of the host to connect to");

    parser.process(arguments);

    const auto positional = parser.positionalArguments();

    if (positional.size() != 2) {
        parser.showHelp(1);
    }

    const QString deviceIdValue = parser.isSet(deviceId) ? parser.value(deviceId) : readDeviceIdFromFile();

    const Options options
    {
        positional[0],
        positional[1],
        deviceIdValue,
        parser.value(keyname),
    };

    return options;
}

void printInfo(const Options &value)
{
    qCInfo(logger) << "connecting to host" << value.hostname;
    qCInfo(logger) << "using device id" << value.deviceId;
    qCInfo(logger) << "read messages from amqp queue" << value.queue;
}


}
