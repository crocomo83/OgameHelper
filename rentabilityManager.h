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
        float _score = -1.0f;
        float timeToCompleteDay;

        LevelUp(const QString& name, int level) : _name(name), _levelToUpdate(level) {}

        virtual void computeRenta(Ressources<float> tradeRate) = 0;
    };

    struct ProductionLevelUp : LevelUp
    {
        using LevelUp::LevelUp;

        Ressources<float> rentaPerDay;

        void computeRenta(Ressources<float> tradeRate)
        {
            _score = cost.getEquivalentDeut(tradeRate) / rentaPerDay.getEquivalentDeut(tradeRate) + timeToCompleteDay;
        }
    };

    struct SpeedLevelUp : LevelUp
    {
        using LevelUp::LevelUp;

        float _percentsGain;

        void computeRenta(Ressources<float> tradeRate)
        {
            _score = cost.getEquivalentDeut(tradeRate) / _percentsGain;
        }
    };

    enum class TypeFilter
    {
        BuildingFilter,
        ResearchFilter,
        LifeFormBuildingFilter,
        LifeFormResearchFilter,
        Count
    };
    inline static const QStringList typeFilterString = {"Building", "Research", "LifeFormBuilding", "LifeFormResearch"};

    enum class RentabilityType
    {
        Ressources,
        BuildingSpeed,
        ResearchSpeed,
        LifeFormResearchSpeed,
        Count
    };


public:
    static RentabilityManager& instance();

    int refresh();
    void refreshRessources();
    void refreshBuildingSpeed();
    void refreshResearchSpeed();
    void refreshLifeFormResearchSpeed();
    inline const RentabilityManager::LevelUp* getLevelUp(int index) const {return rentaLevelUp.at(index).get();}
    inline bool getFilterPlanet(int index) const {return _filterPlanet.at(index);}
    inline bool getFilterType(TypeFilter index) const {return _typeFilter.at(index);}

    Ressources<float> getGainMines(const Planet& planet) const;
    Ressources<float> getGainBuilding(const Planet *planet, const LifeFormBuilding* tech, int numberOfLevels = 1) const;
    Ressources<float> getGainResearch(const LifeFormTech* tech, int numberOfLevels = 1) const;
    float getGainTimePercent(const LifeFormBuilding* tech) const;
    float getGainTimePercent(const LifeFormTech* tech, BonusLifeForm bonusType) const;
    inline int getNbLevelUp() const {return rentaLevelUp.size();}
    inline RentabilityType getRentabilityType() const {return _type;}

    inline void setRentabilityType(RentabilityType type) {_type = type;}

    inline void setFilterPlanet(int index, bool state) {_filterPlanet[index] = state;}
    inline void setFilterType(TypeFilter index, bool state) {_typeFilter[index] = state;}

    inline void planetsAdded() {_filterPlanet.push_back(true);}

private:
    // Ressources gains
    void addNewLevelUp(std::unique_ptr<LevelUp> levelUp, std::optional<int> indexPlanet = std::nullopt);
    void addReasearchRentability();
    void addAstroRentability(PlayerManager::Planification *planif);
    void addPlanifChgtSpecies(PlayerManager::Planification *planif, int indexPlanet);
    void addMinesRentability(int indexPlanet);
    void addLifeFormBuilding(int indexPlanet);
    void addLifeFormResearch(int indexPlanet);
    void addLevelUpLifeForm(PlayerManager::Planification *planif, int indexPlanet);

    // Time gains
    void addSpeedBuildingForBuilding(int indexPlanet);
    void addSpeedBuildingForResearch(int indexPlanet);
    void addSpeedResearchForResearch();
    void addSpeedBuildingLifeFormForResearchLifeForm(int indexPlanet);
    void addSpeedResearchLifeFormForResearch(int indexPlanet, BonusLifeForm bonusType);

public slots:
    void onPlanetRemoved(int indexPlanet);

private:
    RentabilityManager();
    ~RentabilityManager() = default;

    RentabilityManager(const RentabilityManager&) = delete;
    RentabilityManager& operator=(const RentabilityManager&) = delete;

    std::vector<std::unique_ptr<LevelUp>> rentaLevelUp;
    std::vector<bool> _filterPlanet;
    std::unordered_map<TypeFilter, bool> _typeFilter;
    RentabilityType _type = RentabilityType::Ressources;
};
