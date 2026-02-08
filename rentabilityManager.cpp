#include "rentabilityManager.h"
#include "playerManager.h"
#include "techManager.h"

RentabilityManager& RentabilityManager::instance()
{
    static RentabilityManager instance;
    return instance;
}

RentabilityManager::RentabilityManager() {}

int RentabilityManager::refresh()
{
    rentaLevelUp.clear();
    int nbPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < nbPlanet; ++i)
    {
        const Planet* planet = PlayerManager::instance().getPlanet(i);
        addMinesRentability(planet, i);
    }

    std::sort(rentaLevelUp.begin(), rentaLevelUp.end(),
              [](const LevelUp& a, const LevelUp& b) {
                  return a.timeToRecover < b.timeToRecover;
              });

    return rentaLevelUp.size();
}

const RentabilityManager::LevelUp& RentabilityManager::getLevelUp(int index) const
{
    return rentaLevelUp.at(index);
}

QString RentabilityManager::levelUpToString(LevelUpType levelUp)
{
    switch (levelUp)
    {
        case LevelUpType::MetalMine:
            return QStringLiteral("Mine de métal");
        case LevelUpType::CristalMine:
            return QStringLiteral("Mine de cristal");
        case LevelUpType::DeutMine:
            return QStringLiteral("Mine de deut");
        case LevelUpType::BatBonusMetal:
            return QStringLiteral("Bâtiment bonus métal");
        case LevelUpType::BatBonusCristal:
            return QStringLiteral("Bâtiment bonus cristal");
        case LevelUpType::BatBonusDeut:
            return QStringLiteral("Bâtiment bonus deut");
        case LevelUpType::Crawlers:
            return QStringLiteral("Foreuses");
        case LevelUpType::Plasma:
            return QStringLiteral("Plasma");
    }

    return QStringLiteral("Inconnu");
}

