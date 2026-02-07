#pragma once

#include "commons.h"

#include <map>
#include <QString>

class Planet;
class RentabilityManager
{
public:
    enum class LevelUpType
    {
        MetalMine,
        CristalMine,
        DeutMine,
        Crawlers,
        Plasma
    };

    struct LevelUp
    {
        int indexPlanet = -1;
        LevelUpType type;
        int levelToUpdate;
        Ressources cost;
        Ressources rentaPerHour;
        float timeToRecover = -1.0f;
        QString timeToRecoverStr = "";

        void computeRenta(Ressources tradeRate)
            {timeToRecover = cost.getEquivalentDeut(tradeRate) / rentaPerHour.getEquivalentDeut(tradeRate);}
    };

public:
    static RentabilityManager& instance();

    int refresh();
    const RentabilityManager::LevelUp& getLevelUp(int index) const;
    QString levelUpToString(LevelUpType levelUp);
    QString rentaToString(float timeToRecover) const;

private:
    void addMinesRentability(Planet *planet, int indexPlanet);

private:
    RentabilityManager();
    ~RentabilityManager() = default;

    RentabilityManager(const RentabilityManager&) = delete;
    RentabilityManager& operator=(const RentabilityManager&) = delete;

    std::vector <LevelUp> rentaLevelUp;
};
