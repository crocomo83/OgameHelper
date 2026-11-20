#include "planet.h"
#include "techManager.h"
#include "generalManager.h"
#include "playermanager.h"

#include <QDebug>

Planet::Planet()
    : _name("Empty")
    , _position(PlanetPosition())
{
    initEmptyPlanet();
}

Planet::Planet(const QString& name)
    : _name(name)
    , _position(PlanetPosition())
{
    initEmptyPlanet();
}

Planet::Planet(const QString& name, const PlanetPosition& position, int temperature, Species species)
    : _name(name)
    , _position(position)
    , _temperatureMax(temperature)
    , _species(species)
{
    initEmptyPlanet();
}

Planet::Planet(const Planet* planet)
    : _name(planet->getName())
    , _position(planet->getPosition())
    , _temperatureMax(planet->getTemperatureMax())
    , _species(planet->getSpecies())
{
    for (int i = 1; i < static_cast<int>(TechType::Count); ++i)
    {
        TechType current = static_cast<TechType>(i);
        if (current != TechType::CommonResearch)
        {
            int numberTechs = TechManager::instance().getNumberTechs(current);
            for (int i = 0; i < numberTechs; ++i)
            {
                Building building;
                building._level = planet->getTechLevel(current, i);
                _techs[current].push_back(std::move(building));
            }
        }
    }

    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberLifeFormReseach; i++)
    {
        _choicesLifeFormResearch.push_back(planet->getChoiceLifeFormResearch(i));
    }
}

void Planet::initEmptyPlanet()
{
    for (int i = 1; i < static_cast<int>(TechType::Count); ++i)
    {
        TechType current = static_cast<TechType>(i);
        if (current != TechType::CommonResearch)
        {
            int numberTechs = TechManager::instance().getNumberTechs(current);
            _techs[current].assign(numberTechs, Building());
        }
    }
    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    _choicesLifeFormResearch.assign(numberLifeFormReseach, Species::None);
}

void Planet::computeBonusPos()
{
    _bonusProdPositionCoeff = Ressources(1.0f, 1.0f, 1.0f);
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

void Planet::refresh(const Ressources<float> &lifeFormBonuses)
{
    computeBonusPos();
    computeLifeFormBonus();
    computeBaseProduction();
    computeLifeFormBuildingProduction();
    computeProduction(lifeFormBonuses);
    computePoints();
}

void Planet::computeLifeFormResearch()
{
    _lifeFormBonuses.clear();
    int lifeFormBuildingNumber = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int j = 0; j < lifeFormBuildingNumber; j++)
    {
        Species choice = getChoiceLifeFormResearch(j);
        if (choice != Species::None)
        {
            TechType lifeFormReasearch = speciesToTechLifeForm.at(choice);
            int level = getTechLevel(lifeFormReasearch, j);

            const LifeFormTech* lifeFormTech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(lifeFormReasearch, j));
            for (auto itr = lifeFormTech->bonuses.begin(); itr != lifeFormTech->bonuses.end(); ++itr)
            {
                int levelSpecies = GeneralManager::instance().getLevelSpecies(lifeFormTech->species);
                float bonusLevelSpecies = 1.0f + (float)levelSpecies * 0.001f;
                float bonus = level * itr->second * bonusLevelSpecies;
                _lifeFormBonuses[itr->first] += bonus;
            }
        }
    }
}

void Planet::computeLifeFormBonus()
{
    _lifeFormBuildingBonuses.clear();
    for (int i = 0; i < static_cast<int>(BonusLifeFormBuilding::Count); i++)
    {
        _lifeFormBuildingBonuses[static_cast<BonusLifeFormBuilding>(i)];
    }

    TechType typeBuilding = speciesToBuildingLifeForm.at(_species);
    int lifeFormBuildingNumber = typeBuilding == TechType::None ? 0 : TechManager::instance().getNumberTechs(typeBuilding);

    for (int j = 0; j < lifeFormBuildingNumber; j++)
    {
        int level = getTechLevel(typeBuilding, j);

        const LifeFormBuilding* lifeFormTech = dynamic_cast<const LifeFormBuilding*>(TechManager::instance().getTech(typeBuilding, j));
        for (auto itr = lifeFormTech->bonuses.begin(); itr != lifeFormTech->bonuses.end(); ++itr)
        {
            _lifeFormBuildingBonuses[itr->first] += level * itr->second;
        }
    }
}

