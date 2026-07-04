#pragma once

#include "commons.h"

#include <map>
#include <QString>
#include <chrono>
#include <optional>

class Planet;
class RentabilityManager
{
public:
    struct LevelUp
    {
        std::optional<int> indexPlanet;
        int numberInstance = 1;
        QString name;
        int levelToUpdate;
        Ressources cost;
        Ressources rentaPerDay;
        float timeToCompleteDay;
        float timeToRecover = -1.0f;
        QString timeToRecoverStr = "";

        void computeRenta(Ressources tradeRate)
            {timeToRecover = cost.getEquivalentDeut(tradeRate) / rentaPerDay.getEquivalentDeut(tradeRate) + timeToCompleteDay;}
    };

public:
    static RentabilityManager& instance();

    int refresh();
    const RentabilityManager::LevelUp& getLevelUp(int index) const;
    QString rentaToString(float timeToRecover) const;

private:
    void addNewLevelUp(LevelUp levelUp);
    void addReasearchRentability();
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
