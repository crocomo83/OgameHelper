#pragma once

#include <QString>
#include <QLocale>

namespace utils
{
    inline QString formatRessource(float value)
    {
        return QLocale().toString((int)value);
    }

    inline QString formatRessource(int value)
    {
        return QLocale().toString(value);
    }

    inline QString timeToString(float timeDays)
    {
        if (timeDays < 1.0f / 24.0f)
        {
            return QString::number(timeDays * 24.0f * 60.0f, 'f', 2) + " min";
        }
        else if (timeDays < 1.0f)
        {
            return QString::number(timeDays * 24.0f, 'f', 2) + " hours";
        }
        else if (timeDays < 7.0f)
        {
            return QString::number(timeDays, 'f', 2) + " days";
        }
        else if (timeDays < 30.0f)
        {
            return QString::number(timeDays / 7.0f, 'f', 2) + " weeks";
        }
        else if (timeDays < 365.0f)
        {
            return QString::number(timeDays / 30.0f, 'f', 2) + " months";
        }
        else
        {
            return QString::number(timeDays / 365.0f, 'f', 2) + " years";
        }
    }
}