void Planet::computeBaseProduction()
{
    _productionStats.clear();
    _rawRefProduction.clear();
    _globalConsumption = 0;

    // Base
    Ressources<int> baseProd;
    baseProd.metal = (int)(_bonusProdPositionCoeff.metal * 30.0f) * 24;
    baseProd.cristal = (int)(_bonusProdPositionCoeff.cristal * 15.0f) * 24;
    baseProd.deut = 0;
    _productionStats[ProductionStat::Base] = baseProd;

    // Common buildings
    int numberTech = getNumberTech(TechType::CommonBuilding);
    for (int i = 0; i < numberTech; ++i)
    {
        Building& building = _techs.at(TechType::CommonBuilding).at(i);

        CommonBuildingType buildingType = static_cast<CommonBuildingType>(i);

        const Ressources<int>& production = getProduction(buildingType, building._level);
        const Ressources<int>& consumption = getConsumption(buildingType, building._level);
        building._production = production - consumption;
        _productionStats[ProductionStat::Mines] += building._production;
        _rawRefProduction += production;
        _globalConsumption += consumption.energy;
    }

    _productionStats[ProductionStat::Mines].energy += getSatelliteProduction();
    _rawRefProduction.energy += getSatelliteProduction();
}

void Planet::computeLifeFormBuildingProduction()
{
    Ressources<float> refProduction(_rawRefProduction);

    TechType typeBuilding = speciesToBuildingLifeForm.at(_species);
    int lifeFormBuildingNumber = typeBuilding == TechType::None ? 0 : TechManager::instance().getNumberTechs(typeBuilding);

    _productionStats[ProductionStat::BuildingLifeForm] = 0;
    for (int j = 0; j < lifeFormBuildingNumber; j++)
    {
        int level = getTechLevel(typeBuilding, j);

        const LifeFormBuilding* lifeFormTech = dynamic_cast<const LifeFormBuilding*>(TechManager::instance().getTech(typeBuilding, j));
        Ressources<float> factorMineBonus;
        for (auto itr = lifeFormTech->bonuses.begin(); itr != lifeFormTech->bonuses.end(); ++itr)
        {
            switch (itr->first)
            {
            case BonusLifeFormBuilding::Metal:
                factorMineBonus.metal += level * itr->second / 100.0f;
                break;
            case BonusLifeFormBuilding::Cristal:
                factorMineBonus.cristal += level * itr->second / 100.0f;
                break;
            case BonusLifeFormBuilding::Deut:
                factorMineBonus.deut += level * itr->second / 100.0f;
                break;
            case BonusLifeFormBuilding::Energy:
                factorMineBonus.energy += level * itr->second / 100.0f;
                break;
            }
        }

        Building& building = _techs.at(typeBuilding).at(j);
        building._production = factorMineBonus * refProduction;

        float energyCost = building._level * lifeFormTech->baseCost.energy * std::pow(lifeFormTech->energieFactor,building._level);
        energyCost *= getConsumptionReductionFactor();
        building._production.energy -= energyCost;
        _globalConsumption += energyCost;

        _productionStats[ProductionStat::BuildingLifeForm] += building._production;
    }
}

void Planet::computeProduction(const Ressources<float> &lifeFormBonuses)
{
    Ressources<float> refProduction(_rawRefProduction);
    // Crawlers percent
    float crawlerBonus = getCrawlerBonus();
    Ressources<float> crawlersPercent = Ressources(crawlerBonus, crawlerBonus, crawlerBonus);
    _productionStatsPercent[ProductionStatPercent::CrawlersPercent] = crawlersPercent;

    // Crawlers
    Ressources<int> crawlerProd = refProduction * crawlersPercent / 100.0f;
    float consumtion = (int)(50.f * (float)getActiveCrawler() * getConsumptionReductionFactor());
    crawlerProd.energy = -consumtion;
    _productionStats[ProductionStat::Crawlers] = crawlerProd;
    _globalConsumption += consumtion;

    // Buildings life form percent
    Ressources<float> lifeFormBuildingPercent = getLifeFormProdBonus();
    _productionStatsPercent[ProductionStatPercent::BuildingLifeFormPercent] = lifeFormBuildingPercent;

    _productionStats[ProductionStat::Plasma]            = refProduction * GeneralManager::instance().getPlasmaBonus() * 0.01f;
    _productionStats[ProductionStat::Ingenior]          = refProduction * GeneralManager::instance().getIngeniorBonus() * 0.01f;
    _productionStats[ProductionStat::Geolog]            = refProduction * GeneralManager::instance().getGeologBonus() * 0.01f;
    _productionStats[ProductionStat::Council]           = refProduction * GeneralManager::instance().getCouncilBonus() * 0.01f;
    _productionStats[ProductionStat::Class]             = refProduction * GeneralManager::instance().getClassBonus() * 0.01f;
    _productionStats[ProductionStat::AllianceClass]     = refProduction * GeneralManager::instance().getAllianceClassBonus() * 0.01f;
    _productionStats[ProductionStat::LifeFormResearch]  = refProduction * lifeFormBonuses * 0.01f;

    // Total
    _productionStats[ProductionStat::Total] = Ressources<float>();
    for (int i = 0; i < static_cast<int>(ProductionStat::Total); i++)
    {
        ProductionStat stat = static_cast<ProductionStat>(i);
        _productionStats[ProductionStat::Total] += _productionStats.at(stat);
    }
}

