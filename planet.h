#pragma once

#include "commons.h"

#include <QString>

class Planet
{
public:
    Planet(QString name, std::array<int, 3> position, int temperature, Species species = Species::None);

    void computeBonusPos();

    void addTech(TechType type, int level = 0);

    const QString& getName() const;
    Species getSpecies() const;
    int getTemperatureMax() const;
    int getNumberTech(TechType type) const;
    int getTechLevel(TechType type, int index) const;
    Ressources getBaseProduction() const;
    int getProductionMine(CommonBuildingType buildingType) const;

    int getMaxActiveCrawler() const;
    float getCrawlerBonus() const;
    Ressources getCrawlerProduction() const;

    void computeBaseProduction();
    void computeProductionMine();

    void setName(QString name);
    void setSpecies(Species species);
    void setTechLevel(TechType type, int index, int level);
    void setCrawlerNumber(int crawlers);

private:
    QString                                 _name;
    std::array<int, 3>                      _position;
    int                                     _temperature;
    Species                                 _species;
    std::map<TechType, std::vector<int>>    _techs;
    std::array<float, 3>                    _bonusProdPositionCoeff;
    int                                     _crawlerNumber;
    Ressources                              _baseProd;
    std::map<CommonBuildingType, int>       _minesProduction;
};
