#include "rentabilityManager.h"
#include "playerManager.h"
#include "techManager.h"
#include "discoveryManager.h"

RentabilityManager& RentabilityManager::instance()
{
    static RentabilityManager instance;
    return instance;
}

RentabilityManager::RentabilityManager()
{
    int nbPlanet = PlayerManager::instance().getNumberPlanets();
    _filterPlanet.resize(nbPlanet, true);
    _typeFilter[TypeFilter::BuildingFilter]           = true;
    _typeFilter[TypeFilter::ResearchFilter]           = true;
    _typeFilter[TypeFilter::LifeFormBuildingFilter]   = true;
    _typeFilter[TypeFilter::LifeFormResearchFilter]   = true;
}

int RentabilityManager::refresh()
{
    DiscoveryManager::instance().refresh();

    rentaLevelUp.clear();

    if (_typeFilter.at(TypeFilter::ResearchFilter))
        addReasearchRentability();

    int nbPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < nbPlanet; ++i)
    {
        if (!_filterPlanet.at(i)) continue;

        const Planet& planet = PlayerManager::instance().getPlanet(i);

        if (_typeFilter.at(TypeFilter::BuildingFilter))
            addMinesRentability(planet, i);

        if (_typeFilter.at(TypeFilter::LifeFormBuildingFilter))
            addLifeFormBuilding(planet, i);

        if (_typeFilter.at(TypeFilter::LifeFormResearchFilter))
            addLifeFormResearch(planet, i);

        addLevelUpLifeForm(planet, i);
    }

    std::sort(rentaLevelUp.begin(), rentaLevelUp.end(),
              [](const LevelUp& a, const LevelUp& b) {
                  return a.timeToRecover < b.timeToRecover;
              });

    return rentaLevelUp.size();
}

Ressources<float> RentabilityManager::getGain(const LifeFormTech* tech, int numberOfLevels) const
{
    Ressources<float> bonusRessources;

    int levelSpecies = PlayerManager::instance().getSpecies(tech->species);
    float factorSpecies = 1.0f + levelSpecies / 1000.0f;
    Ressources<float> prodMines = PlayerManager::instance().getProduction(PlayerManager::ProductionStat::Mines);

    for (auto itr = tech->bonuses.begin(); itr != tech->bonuses.end(); ++itr)
    {
        float bonusFactor = itr->second / 100.0f * factorSpecies;
        switch(itr->first)
        {
        case BonusLifeForm::Metal:
            bonusRessources += numberOfLevels * bonusFactor * Ressources(prodMines.metal, 0.0f, 0.0f);
            break;
        case BonusLifeForm::Cristal:
            bonusRessources += numberOfLevels * bonusFactor * Ressources(0.0f, prodMines.cristal, 0.0f);
            break;
        case BonusLifeForm::Deuterium:
            bonusRessources += numberOfLevels * bonusFactor * Ressources(0.0f, 0.0f, prodMines.deut);
            break;
        case BonusLifeForm::Antimatter:
        {
            float antimatterMean = DiscoveryManager::instance().getSummary().meanRessourceFound.antimatter;
            bonusRessources = numberOfLevels * bonusFactor * Ressources(0.0f, 0.0f, 0.0f, 0.0f, antimatterMean);
            break;
        }
        case BonusLifeForm::ExpeditionRessourcesIncrease:
        {
            Ressources<float> ressourcesBase = DiscoveryManager::instance().getSummary().meanRessourceFound;
            float percentDiscover = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::ExploratorClass);
            float factorDiscover = 1.0f + percentDiscover / 100.0f;
            bonusRessources += numberOfLevels * bonusFactor * factorDiscover * ressourcesBase;
            break;
        }
        case BonusLifeForm::ExpeditionShipIncrease:
            bonusRessources += numberOfLevels * bonusFactor * DiscoveryManager::instance().getSummary().meanShipFound;
            break;
        case BonusLifeForm::LargeCargoUpdate:
        case BonusLifeForm::SpeedCivilianShips:
        case BonusLifeForm::SpeedAllShips:
            bonusRessources += DiscoveryManager::instance().computeReductionTimeDiscovery(numberOfLevels * bonusFactor * 100.0f);
            break;
        case BonusLifeForm::ExploratorClass:
        {
            Ressources<float> ressourcesBase = DiscoveryManager::instance().getSummary().meanRessourceFound;
            float percentRessources = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::ExpeditionRessourcesIncrease);
            float factorRessources = 1.0f + percentRessources / 100.0f;
            bonusRessources += numberOfLevels * bonusFactor * factorRessources * ressourcesBase;
        }
        }
    }

    return bonusRessources;
}

