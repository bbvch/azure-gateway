#pragma once

#include "QStringMap.h"
#include <QString>
#include <QMetaType>


struct Message
{
    QStringMap header;
    QString id;
    QString contentEncoding;
    QString contentType;
    QString content;
};

Q_DECLARE_METATYPE(Message)
