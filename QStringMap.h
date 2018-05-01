#pragma once

#include <QMap>
#include <QString>
#include <QMetaType>


typedef QMap<QString, QString> QStringMap;

Q_DECLARE_METATYPE(QStringMap)
