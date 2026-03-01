#pragma once

#include "commons.h"

#include <map>
#include <QString>

class Planet;
class RentabilityManager
{
public:
    struct LevelUp
    {
        int indexPlanet = -1;
        QString name;
        int levelToUpdate;
        Ressources cost;
        Ressources rentaPerDay;
        float timeToRecover = -1.0f;
        QString timeToRecoverStr = "";

        void computeRenta(Ressources tradeRate)
            {timeToRecover = cost.getEquivalentDeut(tradeRate) / rentaPerDay.getEquivalentDeut(tradeRate);}
    };

public:
    static RentabilityManager& instance();

    int refresh();
    const RentabilityManager::LevelUp& getLevelUp(int index) const;
    QString rentaToString(float timeToRecover) const;

private:
    void addMinesRentability(const Planet *planet, int indexPlanet);
    void addLifeFormBuilding(const Planet *planet, int indexPlanet);
    void addLifeFormResearch(const Planet *planet, int indexPlanet);

private:
    RentabilityManager();
    ~RentabilityManager() = default;

    RentabilityManager(const RentabilityManager&) = delete;
    RentabilityManager& operator=(const RentabilityManager&) = delete;

    std::vector <LevelUp> rentaLevelUp;
};