void Planet::computePoints()
{
    _allInvestedPoints.clear();
    _usefullInvestedPoints.clear();

    // Common buildings
    int numberTech = getNumberTech(TechType::CommonBuilding);
    for (int i = 0; i < numberTech; ++i)
    {

        int level = getTechLevel(TechType::CommonBuilding, i);
        Ressources<float> cumulPoints = TechManager::instance().getCumulativeCost(TechType::CommonBuilding, i, level) / 1000;

        _allInvestedPoints += cumulPoints;
        _usefullInvestedPoints += cumulPoints;
    }

    // Life form buildings
    TechType typeBuilding = speciesToBuildingLifeForm.at(_species);
    int lifeFormBuildingNumber = TechManager::instance().getNumberTechs(TechType::HumanBuilding);

    static std::array<TechType, 4> buildingTypes = {
        TechType::HumanBuilding,
        TechType::MechBuilding,
        TechType::KaeleshBuilding,
        TechType::RoctasBuilding
    };

    for (int j = 0; j < 4; ++j)
    {
        TechType currentTechType = buildingTypes.at(j);
        for (int i = 0; i < lifeFormBuildingNumber; i++)
        {
            int level = getTechLevel(currentTechType, i);
            Ressources<float> cumulPoints = TechManager::instance().getCumulativeCost(currentTechType, i, level) / 1000;
            _allInvestedPoints += cumulPoints;

            if (currentTechType == typeBuilding)
            {
                _usefullInvestedPoints += cumulPoints;
            }
        }
    }

    // Life form researches
    int lifeFormResearchNumber = TechManager::instance().getNumberTechs(TechType::HumanResearch);

    static std::array<TechType, 4> researchTypes = {
        TechType::HumanResearch,
        TechType::MechResearch,
        TechType::KaeleshResearch,
        TechType::RoctasResearch
    };

    for (int j = 0; j < 4; ++j)
    {
        TechType currentTechType = researchTypes.at(j);
        for (int i = 0; i < lifeFormResearchNumber; i++)
        {
            int level = getTechLevel(currentTechType, i);
            Ressources<float> cumulPoints = TechManager::instance().getCumulativeCost(currentTechType, i, level) / 1000;
            _allInvestedPoints += cumulPoints;

            if (currentTechType == speciesToTechLifeForm.at(_choicesLifeFormResearch.at(i)))
            {
                _usefullInvestedPoints += cumulPoints;
            }
        }
    }

    // Defenses
    for (auto itr = _defenses.begin(); itr != _defenses.end(); ++itr)
    {
        FixUnitType unitType = itr->first;
        int number = itr->second;

        const Unit& unit = TechManager::instance().getFixUnit(unitType);
        Ressources<float> points = number * unit.cost / 1000;
        _allInvestedPoints += points;
        _usefullInvestedPoints += points;
    }
}

Ressources<float> Planet::getLifeFormProdBonus() const
{
    float metalBonus    = _lifeFormBuildingBonuses.at(BonusLifeFormBuilding::Metal);
    float cristalBonus  = _lifeFormBuildingBonuses.at(BonusLifeFormBuilding::Cristal);
    float deutBonus     = _lifeFormBuildingBonuses.at(BonusLifeFormBuilding::Deut);
    return Ressources<float>(metalBonus, cristalBonus, deutBonus);
}

