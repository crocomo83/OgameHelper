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
}
