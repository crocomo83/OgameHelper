#include "rentabilityManager.h"
#include "playerManager.h"
#include "techManager.h"
#include "discoveryManager.h"

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
        addLifeFormResearch(planet, i);
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

void RentabilityManager::addNewLevelUp(LevelUp levelUp)
{
    auto it = std::find_if(rentaLevelUp.begin(), rentaLevelUp.end(), [this, levelUp](const LevelUp& current)
    {
        return current.name == levelUp.name
                && current.levelToUpdate == levelUp.levelToUpdate
                && std::abs(current.timeToRecover - levelUp.timeToRecover) < 1.0f;
    });

    if (it == rentaLevelUp.end())
    {
        rentaLevelUp.push_back(std::move(levelUp));
    }
    else
    {
        it->numberInstance++;
    }
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
    levelUpMetal.cost = planet->getCost(TechType::CommonBuilding, indexMetal, levelMetal);
    levelUpMetal.timeToCompleteDay = planet->getTime(TechType::CommonBuilding, indexMetal, levelMetal);
    levelUpMetal.computeRenta(PlayerManager::instance().getConversionRate());
    levelUpMetal.timeToRecoverStr = rentaToString(levelUpMetal.timeToRecover);
    addNewLevelUp(std::move(levelUpMetal));

    LevelUp levelUpCristal;
    levelUpCristal.name = "Mine de cristal";
    levelUpCristal.indexPlanet = indexPlanet;
    levelUpCristal.levelToUpdate = levelCristal;
    levelUpCristal.rentaPerDay = Ressources(0, prodMinesLevelUp.cristal, 0);
    levelUpCristal.cost = planet->getCost(TechType::CommonBuilding, indexCristal, levelCristal);
    levelUpCristal.timeToCompleteDay = planet->getTime(TechType::CommonBuilding, indexCristal, levelCristal);
    levelUpCristal.computeRenta(PlayerManager::instance().getConversionRate());
    levelUpCristal.timeToRecoverStr = rentaToString(levelUpCristal.timeToRecover);
    addNewLevelUp(std::move(levelUpCristal));

    LevelUp levelUpDeut;
    levelUpDeut.name = "Mine de deut";
    levelUpDeut.indexPlanet = indexPlanet;
    levelUpDeut.levelToUpdate = levelDeut;
    levelUpDeut.rentaPerDay = Ressources(0, 0, prodMinesLevelUp.deut);
    levelUpDeut.cost = planet->getCost(TechType::CommonBuilding, indexDeut, levelDeut);
    levelUpDeut.timeToCompleteDay = planet->getTime(TechType::CommonBuilding, indexDeut, levelDeut);
    levelUpDeut.computeRenta(PlayerManager::instance().getConversionRate());
    levelUpDeut.timeToRecoverStr = rentaToString(levelUpDeut.timeToRecover);
    addNewLevelUp(std::move(levelUpDeut));
}

void RentabilityManager::addLifeFormBuilding(const Planet *planet, int indexPlanet)
{
    Ressources prodMines = planet->getProductionStat(Planet::ProductionStatPlanet::Mines);

    TechType buildingType = speciesToBuildingLifeForm.at(planet->getSpecies());
    int numberBuildingLifeForm = TechManager::instance().getNumberTechs(buildingType);
    for (int i = 0; i < numberBuildingLifeForm; i++)
    {
        Ressources bonusProdPercent;
        bool bonusFound = false;
        const LifeFormBuilding* tech = dynamic_cast<const LifeFormBuilding*>(TechManager::instance().getTech(buildingType, i));
        for (auto itr = tech->bonuses.begin(); itr != tech->bonuses.end(); ++itr)
        {
            switch(itr->first)
            {
            case BonusLifeFormBuilding::Metal:
                bonusProdPercent += Ressources(itr->second, 0.0f, 0.0f);
                bonusFound = true;
                break;
            case BonusLifeFormBuilding::Cristal:
                bonusProdPercent += Ressources(0.0f, itr->second, 0.0f);
                bonusFound = true;
                break;
            case BonusLifeFormBuilding::Deut:
                bonusProdPercent += Ressources(0.0f, 0.0f, itr->second);
                bonusFound = true;
                break;
            }
        }

        if (!bonusFound)
        {
            continue;
        }

        Ressources bonusProd = prodMines * bonusProdPercent * 0.01f;

        int levelUpBatiment = planet->getTechLevel(buildingType, i) + 1;

        LevelUp levelUpBuilding;
        levelUpBuilding.name = tech->name;
        levelUpBuilding.indexPlanet = indexPlanet;
        levelUpBuilding.levelToUpdate = levelUpBatiment;
        levelUpBuilding.rentaPerDay = bonusProd;
        levelUpBuilding.cost = planet->getCost(buildingType, i, levelUpBatiment);
        levelUpBuilding.timeToCompleteDay = planet->getTime(buildingType, i, levelUpBatiment);
        levelUpBuilding.computeRenta(PlayerManager::instance().getConversionRate());
        levelUpBuilding.timeToRecoverStr = rentaToString(levelUpBuilding.timeToRecover);
        addNewLevelUp(std::move(levelUpBuilding));
    }
}

