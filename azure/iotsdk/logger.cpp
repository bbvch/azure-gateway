/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "logger.h"

#include <stdarg.h>


namespace azure::iotsdk
{

Q_LOGGING_CATEGORY(logger, "azure.iotsdk")


void loghandler(LOG_CATEGORY log_category, const char *, const char *, const int, unsigned int, const char *format, ...)
{
    va_list args;
    char buf[2048];

    va_start(args, format);
    vsnprintf(buf, 2048, format, args);
    va_end(args);

    if (log_category == AZ_LOG_ERROR)
    {
        qCWarning(logger) << buf;
    }
    else
    {
        qCDebug(logger) << buf;
    }
}


}
