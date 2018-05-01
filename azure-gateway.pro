QT += core
QT -= gui

CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

CONFIG += warn_on

OBJECTS_DIR = ./$$TARGET-obj/  # workaround for: https://bugreports.qt.io/browse/QTBUG-50803
CONFIG += object_parallel_to_source

QMAKE_CXXFLAGS += -Wall -Wextra -pedantic
QMAKE_CXXFLAGS += -std=c++17

SOURCES += \
    main.cpp \
    azure/Connection.cpp \
    amqp/utility.cpp \
    amqp/amqp_Client.cpp \
    amqp/logger.cpp \
    azure/azure_Client.cpp \
    configuration/configuration.cpp \
    configuration/logger.cpp \
    azure/iotsdk/logger.cpp \
    azure/logger.cpp \
    gateway/Gateway.cpp \
    gateway/logger.cpp \

HEADERS += \
    azure/Connection.h \
    amqp/utility.h \
    QStringMap.h \
    amqp/amqp_Client.h \
    amqp/logger.h \
    azure/azure_Client.h \
    configuration/configuration.h \
    configuration/logger.h \
    azure/iotsdk/logger.h \
    azure/logger.h \
    gateway/Gateway.h \
    gateway/logger.h \

target.path = /usr/bin/
INSTALLS += target


LIBS += \
        -liothub_client \
        -liothub_client_amqp_ws_transport \
        -liothub_client_mqtt_ws_transport \
        -liothub_client_http_transport \
        -liothub_client_mqtt_transport \
        -luamqp \
        -lumqtt \
        -laziotsharedutil \
        -lcurl \
        -luuid \
        -lparson \

LIBS += -lssl -lcrypto

LIBS += -lcute-adapter-production -lcute-adapter

LIBS += -lrabbitmq