void RentabilityManager::addNewLevelUp(LevelUp levelUp, std::optional<int> indexPlanet)
{
    levelUp.computeRenta(PlayerManager::instance().getConversionRate());

    auto it = std::find_if(rentaLevelUp.begin(), rentaLevelUp.end(), [this, levelUp](const LevelUp& current)
    {
        return current._name == levelUp._name
                && current._levelToUpdate == levelUp._levelToUpdate
                && std::abs(current.timeToRecover - levelUp.timeToRecover) < 1.0f;
    });

    if (it == rentaLevelUp.end())
    {
        if (indexPlanet)
        {
            levelUp.indexPlanets.push_back(indexPlanet.value());
        }
        rentaLevelUp.push_back(std::move(levelUp));

    }
    else if(indexPlanet)
    {
        it->indexPlanets.push_back(indexPlanet.value());
    }
}

void RentabilityManager::addReasearchRentability()
{
    // Plasma
    Ressources prodMines = PlayerManager::instance().getProduction(PlayerManager::ProductionStat::Mines);
    Ressources bonusProdPercent = prodMines * Ressources(0.01f, 0.00667f, 0.00333f);

    int indexPlasma = static_cast<int>(ResearchType::Plasma);
    int levelPlasma = PlayerManager::instance().getResearchLevel(ResearchType::Plasma) + 1;

    LevelUp levelUpPlasma ("Plasma", levelPlasma);
    levelUpPlasma.rentaPerDay = bonusProdPercent;
    levelUpPlasma.cost = TechManager::instance().getCost(TechType::CommonResearch, indexPlasma, levelPlasma);
    levelUpPlasma.timeToCompleteDay = PlayerManager::instance().getResearchTime(indexPlasma, levelPlasma);
    addNewLevelUp(std::move(levelUpPlasma));

    // Prop. combustion
    int indexCombu = static_cast<int>(ResearchType::PropCombusion);
    int levelCombu = PlayerManager::instance().getResearchLevel(ResearchType::PropCombusion) + 1;

    LevelUp levelUpCombu ("Combustion", levelCombu);
    levelUpCombu.rentaPerDay = DiscoveryManager::instance().computeReductionTimeDiscovery(10.0f);
    levelUpCombu.cost = TechManager::instance().getCost(TechType::CommonResearch, indexCombu, levelCombu);
    levelUpCombu.timeToCompleteDay = PlayerManager::instance().getResearchTime(indexCombu, levelCombu);
    addNewLevelUp(std::move(levelUpCombu));

    addAstroRentability();
}