Ressources<int> Planet::getProduction(CommonBuildingType mineType, int level) const
{
    Ressources<int> prodHours;
    switch(mineType)
    {
    case CommonBuildingType::MineMetal:
        prodHours.metal = _bonusProdPositionCoeff.metal * 30.0f * (float)level * std::pow(1.1, level);
        break;
    case CommonBuildingType::MineCristal:
        prodHours.cristal = _bonusProdPositionCoeff.cristal * 20.0f * (float)level * std::pow(1.1, level);
        break;
    case CommonBuildingType::MineDeut:
        prodHours.deut = _bonusProdPositionCoeff.deut * 10.0f * (float)level * std::pow(1.1f, level) * (1.44f - 0.004f * (float)_temperatureMax);
        break;
    case CommonBuildingType::CentraleSolaire:
        prodHours.energy = 20.0f * (float)level * std::pow(1.1f, level);
        break;
    case CommonBuildingType::CentraleFusion:
    {
        int levelEnergy = GeneralManager::instance().getResearchLevel(ResearchType::Energie);
        prodHours.energy = 30.0f * (float)level * std::pow(1.05f + 0.01f * (float)levelEnergy, level);
        break;
    }
    }
    prodHours.metal *= 24;
    prodHours.cristal *= 24;
    prodHours.deut *= 24;
    return prodHours;
}

Ressources<int> Planet::getConsumption(CommonBuildingType mineType, int level) const
{
    Ressources<int> prodHours;
    switch(mineType)
    {
    case CommonBuildingType::MineMetal:
        prodHours.energy = 10.0f * (float)level * std::pow(1.1, level);
        break;
    case CommonBuildingType::MineCristal:
        prodHours.energy = 10.0f * (float)level * std::pow(1.1, level);
        break;
    case CommonBuildingType::MineDeut:
        prodHours.energy = 20.0f * (float)level * std::pow(1.1, level);
        break;
    case CommonBuildingType::CentraleFusion:
    {
        prodHours.deut = 10.0f * (float)level * std::pow(1.1, level);
        break;
    }
    }
    prodHours.metal *= 24;
    prodHours.cristal *= 24;
    prodHours.deut *= 24;
    prodHours.energy *= getConsumptionReductionFactor();
    return prodHours;
}


int Planet::getSatelliteProduction() const
{
    return getDefense(FixUnitType::SolarSatellite) * ((_temperatureMax + 140) / 6);
}

Ressources<float> Planet::getCost(TechType techType, int indexTech, int level) const
{
    Ressources<float> basicCost = TechManager::instance().getCost(techType, indexTech, level);
    float factor;
    switch (techType)
    {
    case TechType::CommonBuilding:
    {
        CommonBuildingType buildingType = static_cast<CommonBuildingType>(indexTech);
        switch(buildingType)
        {
            case CommonBuildingType::MineMetal:
            case CommonBuildingType::MineCristal:
            case CommonBuildingType::MineDeut:
                factor = 1.0f - _lifeFormBuildingBonuses.at(BonusLifeFormBuilding::ReducMineCostPercent) / 100.0f;
                break;
            default:
                factor = 1.0f;
                break;
        }
        break;
    }
    case TechType::HumanBuilding:
    case TechType::MechBuilding:
    case TechType::KaeleshBuilding:
    case TechType::RoctasBuilding:
        factor = 1.0f - _lifeFormBuildingBonuses.at(BonusLifeFormBuilding::ReducLifeFormBuildingCostPercent) / 100.0f;
        // Production of ressources
        if (indexTech <= 1)
        {
            factor -= _lifeFormBuildingBonuses.at(BonusLifeFormBuilding::ReducMineCostPercent) / 100.0f;
        }
        break;
    case TechType::HumanResearch:
    case TechType::MechResearch:
    case TechType::KaeleshResearch:
    case TechType::RoctasResearch:
        factor = 1.0f - _lifeFormBuildingBonuses.at(BonusLifeFormBuilding::ReducLifeFormResearchCostPercent) / 100.0f;
        break;
    default:
        factor = 1.0f;
        break;
    }
    return factor * basicCost;
}

