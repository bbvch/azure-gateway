/*
 * (C) Copyright 2018
 * Urs Fässler, bbv Software Services, http://bbv.ch
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "azure_c_shared_utility/xlogging.h"

#include <stdarg.h>
#include <QLoggingCategory>

namespace azure::iotsdk
{

Q_DECLARE_LOGGING_CATEGORY(logger)


void loghandler(LOG_CATEGORY log_category, const char* file, const char* func, const int line, unsigned int options, const char* format, ...);


}