void RentabilityManager::addAstroRentability()
{
    Planet& planet = PlayerManager::instance().getPlanifAstro();
    planet.computeLifeFormResearch(PlayerManager::instance().getAllSpecies());
    planet.refresh();
    Ressources<float> globalCost, globalGain;

    // Compute life form bonuses
    std::map<BonusLifeForm, float> bonusAstro;
    std::map<BonusLifeForm, float> bonusTotal;
    for (int j = 0; j < static_cast<int>(BonusLifeForm::Count); ++j)
    {
        BonusLifeForm typeBonus = static_cast<BonusLifeForm>(j);
        bonusAstro[typeBonus] += planet.getLifeFormBonus(typeBonus);
        bonusTotal[typeBonus] += bonusAstro[typeBonus];
        bonusTotal[typeBonus] += PlayerManager::instance().getLifeFormBonus(typeBonus);
    }

    // Compute mines gain
    Ressources<float> bonusPercent;
    bonusPercent += planet.getProductionStatPercent(Planet::ProductionStatPercent::BuildingLifeFormPercent);
    bonusPercent += planet.getProductionStatPercent(Planet::ProductionStatPercent::CrawlersPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::PlasmaPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::LifeFormBonusPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::GeologPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::ClassBonusPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::AllianceBonusPercent);

    int indexMetal = static_cast<int>(CommonBuildingType::MineMetal);
    int indexCristal = static_cast<int>(CommonBuildingType::MineCristal);
    int indexDeut = static_cast<int>(CommonBuildingType::MineDeut);

    int levelMetal = planet.getTechLevel(TechType::CommonBuilding, indexMetal);
    int levelCristal = planet.getTechLevel(TechType::CommonBuilding, indexCristal);
    int levelDeut = planet.getTechLevel(TechType::CommonBuilding, indexDeut);

    int temperatureMax = planet.getTemperatureMax();
    Ressources<float> bonus = planet.getBonusMine();
    Ressources<float> gainMinesRaw;
    gainMinesRaw.metal     = TechManager::instance().getProductionMine(CommonBuildingType::MineMetal, levelMetal, bonus.metal);
    gainMinesRaw.cristal   = TechManager::instance().getProductionMine(CommonBuildingType::MineCristal, levelCristal, bonus.cristal);
    gainMinesRaw.deut      = TechManager::instance().getProductionMine(CommonBuildingType::MineDeut, levelDeut, bonus.deut, temperatureMax);
    globalGain = gainMinesRaw + gainMinesRaw * bonusPercent * 0.01f;

    // All building cost
    std::vector<TechType> techTypes = planet.getAvailableBuildings();
    for (TechType techType : techTypes)
    {
        int numberTechs = TechManager::instance().getNumberTechs(techType);
        for (int i = 0; i < numberTechs; ++i)
        {
            int level = planet.getTechLevel(techType, i);
            for (int j = 1; j <= level; ++j)
            {
                globalCost += planet.getCost(techType, i, j);
            }
        }
    }

    // Life form research cost
    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberLifeFormReseach; ++i)
    {
        Species speciesChoice = planet.getChoiceLifeFormResearch(i);

        if (speciesChoice == Species::None) {continue;}

        TechType lifeFormResearch = speciesToTechLifeForm.at(speciesChoice);
        int level = planet.getLevelLifeFormResearch(speciesChoice, i);
        for (int j = 1; j <= level; ++j)
        {
            globalCost += planet.getCost(lifeFormResearch, i, j);
        }

        TechType researchLifeFormType = speciesToTechLifeForm.at(speciesChoice);
        const LifeFormTech* tech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(researchLifeFormType, i));
        globalGain += getGain(tech, level);
    }

    // Units cost
    for (int i = 0; i < static_cast<int>(FixUnitType::Count); ++i)
    {
        FixUnitType unitType = static_cast<FixUnitType>(i);
        Unit unit = TechManager::instance().getFixUnit(unitType);
        int number = planet.getDefense(unitType);

        globalCost += number * unit.cost;
    }

    int indexAstro = static_cast<int>(ResearchType::Astrophysique);
    int levelAstro = PlayerManager::instance().getResearchLevel(ResearchType::Astrophysique) + 1;

    float time = PlayerManager::instance().getResearchTime(indexAstro, levelAstro);
    globalCost += TechManager::instance().getCost(TechType::CommonResearch, indexAstro, levelAstro);
    if (levelAstro % 2 == 0)
    {
        levelAstro++;
        time += PlayerManager::instance().getResearchTime(indexAstro, levelAstro);
        globalCost += TechManager::instance().getCost(TechType::CommonResearch, indexAstro, levelAstro);
    }

    LevelUp levelUpAstro ("Astro", levelAstro);
    levelUpAstro.rentaPerDay = globalGain;
    levelUpAstro.cost = globalCost;
    levelUpAstro.timeToCompleteDay = time;
    addNewLevelUp(std::move(levelUpAstro));
}