float Planet::getTime(TechType techType, int indexTech, int level) const
{
    int robotLevel = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::UsineRobots));
    int nanitLevel = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::UsineNanite));
    float timeDays = TechManager::instance().getBaseTime(techType, indexTech, level);

    switch (techType)
    {
    case TechType::CommonBuilding:
    {
        timeDays /= (float)(1 + robotLevel);
        timeDays /= (float)std::pow(2, nanitLevel);
        break;
    }
    case TechType::HumanBuilding:
    case TechType::MechBuilding:
    case TechType::KaeleshBuilding:
    case TechType::RoctasBuilding:
    {
        float factor = 1.0f - _lifeFormBuildingBonuses.at(BonusLifeFormBuilding::ReducLifeFormBuildingDurationPercent) / 100.0f;
        timeDays /= (float)(1 + robotLevel);
        timeDays /= (float)std::pow(2, nanitLevel);
        timeDays *= factor;
        break;
    }
    case TechType::HumanResearch:
    case TechType::MechResearch:
    case TechType::KaeleshResearch:
    case TechType::RoctasResearch:
    {
        // Building reduction + research reduction
        float totalBonus = _lifeFormBuildingBonuses.at(BonusLifeFormBuilding::ReducLifeFormResearchDurationPercent)
                           + PlayerManager::instance().getLifeFormBonus(BonusLifeForm::LifeFormResearchDuration);
        float factor = 1.0f - totalBonus / 100.0f;
        factor = std::max(0.0f, factor);
        timeDays *= factor;
        break;
    }
    default:
        return -1.0f;
    }
    return timeDays;
}

int Planet::getDefense(FixUnitType unitType) const
{
    auto it = _defenses.find(unitType);
    return it == _defenses.end() ? 0 : it->second;
}

Ressources<float> Planet::getAllBuildingCost() const
{
    Ressources<float> globalCost;
    std::vector<TechType> techTypes = getAvailableBuildings();
    for (TechType techType : techTypes)
    {
        int numberTechs = TechManager::instance().getNumberTechs(techType);
        for (int i = 0; i < numberTechs; ++i)
        {
            int level = getTechLevel(techType, i);
            for (int j = 1; j <= level; ++j)
            {
                globalCost += getCost(techType, i, j);
            }
        }
    }
    return globalCost;
}

Ressources<float> Planet::getAllBuildingCostUpgrade(const Planet& planet) const
{
    Ressources<float> globalCost;
    std::vector<TechType> techTypes = getAvailableBuildings();
    for (TechType techType : techTypes)
    {
        int numberTechs = TechManager::instance().getNumberTechs(techType);
        for (int i = 0; i < numberTechs; ++i)
        {
            int previousLevel = planet.getTechLevel(techType, i);
            int targetLevel = getTechLevel(techType, i);
            for (int j = previousLevel + 1; j <= targetLevel; ++j)
            {
                globalCost += getCost(techType, i, j);
            }
        }
    }
    return globalCost;
}

float Planet::getConsumptionReductionFactor() const
{
    float reductionConsumptionPercent = _lifeFormBuildingBonuses.at(BonusLifeFormBuilding::ReducEnergyConsumptionPercent);
    float reductionConsumptionFactor = 1.0f - reductionConsumptionPercent / 100.0f;
    return reductionConsumptionFactor;
}

void Planet::setTechLevel(TechType type, int index, int level)
{
    _techs[type][index]._level = level;
}

float Planet::getLifeFormBonus(BonusLifeForm bonus) const
{
    auto it = _lifeFormBonuses.find(bonus);
    return it != _lifeFormBonuses.end() ? _lifeFormBonuses.at(bonus) : 0.0f;
}

int Planet::getMaxActiveCrawler() const
{
    int levelMetal = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineMetal));
    int levelCristal = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineCristal));
    int levelDeut = getTechLevel(TechType::CommonBuilding, static_cast<int>(CommonBuildingType::MineDeut));

    return 8 * (levelMetal + levelCristal + levelDeut);
}

int Planet::getActiveCrawler() const
{
    int maxCrawler = getMaxActiveCrawler();
    return std::min(maxCrawler, getDefense(FixUnitType::Crawler));
}

float Planet::getCrawlerBonus() const
{
    int maxCrawler = getActiveCrawler();
    int activeCrawlers = std::min(maxCrawler, getDefense(FixUnitType::Crawler));
    return std::min(50.0f, 0.02f * activeCrawlers);
}

Ressources<float> Planet::getLifeFormBuildingProduction() const
{
    Ressources<float> bonus = 0.01f * static_cast<Ressources<float>>(_productionStatsPercent.at(ProductionStatPercent::BuildingLifeFormPercent));
    Ressources<float> prodMines = _rawRefProduction;
    return Ressources<float>(bonus.metal * prodMines.metal, bonus.cristal * prodMines.cristal, bonus.deut * prodMines.deut);
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
    return _techs.at(lifeFormResearch).at(index)._level;
}

void Planet::setLevelLifeFormResearch(Species species, int index, int level)
{
    if (species == Species::None) {return;}

    TechType lifeFormResearch = speciesToTechLifeForm.at(species);
    _techs[lifeFormResearch][index]._level = level;
}
