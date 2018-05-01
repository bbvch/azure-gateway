QT += core
QT -= gui

CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

CONFIG += warn_on

QMAKE_CXXFLAGS += -Wall -Wextra -pedantic
QMAKE_CXXFLAGS += -std=c++17

SOURCES += \
    main.cpp \
    azure/Logger.cpp \
    azure/Connection.cpp \
    amqp/utility.cpp \
    amqp/amqp_Client.cpp \
    azure/azure_Client.cpp \
    configuration.cpp \

HEADERS += \
    azure/Logger.h \
    azure/Connection.h \
    amqp/utility.h \
    QStringMap.h \
    amqp/amqp_Client.h \
    azure/azure_Client.h \
    configuration.h \

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
