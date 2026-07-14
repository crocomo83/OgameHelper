#pragma once

#include "commons.h"

#include <QString>
#include <map>

class Planet
{
public:
    enum class ProductionStat{
        Base,
        Mines,
        Crawlers,
        BuildingLifeForm,
        Count
    };

    enum class ProductionStatPercent{
        CrawlersPercent,
        BuildingLifeFormPercent,
        Count
    };

    Planet();
    Planet(const QString& name, const PlanetPosition& position, int temperature, Species species = Species::None);
    Planet(const Planet* planet);

    void                    initEmptyPlanet();

    inline const QString&   getName() const {return _name;}
    inline Species          getSpecies() const {return _species;}
    inline int              getTemperatureMax() const {return _temperature;}
    inline int              getNumberTech(TechType type) const {return _techs.at(type).size();}
    inline int              getTechLevel(TechType type, int index) const {return _techs.at(type).at(index);}
    inline Ressources<float> getBonusMine() const {return _bonusProdPositionCoeff;}
    inline PlanetPosition   getPosition() const {return _position;}
    inline int              getCrawlerNumber() const {return _crawlerNumber;}
    inline const Ressources<int>& getProductionStat(ProductionStat stat) const {return _productionStats.at(stat);}
    inline const Ressources<float>& getProductionStatPercent(ProductionStatPercent stat) const {return _productionStatsPercent.at(stat);}
    inline float            getLifeFormBuildingBonus(BonusLifeFormBuilding bonus) const {return _lifeFormBuildingBonuses.at(bonus);}
    int                     getMaxActiveCrawler() const;
    float                   getCrawlerBonus() const;
    Ressources<float>       getLifeFormBuildingProduction() const;
    std::vector<TechType>   getAvailableBuildings() const;
    TechType                getLifeFormBuilding() const;
    Species                 getChoiceLifeFormResearch(int index) const;
    int                     getLevelLifeFormResearch(Species species, int index) const;
    Ressources<float>       getLifeFormProdBonus() const;
    Ressources<float>       getCost(TechType techType, int indexTech, int level) const;
    float                   getTime(TechType techType, int indexTech, int level) const; //time in days

    void                    refresh();
    void                    computeBonusPos();
    void                    computeLifeFormBuildingBonus();
    void                    computeProduction();

    inline void             setName(QString name) {_name = name;}
    inline void             setTemperatureMax(int tempMax) {_temperature = tempMax;}
    inline void             setSpecies(Species species) {_species = species;}
    inline void             setCrawlerNumber(int crawlers) {_crawlerNumber = crawlers;}
    inline void             setPosition(const PlanetPosition& planetPosition) {_position = planetPosition;}
    inline void             setChoiceLifeFormResearch(int index, Species species) {_choicesLifeFormResearch[index] = species;}
    void                    setTechLevel(TechType type, int index, int level);
    void                    setLevelLifeFormResearch(Species species, int index, int level);

private:
    QString                                     _name;
    PlanetPosition                              _position;
    int                                         _temperature {0};
    Species                                     _species {Species::None};
    std::map<TechType, std::vector<int>>        _techs;
    std::vector<Species>                        _choicesLifeFormResearch;
    Ressources<float>                           _bonusProdPositionCoeff;
    int                                         _crawlerNumber {0};

    std::map<ProductionStat, Ressources<int>> _productionStats;
    std::map<ProductionStatPercent, Ressources<float>> _productionStatsPercent;
    std::map<BonusLifeFormBuilding, float>      _lifeFormBuildingBonuses;
};
