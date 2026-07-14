#pragma once

#include "commons.h"

#include <map>
#include <unordered_map>
#include <QString>
#include <chrono>
#include <optional>

class Planet;
class RentabilityManager
{
public:
    struct LevelUp
    {
        std::vector<int> indexPlanets;
        QString name;
        int levelToUpdate;
        Ressources<float> cost;
        Ressources<float> rentaPerDay;
        float timeToCompleteDay;
        float timeToRecover = -1.0f;
        QString timeToRecoverStr = "";

        void computeRenta(Ressources<float> tradeRate)
        {
            timeToRecover = cost.getEquivalentDeut(tradeRate) / rentaPerDay.getEquivalentDeut(tradeRate) + timeToCompleteDay;
        }
    };

    enum TypeFilter
    {
        BuildingFilter,
        ResearchFilter,
        LifeFormBuildingFilter,
        LifeFormResearchFilter,
        Count
    };
    inline static const QStringList typeFilterString = {"Building", "Research", "LifeFormBuilding", "LifeFormResearch"};


public:
    static RentabilityManager& instance();

    int refresh();
    inline const RentabilityManager::LevelUp& getLevelUp(int index) const {return rentaLevelUp.at(index);}
    inline bool getFilterPlanet(int index) const {return _filterPlanet.at(index);}
    inline bool getFilterType(TypeFilter index) const {return _typeFilter.at(index);}

    Ressources<float> getGain(const LifeFormTech* tech) const;

    inline void setFilterPlanet(int index, bool state) {_filterPlanet[index] = state;}
    inline void setFilterType(TypeFilter index, bool state) {_typeFilter[index] = state;}
    QString rentaToString(float timeToRecover) const;

private:
    void addNewLevelUp(LevelUp levelUp, std::optional<int> indexPlanet = std::nullopt);
    void addReasearchRentability();
    void addMinesRentability(const Planet& planet, int indexPlanet);
    void addLifeFormBuilding(const Planet& planet, int indexPlanet);
    void addLifeFormResearch(const Planet& planet, int indexPlanet);
    void addLevelUpLifeForm(const Planet& planet, int indexPlanet);

private:
    RentabilityManager();
    ~RentabilityManager() = default;

    RentabilityManager(const RentabilityManager&) = delete;
    RentabilityManager& operator=(const RentabilityManager&) = delete;

    std::vector <LevelUp> rentaLevelUp;
    std::vector<bool> _filterPlanet;
    std::unordered_map<TypeFilter, bool> _typeFilter;
};
