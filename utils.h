#pragma once

#include <QString>
#include <QLocale>
#include <cmath>

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

    inline QString ressourceToString(float value)
    {
        if (value >= 1000000000)
        {
            return QString::number(value / 1000000000.f, 'f', 2) + " Md";
        }
        else if (value >= 1000000)
        {
            return QString::number(value / 1000000.f, 'f', 2) + " M";
        }
        else
        {
            return QString::number(value);
        }
    }

    inline double cumulativeCost(double cste, double increaseFactor, int n)
    {
        if (n <= 0)
            return 0.0;

        if (increaseFactor == 1.0)
            return cste * n;

        double r = increaseFactor;

        return cste * (std::pow(r, n) - 1.0) / (r - 1.0);
    }

    inline double cumulativeWeightedCost(double cste, double increaseFactor, int n)
    {
        if (n <= 0)
            return 0.0;

        if (increaseFactor == 1.0)
            return cste * n * (n + 1) / 2.0;

        double r = increaseFactor;
        double rn = std::pow(r, n);

        return cste * (1.0 - (n + 1) * rn + n * rn * r) / std::pow(1.0 - r, 2);
    }
}
