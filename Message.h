/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
