#pragma once

#include "commons.h"
#include "playerManager.h"

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
        QString _name;
        int _levelToUpdate;
        Ressources<float> cost;
        Ressources<float> rentaPerDay;
        float timeToCompleteDay;
        float timeToRecover = -1.0f;

        LevelUp(const QString& name, int level) : _name(name), _levelToUpdate(level) {}

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

    Ressources<float> getGainMines(const Planet& planet) const;
    Ressources<float> getGainBuilding(const Planet *planet, const LifeFormBuilding* tech, int numberOfLevels = 1) const;
    Ressources<float> getGainResearch(const LifeFormTech* tech, int numberOfLevels = 1) const;

    inline void setFilterPlanet(int index, bool state) {_filterPlanet[index] = state;}
    inline void setFilterType(TypeFilter index, bool state) {_typeFilter[index] = state;}

    inline void planetsAdded() {_filterPlanet.push_back(true);}

private:
    void addNewLevelUp(LevelUp levelUp, std::optional<int> indexPlanet = std::nullopt);
    void addReasearchRentability();
    void addAstroRentability(PlayerManager::Planification *planif);
    void addPlanifChgtSpecies(PlayerManager::Planification *planif, int indexPlanet);
    void addMinesRentability(int indexPlanet);
    void addLifeFormBuilding(int indexPlanet);
    void addLifeFormResearch(int indexPlanet);
    void addLevelUpLifeForm(PlayerManager::Planification *planif, int indexPlanet);

public slots:
    void onPlanetRemoved(int indexPlanet);

private:
    RentabilityManager();
    ~RentabilityManager() = default;

    RentabilityManager(const RentabilityManager&) = delete;
    RentabilityManager& operator=(const RentabilityManager&) = delete;

    std::vector <LevelUp> rentaLevelUp;
    std::vector<bool> _filterPlanet;
    std::unordered_map<TypeFilter, bool> _typeFilter;
};