void RentabilityManager::addLifeFormResearch(const Planet *planet, int indexPlanet)
{
    DiscoveryManager::instance().computeRentability();

    int numberResearchLifeForm = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberResearchLifeForm; i++)
    {
        Ressources bonusProdPercent;
        bool bonusMiningFound = false;

        Species speciesLifeForm = planet->getChoiceLifeFormResearch(i);
        if (speciesLifeForm == Species::None)
        {
            continue;
        }
        TechType researchLifeFormType = speciesToTechLifeForm.at(speciesLifeForm);
        const LifeFormTech* tech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(researchLifeFormType, i));

        int levelSpecies = PlayerManager::instance().getSpecies(tech->species);
        float factorSpecies = 1.0f + (float)levelSpecies / 1000.0f;

        for (auto itr = tech->bonuses.begin(); itr != tech->bonuses.end(); ++itr)
        {
            switch(itr->first)
            {
            case BonusLifeForm::Metal:
                bonusProdPercent += Ressources(itr->second, 0.0f, 0.0f);
                bonusMiningFound = true;
                break;
            case BonusLifeForm::Cristal:
                bonusProdPercent += Ressources(0.0f, itr->second, 0.0f);
                bonusMiningFound = true;
                break;
            case BonusLifeForm::Deuterium:
                bonusProdPercent += Ressources(0.0f, 0.0f, itr->second);
                bonusMiningFound = true;
                break;
            case BonusLifeForm::ExpeditionRessourcesIncrease:
            {
                const DiscoveryManager::SummaryPerDay& summary = DiscoveryManager::instance().getSummary();
                const Ressources& mean = summary.meanRessourceFound - summary.meanLost;
                float bonusPercent = itr->second * factorSpecies;

                int levelUpResearch = planet->getTechLevel(researchLifeFormType, i) + 1;

                LevelUp levelUpLifeFormResearch;
                levelUpLifeFormResearch.name = tech->name;
                levelUpLifeFormResearch.indexPlanet = indexPlanet;
                levelUpLifeFormResearch.levelToUpdate = levelUpResearch;
                levelUpLifeFormResearch.rentaPerDay = mean * bonusPercent / 100.0f;
                levelUpLifeFormResearch.cost = planet->getCost(researchLifeFormType, i, levelUpResearch);
                levelUpLifeFormResearch.timeToCompleteDay = planet->getTime(researchLifeFormType, i, levelUpResearch);
                levelUpLifeFormResearch.computeRenta(PlayerManager::instance().getConversionRate());
                levelUpLifeFormResearch.timeToRecoverStr = rentaToString(levelUpLifeFormResearch.timeToRecover);
                addNewLevelUp(std::move(levelUpLifeFormResearch));
                break;
            }
            case BonusLifeForm::ExpeditionShipIncrease:
            {
                const DiscoveryManager::SummaryPerDay& summary = DiscoveryManager::instance().getSummary();
                const Ressources& mean = summary.meanShipFound - summary.meanLost;
                float bonusPercent = itr->second * factorSpecies;

                int levelUpResearch = planet->getTechLevel(researchLifeFormType, i) + 1;

                LevelUp levelUpLifeFormResearch;
                levelUpLifeFormResearch.name = tech->name;
                levelUpLifeFormResearch.indexPlanet = indexPlanet;
                levelUpLifeFormResearch.levelToUpdate = levelUpResearch;
                levelUpLifeFormResearch.rentaPerDay = mean * bonusPercent / 100.0f;
                levelUpLifeFormResearch.cost = planet->getCost(researchLifeFormType, i, levelUpResearch);
                levelUpLifeFormResearch.timeToCompleteDay = planet->getTime(researchLifeFormType, i, levelUpResearch);
                levelUpLifeFormResearch.computeRenta(PlayerManager::instance().getConversionRate());
                levelUpLifeFormResearch.timeToRecoverStr = rentaToString(levelUpLifeFormResearch.timeToRecover);
                addNewLevelUp(std::move(levelUpLifeFormResearch));
                break;
            }
            }
        }

        if (bonusMiningFound)
        {
            Ressources prodMines = PlayerManager::instance().getProduction(PlayerManager::ProductionStat::Mines);
            Ressources bonusProd = prodMines * bonusProdPercent * factorSpecies * 0.01f;

            int levelUpResearch = planet->getTechLevel(researchLifeFormType, i) + 1;

            LevelUp levelUpLifeFormResearch;
            levelUpLifeFormResearch.name = tech->name;
            levelUpLifeFormResearch.indexPlanet = indexPlanet;
            levelUpLifeFormResearch.levelToUpdate = levelUpResearch;
            levelUpLifeFormResearch.rentaPerDay = bonusProd;
            levelUpLifeFormResearch.cost = planet->getCost(researchLifeFormType, i, levelUpResearch);
            levelUpLifeFormResearch.timeToCompleteDay = planet->getTime(researchLifeFormType, i, levelUpResearch);
            levelUpLifeFormResearch.computeRenta(PlayerManager::instance().getConversionRate());
            levelUpLifeFormResearch.timeToRecoverStr = rentaToString(levelUpLifeFormResearch.timeToRecover);
            addNewLevelUp(std::move(levelUpLifeFormResearch));
        }
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
