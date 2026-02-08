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
        addLifeFormBuilding(planet, i);
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
    levelUpMetal.name = "Mine de métal";
    levelUpMetal.indexPlanet = indexPlanet;
    levelUpMetal.levelToUpdate = levelMetal;
    levelUpMetal.rentaPerDay = Ressources(prodMinesLevelUp.metal, 0, 0);
    levelUpMetal.cost = TechManager::instance().getCost(TechType::CommonBuilding, indexMetal, levelMetal);
    levelUpMetal.computeRenta(PlayerManager::instance().getConversionRate());
    levelUpMetal.timeToRecoverStr = rentaToString(levelUpMetal.timeToRecover);
    rentaLevelUp.push_back(levelUpMetal);

    LevelUp levelUpCristal;
    levelUpCristal.name = "Mine de cristal";
    levelUpCristal.indexPlanet = indexPlanet;
    levelUpCristal.levelToUpdate = levelCristal;
    levelUpCristal.rentaPerDay = Ressources(0, prodMinesLevelUp.cristal, 0);
    levelUpCristal.cost = TechManager::instance().getCost(TechType::CommonBuilding, indexCristal, levelCristal);
    levelUpCristal.computeRenta(PlayerManager::instance().getConversionRate());
    levelUpCristal.timeToRecoverStr = rentaToString(levelUpCristal.timeToRecover);
    rentaLevelUp.push_back(levelUpCristal);

    LevelUp levelUpDeut;
    levelUpDeut.name = "Mine de deut";
    levelUpDeut.indexPlanet = indexPlanet;
    levelUpDeut.levelToUpdate = levelDeut;
    levelUpDeut.rentaPerDay = Ressources(0, 0, prodMinesLevelUp.deut);
    levelUpDeut.cost = TechManager::instance().getCost(TechType::CommonBuilding, indexDeut, levelDeut);
    levelUpDeut.computeRenta(PlayerManager::instance().getConversionRate());
    levelUpDeut.timeToRecoverStr = rentaToString(levelUpDeut.timeToRecover);
    rentaLevelUp.push_back(levelUpDeut);
}

void RentabilityManager::addLifeFormBuilding(const Planet *planet, int indexPlanet)
{
    Ressources prodMines = planet->getProductionStat(Planet::ProductionStatPlanet::Mines);

    TechType buildingType = speciesToBuildingLifeForm.at(planet->getSpecies());
    int numberBuildingLifeForm = TechManager::instance().getNumberTechs(buildingType);
    for (int i = 0; i < numberBuildingLifeForm; i++)
    {
        Ressources bonusProdPercent;
        const LifeFormBuilding* tech = dynamic_cast<const LifeFormBuilding*>(TechManager::instance().getTech(buildingType, i));
        for (auto itr = tech->bonuses.begin(); itr != tech->bonuses.end(); ++itr)
        {
            switch(itr->first)
            {
            case BonusLifeFormBuilding::Metal:
                bonusProdPercent += Ressources(itr->second, 0.0f, 0.0f);
                break;
            case BonusLifeFormBuilding::Cristal:
                bonusProdPercent += Ressources(0.0f, itr->second, 0.0f);
                break;
            case BonusLifeFormBuilding::Deut:
                bonusProdPercent += Ressources(0.0f, 0.0f, itr->second);
                break;
            }
        }

        Ressources bonusProd = prodMines * bonusProdPercent * 0.01f;

        int levelUpBatiment = planet->getTechLevel(buildingType, i) + 1;

        LevelUp levelUpBuilding;
        levelUpBuilding.name = tech->name;
        levelUpBuilding.indexPlanet = indexPlanet;
        levelUpBuilding.levelToUpdate = levelUpBatiment;
        levelUpBuilding.rentaPerDay = bonusProd;
        levelUpBuilding.cost = TechManager::instance().getCost(buildingType, i, levelUpBatiment);
        levelUpBuilding.computeRenta(PlayerManager::instance().getConversionRate());
        levelUpBuilding.timeToRecoverStr = rentaToString(levelUpBuilding.timeToRecover);
        rentaLevelUp.push_back(levelUpBuilding);
    }
}

QString RentabilityManager::rentaToString(float timeToRecover) const
{
    if (timeToRecover < 7.0f)
    {
        return QString::number(timeToRecover) + "j";
    }
    else if (timeToRecover < 30.0f)
    {
        return QString::number(timeToRecover / 7.0f) + "s";
    }
    else if (timeToRecover < 365.0f)
    {
        return QString::number(timeToRecover / 30.0f) + "m";
    }
    else
    {
        return QString::number(timeToRecover / 365.0f) + "a";
    }
}
