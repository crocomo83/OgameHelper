#pragma once

#include "commons.h"

#include <QString>
#include <map>

class Planet
{
public:
    enum class ProductionStatPlanet{
        Base,
        Mines,
        CrawlersPercent,
        BuildingLifeFormPercent,
        Count
    };

    Planet(const QString& name, const PlanetPosition& position, int temperature, Species species = Species::None);
    Planet(const Planet* planet);

    void computeBonusPos();

    const QString&          getName() const;
    Species                 getSpecies() const;
    int                     getTemperatureMax() const;
    int                     getNumberTech(TechType type) const;
    int                     getTechLevel(TechType type, int index) const;
    Ressources              getBonusMine() const;
    PlanetPosition          getPosition() const;
    int                     getCrawlerNumber() const;
    int                     getMaxActiveCrawler() const;
    float                   getCrawlerBonus() const;
    Ressources              getCrawlerProduction() const;
    Ressources              getLifeFormBuildingProduction() const;
    std::vector<TechType>   getAvailableBuildings() const;
    TechType                getLifeFormBuilding() const;
    Species                 getChoiceLifeFormResearch(int index) const;
    int                     getLevelLifeFormResearch(Species species, int index) const;
    const Ressources&       getProductionStat(ProductionStatPlanet stat) const;
    float                   getLifeFormBuildingBonus(BonusLifeFormBuilding bonus) const;
    Ressources              getLifeFormProdBonus() const;
    Ressources              getCost(TechType techType, int indexTech, int level) const;

    void                    computeLifeFormBuildingBonus();
    void                    computeProduction();

    void                    setName(QString name);
    void                    setSpecies(Species species);
    void                    setTechLevel(TechType type, int index, int level);
    void                    setCrawlerNumber(int crawlers);
    void                    setPosition(const PlanetPosition& planetPosition);
    void                    setChoiceLifeFormResearch(int index, Species species);
    void                    setLevelLifeFormResearch(Species species, int index, int level);

private:
    QString                                     _name;
    PlanetPosition                              _position;
    int                                         _temperature;
    Species                                     _species;
    std::map<TechType, std::vector<int>>        _techs;
    std::vector<Species>                        _choicesLifeFormResearch;
    Ressources                                  _bonusProdPositionCoeff;
    int                                         _crawlerNumber;

    std::map<ProductionStatPlanet, Ressources>  _productionStats;
    std::map<BonusLifeFormBuilding, float>      _lifeFormBuildingBonuses;
};
