#pragma once

#include "azure_c_shared_utility/xlogging.h"

#include <stdarg.h>

namespace azure
{


void azureiotsdk_logger(LOG_CATEGORY log_category, const char* file, const char* func, const int line, unsigned int options, const char* format, ...);


}
