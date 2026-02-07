#include "planet.h"
#include "techManager.h"

#include <QDebug>

Planet::Planet(const QString& name, const PlanetPosition& position, int temperature, Species species)
    : _name(name)
    , _position(position)
    , _temperature(temperature)
    , _species(species)
    , _bonusProdPositionCoeff(1.0f, 1.0f, 1.0f)
    , _crawlerNumber(0)
{
    for (int i = 1; i < static_cast<int>(TechType::Count); ++i)
    {
        TechType current = static_cast<TechType>(i);
        if (current != TechType::CommonResearch)
        {
            int numberTechs = TechManager::instance().getNumberTechs(current);
            _techs[current].assign(numberTechs, 0);
        }
    }

    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    _choicesLifeFormResearch.assign(numberLifeFormReseach, Species::None);

    computeBonusPos();
    computeProduction();
}

void Planet::computeBonusPos()
{
    switch(_position.position)
    {
        case 1:
            _bonusProdPositionCoeff.cristal = 1.4f;
           break;
        case 2:
            _bonusProdPositionCoeff.cristal = 1.3f;
            break;
        case 3:
            _bonusProdPositionCoeff.cristal = 1.2f;
            break;
        case 6:
        case 10:
            _bonusProdPositionCoeff.metal = 1.17f;
            break;
        case 7:
        case 9:
            _bonusProdPositionCoeff.metal = 1.23f;
            break;
        case 8:
            _bonusProdPositionCoeff.metal = 1.35f;
            break;
        default:
            break;
    }
}

void Planet::computeProduction()
{
    _productionStats.clear();

    // Base
    Ressources baseProd, minesProduction;
    baseProd.metal = _bonusProdPositionCoeff.metal * 24 * 30;
    baseProd.cristal = _bonusProdPositionCoeff.cristal * 24 * 15;
    baseProd.deut = 0;
    _productionStats[ProductionStatPlanet::Base] = baseProd;

    // Mines
    int levelMetal = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineMetal));
    int levelCristal = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineCristal));
    int levelDeut = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineDeut));
    minesProduction.metal   = _bonusProdPositionCoeff.metal * 24 * 30 * levelMetal * std::pow(1.1, levelMetal);
    minesProduction.cristal = _bonusProdPositionCoeff.cristal * 24 * 20 * levelCristal * std::pow(1.1, levelCristal);
    minesProduction.deut    = _bonusProdPositionCoeff.deut * 24 * 10 * levelDeut * std::pow(1.1, levelDeut) * (1.28 - 0.002 * _temperature);
    _productionStats[ProductionStatPlanet::Mines] = minesProduction;

    // Crawlers
    float crawlerBonus = getCrawlerBonus();
    _productionStats[ProductionStatPlanet::CrawlersPercent] = Ressources(crawlerBonus, crawlerBonus, crawlerBonus);

    // Buildings life form
    _productionStats[ProductionStatPlanet::BuildingLifeFormPercent] = getLifeFormBuildingBonusPercent();
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

Ressources Planet::getBonusMine() const
{
    return _bonusProdPositionCoeff;
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
    computeProduction();
}

PlanetPosition Planet::getPosition() const
{
    return _position;
}

int Planet::getCrawlerNumber() const
{
    return _crawlerNumber;
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
    return std::min(50.0f, 0.02f * activeCrawlers);
}

Ressources Planet::getLifeFormBuildingBonusPercent() const
{
    TechType lifeFormBuilding = getLifeFormBuilding();
    switch(lifeFormBuilding)
    {
        case TechType::HumanBuilding:
        {
            int levelBuildingMetal          = _techs.at(lifeFormBuilding).at(5);
            int levelBuildingCristalAndDeut = _techs.at(lifeFormBuilding).at(7);
            return Ressources(1.5f * levelBuildingMetal, 1.5f * levelBuildingCristalAndDeut, 1.0f * levelBuildingCristalAndDeut);
        }
        case TechType::RoctasBuilding:
        {
            int levelBuildingMetal      = _techs.at(lifeFormBuilding).at(5);
            int levelBuildingCristal    = _techs.at(lifeFormBuilding).at(8);
            int levelBuildingDeut       = _techs.at(lifeFormBuilding).at(9);
            return Ressources(2 * levelBuildingMetal, 2 * levelBuildingCristal, 2 * levelBuildingDeut);
        }
        default:
            return Ressources();
    }
}

Ressources Planet::getCrawlerProduction() const
{
    float bonus = getCrawlerBonus() / 100.0f;
    Ressources mineProduction = _productionStats.at(ProductionStatPlanet::Mines);
    return bonus * mineProduction;
}

Ressources Planet::getLifeFormBuildingProduction() const
{
    Ressources bonus = 0.01f * _productionStats.at(ProductionStatPlanet::BuildingLifeFormPercent);
    Ressources prodMines = _productionStats.at(ProductionStatPlanet::Mines);
    return Ressources(bonus.metal * prodMines.metal, bonus.cristal * prodMines.cristal, bonus.deut * prodMines.deut);
}

TechType Planet::getLifeFormBuilding() const
{
    switch (_species)
    {
    case Species::Humans:
        return TechType::HumanBuilding;
    case Species::Mechs:
        return TechType::MechBuilding;
    case Species::Kaeleshs:
        return TechType::KaeleshBuilding;
    case Species::Roctas:
        return TechType::RoctasBuilding;
    default:
        return TechType::None;
    }
}

std::vector<TechType> Planet::getAvailableBuildings() const
{
    std::vector<TechType> techs;
    techs.push_back(TechType::CommonBuilding);

    TechType lifeFormBuilding = getLifeFormBuilding();
    if (lifeFormBuilding != TechType::None)
    {
        techs.push_back(lifeFormBuilding);
    }
    return techs;
}

Species Planet::getChoiceLifeFormResearch(int index) const
{
    if (index >= 0 && index < _choicesLifeFormResearch.size())
    {
        return _choicesLifeFormResearch.at(index);
    }
    else
    {
        qWarning() << "try to access invalid index : " << index << " in getChoiceLifeFormResearch";
        return Species::None;
    }
}

int Planet::getLevelLifeFormResearch(Species species, int index) const
{
    if (species == Species::None) {return 0;}

    TechType lifeFormResearch = speciesToTechLifeForm.at(species);
    return _techs.at(lifeFormResearch).at(index);
}

const Ressources& Planet::getProductionStat(Planet::ProductionStatPlanet stat) const
{
    return _productionStats.at(stat);
}

void Planet::setCrawlerNumber(int crawlers)
{
    _crawlerNumber = crawlers;
}

void Planet::setPosition(const PlanetPosition& planetPosition)
{
    _position = planetPosition;
}

void Planet::setChoiceLifeFormResearch(int index, Species species)
{
    _choicesLifeFormResearch[index] = species;
}

void Planet::setLevelLifeFormResearch(Species species, int index, int level)
{
    if (species == Species::None) {return;}

    TechType lifeFormResearch = speciesToTechLifeForm.at(species);
    _techs[lifeFormResearch][index] = level;
}