void RentabilityManager::addMinesRentability(const Planet& planet, int indexPlanet)
{
    int temperatureMax = planet.getTemperatureMax();
    Ressources<float> bonus = planet.getBonusMine();

    int indexMetal = static_cast<int>(CommonBuildingType::MineMetal);
    int indexCristal = static_cast<int>(CommonBuildingType::MineCristal);
    int indexDeut = static_cast<int>(CommonBuildingType::MineDeut);

    int levelMetal = planet.getTechLevel(TechType::CommonBuilding, indexMetal) + 1;
    int levelCristal = planet.getTechLevel(TechType::CommonBuilding, indexCristal) + 1;
    int levelDeut = planet.getTechLevel(TechType::CommonBuilding, indexDeut) + 1;

    Ressources<float> prodMinesLevelUp;

    prodMinesLevelUp.metal = TechManager::instance().getProductionMine(CommonBuildingType::MineMetal, levelMetal, bonus.metal)
        - TechManager::instance().getProductionMine(CommonBuildingType::MineMetal, levelMetal - 1, bonus.metal);
    prodMinesLevelUp.cristal = TechManager::instance().getProductionMine(CommonBuildingType::MineCristal, levelCristal, bonus.cristal)
        - TechManager::instance().getProductionMine(CommonBuildingType::MineCristal, levelCristal - 1, bonus.cristal);
    prodMinesLevelUp.deut = TechManager::instance().getProductionMine(CommonBuildingType::MineDeut, levelDeut, bonus.deut, temperatureMax)
        - TechManager::instance().getProductionMine(CommonBuildingType::MineDeut, levelDeut - 1, bonus.deut, temperatureMax);

    Ressources<float> bonusPercent;
    bonusPercent += planet.getProductionStatPercent(Planet::ProductionStatPercent::BuildingLifeFormPercent);
    bonusPercent += planet.getProductionStatPercent(Planet::ProductionStatPercent::CrawlersPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::PlasmaPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::LifeFormBonusPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::GeologPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::ClassBonusPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::AllianceBonusPercent);

    prodMinesLevelUp += prodMinesLevelUp * bonusPercent * 0.01f;

    LevelUp levelUpMetal("Mine de métal", levelMetal);
    levelUpMetal.rentaPerDay = Ressources<float>(prodMinesLevelUp.metal, 0, 0);
    levelUpMetal.cost = planet.getCost(TechType::CommonBuilding, indexMetal, levelMetal);
    levelUpMetal.timeToCompleteDay = planet.getTime(TechType::CommonBuilding, indexMetal, levelMetal);
    addNewLevelUp(std::move(levelUpMetal), indexPlanet);

    LevelUp levelUpCristal("Mine de cristal", levelCristal);
    levelUpCristal.rentaPerDay = Ressources<float>(0, prodMinesLevelUp.cristal, 0);
    levelUpCristal.cost = planet.getCost(TechType::CommonBuilding, indexCristal, levelCristal);
    levelUpCristal.timeToCompleteDay = planet.getTime(TechType::CommonBuilding, indexCristal, levelCristal);
    addNewLevelUp(std::move(levelUpCristal), indexPlanet);

    LevelUp levelUpDeut("Mine de deut", levelDeut);
    levelUpDeut.rentaPerDay = Ressources<float>(0, 0, prodMinesLevelUp.deut);
    levelUpDeut.cost = planet.getCost(TechType::CommonBuilding, indexDeut, levelDeut);
    levelUpDeut.timeToCompleteDay = planet.getTime(TechType::CommonBuilding, indexDeut, levelDeut);
    addNewLevelUp(std::move(levelUpDeut), indexPlanet);
}

void RentabilityManager::addLifeFormBuilding(const Planet& planet, int indexPlanet)
{
    Ressources<float> prodMines = planet.getProductionStat(Planet::ProductionStat::Mines);

    TechType buildingType = speciesToBuildingLifeForm.at(planet.getSpecies());
    int numberBuildingLifeForm = TechManager::instance().getNumberTechs(buildingType);
    for (int i = 0; i < numberBuildingLifeForm; i++)
    {
        Ressources<float> bonusProdPercent;
        bool bonusFound = false;
        const LifeFormBuilding* tech = dynamic_cast<const LifeFormBuilding*>(TechManager::instance().getTech(buildingType, i));
        for (auto itr = tech->bonuses.begin(); itr != tech->bonuses.end(); ++itr)
        {
            switch(itr->first)
            {
            case BonusLifeFormBuilding::Metal:
                bonusProdPercent += Ressources<float>(itr->second, 0.0f, 0.0f);
                bonusFound = true;
                break;
            case BonusLifeFormBuilding::Cristal:
                bonusProdPercent += Ressources<float>(0.0f, itr->second, 0.0f);
                bonusFound = true;
                break;
            case BonusLifeFormBuilding::Deut:
                bonusProdPercent += Ressources<float>(0.0f, 0.0f, itr->second);
                bonusFound = true;
                break;
            }
        }

        if (!bonusFound)
        {
            continue;
        }

        Ressources bonusProd = prodMines * bonusProdPercent * 0.01f;

        int levelUpBatiment = planet.getTechLevel(buildingType, i) + 1;

        LevelUp levelUpBuilding(tech->name, levelUpBatiment);
        levelUpBuilding.rentaPerDay = bonusProd;
        levelUpBuilding.cost = planet.getCost(buildingType, i, levelUpBatiment);
        levelUpBuilding.timeToCompleteDay = planet.getTime(buildingType, i, levelUpBatiment);
        addNewLevelUp(std::move(levelUpBuilding), indexPlanet);
    }
}

