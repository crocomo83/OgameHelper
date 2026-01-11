#include "planet.h"
#include "techManager.h"

Planet::Planet(QString name, std::array<int, 3> position, int temperature, Species species)
    : _name(name)
    , _position(position)
    , _temperature(temperature)
    , _species(species)
    , _bonusProdPositionCoeff({1.0f, 1.0f, 1.0f})
    , _crawlerNumber(0)
{
    // Buildings
    int numberTechs = TechManager::instance().getNumberTechs(TechType::CommonBuilding);
    _techs[TechType::CommonBuilding].assign(numberTechs, 0);

    computeBonusPos();
    computeProductionMine();
    computeBaseProduction();
}

void Planet::computeBonusPos()
{
    switch(_position[2])
    {
        case 1:
            _bonusProdPositionCoeff[1] = 1.4f;
           break;
        case 2:
            _bonusProdPositionCoeff[1] = 1.3f;
            break;
        case 3:
            _bonusProdPositionCoeff[1] = 1.2f;
            break;
        case 6:
        case 10:
            _bonusProdPositionCoeff[0] = 1.17f;
            break;
        case 7:
        case 9:
            _bonusProdPositionCoeff[0] = 1.23f;
            break;
        case 8:
            _bonusProdPositionCoeff[0] = 1.35f;
            break;
        default:
            break;
    }
}

void Planet::computeProductionMine()
{
    int levelMetal = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineMetal));
    int levelCristal = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineCristal));
    int levelDeut = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineDeut));

    _minesProduction[CommonBuildingType::MineMetal] = _bonusProdPositionCoeff[0] * 24 * 30 * levelMetal * std::pow(1.1, levelMetal);
    _minesProduction[CommonBuildingType::MineCristal] = _bonusProdPositionCoeff[1] * 24 * 20 * levelCristal * std::pow(1.1, levelCristal);
    _minesProduction[CommonBuildingType::MineDeut] = _bonusProdPositionCoeff[2] * 24 * 10 * levelDeut * std::pow(1.1, levelDeut) * (1.28 - 0.002 * _temperature);
}

void Planet::computeBaseProduction()
{
    _baseProd.metal = _bonusProdPositionCoeff[0] * 30;
    _baseProd.cristal = _bonusProdPositionCoeff[1] * 15;
    _baseProd.deut = 0;
}

void Planet::addTech(TechType type, int level)
{
    _techs[type].push_back(level);
}

const QString& Planet::getName() const
{
    return _name;
}

Species Planet::getSpecies() const
{
    return _species;
}

int Planet::getTemperatureMax() const
{
    return _temperature;
}

int Planet::getNumberTech(TechType type) const
{
    return _techs.at(type).size();
}

int Planet::getTechLevel(TechType type, int index) const
{
    return _techs.at(type).at(index);
}

void Planet::setName(QString name)
{
    _name = name;
}

void Planet::setSpecies(Species species)
{
    _species = species;
}

void Planet::setTechLevel(TechType type, int index, int level)
{
    _techs[type][index] = level;
    computeProductionMine();
}

Ressources Planet::getBaseProduction() const
{
    return _baseProd;
}

int Planet::getProductionMine(CommonBuildingType buildingType) const
{
    if (_minesProduction.contains(buildingType))
    {
        return _minesProduction.at(buildingType);
    }
    else
    {
        return 0;
    }
}

int Planet::getMaxActiveCrawler() const
{
    int levelMetal = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineMetal));
    int levelCristal = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineCristal));
    int levelDeut = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineDeut));

    return 8 * (levelMetal + levelCristal + levelDeut);
}

float Planet::getCrawlerBonus() const
{
    int maxCrawler = getMaxActiveCrawler();
    int activeCrawlers = std::min(maxCrawler, _crawlerNumber);
    return std::min(0.5f, 0.02f / 100.0f * activeCrawlers);
}

Ressources Planet::getCrawlerProduction() const
{
    float bonus = getCrawlerBonus();
    float metalProd = _minesProduction.at(CommonBuildingType::MineMetal);
    float cristalProd = _minesProduction.at(CommonBuildingType::MineCristal);
    float deutProd = _minesProduction.at(CommonBuildingType::MineDeut);
    return Ressources(bonus * metalProd, bonus * cristalProd, bonus * deutProd);
}

void Planet::setCrawlerNumber(int crawlers)
{
    _crawlerNumber = crawlers;
}
