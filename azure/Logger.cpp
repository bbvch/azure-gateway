#include "Logger.h"

#include <QFileInfo>
#include <QDebug>
#include <stdarg.h>


namespace azure
{


void azureiotsdk_logger(LOG_CATEGORY log_category, const char *file, const char *func, const int line, unsigned int options, const char *format, ...)
{
    Q_UNUSED(options);

    va_list args;
    char buf[2048];

    va_start(args, format);
    vsnprintf(buf, 2048, format, args);
    va_end(args);

    QString info = QString("%1:%2,%3():").arg(QFileInfo(file).fileName()).arg(line).arg(func);
    if (log_category == AZ_LOG_ERROR)
    {
        qWarning() << info.toLatin1().data() << buf;
    }
    else
    {
        qDebug() << info.toLatin1().data() << buf;
    }
}


}