void RentabilityManager::addLifeFormResearch(const Planet& planet, int indexPlanet)
{
    int numberResearchLifeForm = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberResearchLifeForm; i++)
    {
        Species speciesLifeForm = planet.getChoiceLifeFormResearch(i);
        if (speciesLifeForm == Species::None) continue;

        TechType researchLifeFormType = speciesToTechLifeForm.at(speciesLifeForm);
        const LifeFormTech* tech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(researchLifeFormType, i));
        int levelUpResearch = planet.getTechLevel(researchLifeFormType, i) + 1;

        Ressources<float> bonusRessources = getGain(tech);
        if (!bonusRessources.empty())
        {
            const LifeFormTech* tech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(researchLifeFormType, i));

            LevelUp levelUpLifeFormResearch(tech->name, levelUpResearch);
            levelUpLifeFormResearch.rentaPerDay         = bonusRessources;
            levelUpLifeFormResearch.cost                = planet.getCost(researchLifeFormType, i, levelUpResearch);
            levelUpLifeFormResearch.timeToCompleteDay   = planet.getTime(researchLifeFormType, i, levelUpResearch);
            addNewLevelUp(std::move(levelUpLifeFormResearch), indexPlanet);
        }
    }
}

void RentabilityManager::addLevelUpLifeForm(const Planet& planet, int indexPlanet)
{
    const Planet& planifPlanet = PlayerManager::instance().getPlanifPlanet(planet.getSpecies());
    Ressources<float> globalCost;
    Ressources<float> globalGain;
    float globalTime;

    // All building cost
    std::vector<TechType> techTypes = planet.getAvailableBuildings();
    for (TechType techType : techTypes)
    {
        if (techType == TechType::None) { continue;}

        int numberTechs = TechManager::instance().getNumberTechs(techType);
        for (int i = 0; i < numberTechs; ++i)
        {
            int level = planet.getTechLevel(techType, i);
            int levelPlanif = planifPlanet.getTechLevel(techType, i);

            if (level >= levelPlanif) continue;

            for (int j = level + 1; j <= levelPlanif; ++j)
            {
                globalCost += planifPlanet.getCost(techType, i, j);
                globalTime += planifPlanet.getTime(techType, i, j);
            }
        }
    }

    // Life form research
    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberLifeFormReseach; ++i)
    {
        Species speciesChoice = planifPlanet.getChoiceLifeFormResearch(i);
        int level = planifPlanet.getLevelLifeFormResearch(speciesChoice, i);
        TechType lifeFormResearch = speciesToTechLifeForm.at(speciesChoice);

        for (int j = 1; j <= level; ++j)
        {
            TechType researchLifeFormType = speciesToTechLifeForm.at(speciesChoice);
            const LifeFormTech* tech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(researchLifeFormType, i));

            globalGain += getGain(tech);
            globalCost += planifPlanet.getCost(lifeFormResearch, i, j);
        }
    }

    LevelUp levelUpLifeForm ("Level up LF", 0);
    levelUpLifeForm.rentaPerDay         = globalGain;
    levelUpLifeForm.cost                = globalCost;
    levelUpLifeForm.timeToCompleteDay   = globalTime;
    addNewLevelUp(std::move(levelUpLifeForm), indexPlanet);
}