void RentabilityManager::addMinesRentability(const Planet* planet, int indexPlanet)
{
    int temperatureMax = planet->getTemperatureMax();
    Ressources bonus = planet->getBonusMine();

    int indexMetal = static_cast<int>(CommonBuildingType::MineMetal);
    int indexCristal = static_cast<int>(CommonBuildingType::MineCristal);
    int indexDeut = static_cast<int>(CommonBuildingType::MineDeut);

    int levelMetal = planet->getTechLevel(TechType::CommonBuilding, indexMetal) + 1;
    int levelCristal = planet->getTechLevel(TechType::CommonBuilding, indexCristal) + 1;
    int levelDeut = planet->getTechLevel(TechType::CommonBuilding, indexDeut) + 1;

    Ressources prodMinesLevelUp;

    prodMinesLevelUp.metal = TechManager::instance().getProductionMine(CommonBuildingType::MineMetal, levelMetal, bonus.metal)
        - TechManager::instance().getProductionMine(CommonBuildingType::MineMetal, levelMetal - 1, bonus.metal);
    prodMinesLevelUp.cristal = TechManager::instance().getProductionMine(CommonBuildingType::MineCristal, levelCristal, bonus.cristal)
        - TechManager::instance().getProductionMine(CommonBuildingType::MineCristal, levelCristal - 1, bonus.cristal);
    prodMinesLevelUp.deut = TechManager::instance().getProductionMine(CommonBuildingType::MineDeut, levelDeut, bonus.deut, temperatureMax)
        - TechManager::instance().getProductionMine(CommonBuildingType::MineDeut, levelDeut - 1, bonus.deut, temperatureMax);

    Ressources bonusPercent;
    bonusPercent += planet->getProductionStat(Planet::ProductionStatPlanet::BuildingLifeFormPercent);
    bonusPercent += planet->getProductionStat(Planet::ProductionStatPlanet::CrawlersPercent);
    bonusPercent += PlayerManager::instance().getProduction(PlayerManager::ProductionStat::PlasmaPercent);
    bonusPercent += PlayerManager::instance().getProduction(PlayerManager::ProductionStat::LifeFormBonusPercent);
    bonusPercent += PlayerManager::instance().getProduction(PlayerManager::ProductionStat::GeologPercent);
    bonusPercent += PlayerManager::instance().getProduction(PlayerManager::ProductionStat::ClassBonusPercent);
    bonusPercent += PlayerManager::instance().getProduction(PlayerManager::ProductionStat::AllianceBonusPercent);

    prodMinesLevelUp += prodMinesLevelUp * bonusPercent * 0.01f;

    LevelUp levelUpMetal;
    levelUpMetal.type = LevelUpType::MetalMine;
    levelUpMetal.indexPlanet = indexPlanet;
    levelUpMetal.levelToUpdate = levelMetal;
    levelUpMetal.rentaPerHour = Ressources(prodMinesLevelUp.metal, 0, 0);
    levelUpMetal.cost = TechManager::instance().getCost(TechType::CommonBuilding, indexMetal, levelMetal);
    levelUpMetal.computeRenta(PlayerManager::instance().getConversionRate());
    levelUpMetal.timeToRecoverStr = rentaToString(levelUpMetal.timeToRecover);
    rentaLevelUp.push_back(levelUpMetal);

    LevelUp levelUpCristal;
    levelUpCristal.type = LevelUpType::CristalMine;
    levelUpCristal.indexPlanet = indexPlanet;
    levelUpCristal.levelToUpdate = levelCristal;
    levelUpCristal.rentaPerHour = Ressources(0, prodMinesLevelUp.cristal, 0);
    levelUpCristal.cost = TechManager::instance().getCost(TechType::CommonBuilding, indexCristal, levelCristal);
    levelUpCristal.computeRenta(PlayerManager::instance().getConversionRate());
    levelUpCristal.timeToRecoverStr = rentaToString(levelUpCristal.timeToRecover);
    rentaLevelUp.push_back(levelUpCristal);

    LevelUp levelUpDeut;
    levelUpDeut.type = LevelUpType::DeutMine;
    levelUpDeut.indexPlanet = indexPlanet;
    levelUpDeut.levelToUpdate = levelDeut;
    levelUpDeut.rentaPerHour = Ressources(0, 0, prodMinesLevelUp.deut);
    levelUpDeut.cost = TechManager::instance().getCost(TechType::CommonBuilding, indexDeut, levelDeut);
    levelUpDeut.computeRenta(PlayerManager::instance().getConversionRate());
    levelUpDeut.timeToRecoverStr = rentaToString(levelUpDeut.timeToRecover);
    rentaLevelUp.push_back(levelUpDeut);
}

void RentabilityManager::addLifeFormBuilding(const Planet *planet, int indexPlanet)
{
    Ressources prodMines = planet->getProductionStat(Planet::ProductionStatPlanet::Mines);

    Species species = planet->getSpecies();
    TechType buildingTech = TechManager::instance().getBuildingTech(species);

    Ressources bonusBuilding = planet->getProductionStat(Planet::ProductionStatPlanet::BuildingLifeFormPercent);
}

QString RentabilityManager::rentaToString(float timeToRecover) const
{
    if (timeToRecover < 24.0f)
    {
        return QString::number(timeToRecover) + "h";
    }
    else if (timeToRecover < 24.0f * 7.0f)
    {
        return QString::number(timeToRecover / 24.0f) + "j";
    }
    else if (timeToRecover < 24.0f * 30.0f)
    {
        return QString::number(timeToRecover / 24.0f / 7.0f) + "s";
    }
    else if (timeToRecover < 24.0f * 30.0f * 12.0f)
    {
        return QString::number(timeToRecover / 24.0f / 30.0f) + "m";
    }
    else
    {
        return QString::number(timeToRecover / 24.0f / 30.0f / 12.0f) + "a";
    }
}
