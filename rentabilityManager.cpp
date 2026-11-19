#include "rentabilityManager.h"
#include "techManager.h"
#include "generalManager.h"
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
    PlayerManager::instance().refresh();
    DiscoveryManager::instance().refresh();

    int nbPlanet = PlayerManager::instance().getNumberPlanets();
    if (_filterPlanet.size() != nbPlanet)
    {
        _filterPlanet.resize(nbPlanet, true);
    }

    rentaLevelUp.clear();

    switch (_type)
    {
    case RentabilityType::Ressources:
        refreshRessources();
        break;
    case RentabilityType::Energy:
        refreshEnergy();
        break;
    case RentabilityType::BuildingSpeed:
        refreshBuildingSpeed();
        break;
    case RentabilityType::ResearchSpeed:
        refreshResearchSpeed();
        break;
    case RentabilityType::LifeFormResearchSpeed:
        refreshLifeFormResearchSpeed();
        break;
    }

    std::sort(rentaLevelUp.begin(), rentaLevelUp.end(),
              [](const std::unique_ptr<LevelUp>& a, const std::unique_ptr<LevelUp>& b) {
                  return a->_score < b->_score;
              });

    return rentaLevelUp.size();
}

void RentabilityManager::refreshRessources()
{
    if (_typeFilter.at(TypeFilter::ResearchFilter))
        addReasearchRentability();

    int nbPlanif = PlayerManager::instance().getNumberPlanifs();
    for (int i = 0; i < nbPlanif; ++i)
    {
        PlayerManager::Planification* planif = PlayerManager::instance().getPlanif(i);
        if (planif->type == PlayerManager::PlanificationType::NewColony)
        {
            addAstroRentability(planif);
        }
    }

    int nbPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < nbPlanet; ++i)
    {
        if (!_filterPlanet.at(i)) continue;

        if (_typeFilter.at(TypeFilter::BuildingFilter))
        {
            addMinesRentability(i);
        }

        if (_typeFilter.at(TypeFilter::LifeFormBuildingFilter))
        {
            addLifeFormBuilding(i);
        }

        if (_typeFilter.at(TypeFilter::LifeFormResearchFilter))
        {
            addLifeFormResearch(i);
        }

        for (int j = 0; j < nbPlanif; ++j)
        {
            if (PlayerManager::Planification* planif = PlayerManager::instance().getPlanif(j))
            {
                switch (planif->type)
                {
                case PlayerManager::PlanificationType::ChangeSpecies:
                    addPlanifChgtSpecies(planif, i);
                    break;
                case PlayerManager::PlanificationType::LevelUpFDV:
                    addLevelUpLifeForm(planif, i);
                    break;
                }
            }
            else
            {
                qCritical() << "Planif " << j << " not found";
            }
        }
    }
}

void RentabilityManager::refreshEnergy()
{
    if (_typeFilter.at(TypeFilter::ResearchFilter))
        addResearchForEnergy();

    int nbPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < nbPlanet; ++i)
    {
        if (!_filterPlanet.at(i)) continue;

        if (_typeFilter.at(TypeFilter::BuildingFilter))
        {
            addBuildingForEnergy(i);
        }
    }
}

void RentabilityManager::refreshBuildingSpeed()
{
    int nbPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < nbPlanet; ++i)
    {
        if (!_filterPlanet.at(i)) continue;

        if (_typeFilter.at(TypeFilter::BuildingFilter))
        {
            addSpeedBuildingForBuilding(i);
        }
    }
}

void RentabilityManager::refreshResearchSpeed()
{
    if (_typeFilter.at(TypeFilter::ResearchFilter))
        addSpeedResearchForResearch();

    int nbPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < nbPlanet; ++i)
    {
        if (!_filterPlanet.at(i)) continue;

        if (_typeFilter.at(TypeFilter::BuildingFilter))
        {
            addSpeedBuildingForResearch(i);
        }

        if (_typeFilter.at(TypeFilter::LifeFormResearchFilter))
        {
            addSpeedResearchLifeFormForResearch(i, BonusLifeForm::ResearchDuration);
        }
    }
}


void RentabilityManager::refreshLifeFormResearchSpeed()
{
    int nbPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < nbPlanet; ++i)
    {
        if (!_filterPlanet.at(i)) continue;

        if (_typeFilter.at(TypeFilter::LifeFormBuildingFilter))
        {
            addSpeedBuildingLifeFormForResearchLifeForm(i);
        }

        if (_typeFilter.at(TypeFilter::LifeFormResearchFilter))
        {
            addSpeedResearchLifeFormForResearch(i, BonusLifeForm::LifeFormResearchDuration);
        }
    }
}

Ressources<float> RentabilityManager::getGainMines(const Planet& planet) const
{
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
    bonusPercent += planet.getProductionStatPercent(Planet::ProductionStatPercent::CrawlersPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::PlasmaPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::LifeFormBonusPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::GeologPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::ClassBonusPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::AllianceBonusPercent);

    int indexMetal      = static_cast<int>(CommonBuildingType::MineMetal);
    int indexCristal    = static_cast<int>(CommonBuildingType::MineCristal);
    int indexDeut       = static_cast<int>(CommonBuildingType::MineDeut);

    int levelMetal      = planet.getTechLevel(TechType::CommonBuilding, indexMetal);
    int levelCristal    = planet.getTechLevel(TechType::CommonBuilding, indexCristal);
    int levelDeut       = planet.getTechLevel(TechType::CommonBuilding, indexDeut);

    Ressources<float> gainMinesRaw;
    gainMinesRaw.metal     = planet.getProduction(CommonBuildingType::MineMetal, levelMetal).metal;
    gainMinesRaw.cristal   = planet.getProduction(CommonBuildingType::MineCristal, levelCristal).cristal;
    gainMinesRaw.deut      = planet.getProduction(CommonBuildingType::MineDeut, levelDeut).deut;
    return gainMinesRaw + gainMinesRaw * bonusPercent * 0.01f;
}

// TODO : move to Planet class
Ressources<float> RentabilityManager::getGainBuilding(const Planet* planet, const LifeFormBuilding* tech, int numberOfLevels) const
{
    Ressources<float> bonusRessources;
    Ressources<float> prodMines = planet->getRefRawProduction();
    for (auto itr = tech->bonuses.begin(); itr != tech->bonuses.end(); ++itr)
    {
        switch(itr->first)
        {
        case BonusLifeFormBuilding::Metal:
            bonusRessources += numberOfLevels * prodMines * Ressources<float>(itr->second, 0.0f, 0.0f) * 0.01f;
            break;
        case BonusLifeFormBuilding::Cristal:
            bonusRessources += numberOfLevels * prodMines * Ressources<float>(0.0f, itr->second, 0.0f) * 0.01f;
            break;
        case BonusLifeFormBuilding::Deut:
            bonusRessources += numberOfLevels * prodMines * Ressources<float>(0.0f, 0.0f, itr->second) * 0.01f;
            break;
        case BonusLifeFormBuilding::IncreaseLifeForm:
            // Life form research cost
            int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
            for (int i = 0; i < numberLifeFormReseach; ++i)
            {
                Species speciesChoice = planet->getChoiceLifeFormResearch(i);

                if (speciesChoice == Species::None) {continue;}

                TechType researchLifeFormType = speciesToTechLifeForm.at(speciesChoice);
                const LifeFormTech* tech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(researchLifeFormType, i));
                int level = planet->getLevelLifeFormResearch(speciesChoice, i);
                bonusRessources += numberOfLevels * itr->second * 0.01f * getGainResearch(tech, level);
            }
            break;
        }
    }
    return bonusRessources;
}

Ressources<float> RentabilityManager::getGainResearch(const LifeFormTech* tech, int numberOfLevels) const
{
    Ressources<float> bonusRessources;

    int levelSpecies = GeneralManager::instance().getLevelSpecies(tech->species);
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
        case BonusLifeForm::SpeedToExpedition:
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

float RentabilityManager::getGainTimePercent(const LifeFormBuilding* tech) const
{
    float bonusTime = 0.0f;
    for (auto itr = tech->bonuses.begin(); itr != tech->bonuses.end(); ++itr)
    {
        switch(itr->first)
        {
        case BonusLifeFormBuilding::ReducLifeFormResearchDurationPercent:
            bonusTime += itr->second;
            break;
        }
    }
    return bonusTime;
}

float RentabilityManager::getGainTimePercent(const LifeFormTech* tech, BonusLifeForm bonusType) const
{
    int levelSpecies = GeneralManager::instance().getLevelSpecies(tech->species);
    float factorSpecies = 1.0f + levelSpecies / 1000.0f;

    float bonusTime = 0.0f;

    for (auto itr = tech->bonuses.begin(); itr != tech->bonuses.end(); ++itr)
    {
        float bonusFactor = itr->second * factorSpecies;
        if (itr->first == bonusType)
        {
            bonusTime += bonusFactor;
        }
    }

    return bonusTime;
}

void RentabilityManager::addNewLevelUp(std::unique_ptr<LevelUp> levelUp, std::optional<int> indexPlanet)
{
    levelUp->computeRenta(GeneralManager::instance().getConversionRate());

    auto it = std::find_if(rentaLevelUp.begin(), rentaLevelUp.end(), [this, &levelUp](const std::unique_ptr<LevelUp>& current)
    {
        return current->_name == levelUp->_name
                && current->_levelToUpdate == levelUp->_levelToUpdate
                && std::abs(current->_score - levelUp->_score) < 1.0f;
    });

    if (it == rentaLevelUp.end())
    {
        if (indexPlanet)
        {
            levelUp->indexPlanets.push_back(indexPlanet.value());
        }
        rentaLevelUp.push_back(std::move(levelUp));

    }
    else if(indexPlanet.has_value())
    {
        it->get()->indexPlanets.push_back(*indexPlanet);
    }
}

void RentabilityManager::addReasearchRentability()
{
    // Plasma
    Ressources prodMines = PlayerManager::instance().getProduction(PlayerManager::ProductionStat::Mines);
    Ressources bonusProdPercent = prodMines * Ressources(0.01f, 0.00667f, 0.00333f);

    int indexPlasma = static_cast<int>(ResearchType::Plasma);
    int levelPlasma = GeneralManager::instance().getResearchLevel(ResearchType::Plasma) + 1;

    ProductionLevelUp levelUpPlasma ("Plasma", levelPlasma);
    levelUpPlasma.rentaPerDay = bonusProdPercent;
    levelUpPlasma.cost = TechManager::instance().getCost(TechType::CommonResearch, indexPlasma, levelPlasma);
    levelUpPlasma.timeToCompleteDay = PlayerManager::instance().getResearchTime(indexPlasma, levelPlasma);
    addNewLevelUp(std::make_unique<ProductionLevelUp>(levelUpPlasma));

    // Prop. combustion
    int indexCombu = static_cast<int>(ResearchType::PropCombusion);
    int levelCombu = GeneralManager::instance().getResearchLevel(ResearchType::PropCombusion) + 1;

    ProductionLevelUp levelUpCombu ("Combustion", levelCombu);
    levelUpCombu.rentaPerDay = DiscoveryManager::instance().computeReductionTimeDiscovery(10.0f);
    levelUpCombu.cost = TechManager::instance().getCost(TechType::CommonResearch, indexCombu, levelCombu);
    levelUpCombu.timeToCompleteDay = PlayerManager::instance().getResearchTime(indexCombu, levelCombu);
    addNewLevelUp(std::make_unique<ProductionLevelUp>(levelUpCombu));
}

void RentabilityManager::addAstroRentability(PlayerManager::Planification* planif)
{
    Planet& planet = planif->planet;
    Ressources<float> globalCost, globalGain;

    globalGain += getGainMines(planet);

    // All building cost
    globalCost +=  planet.getAllBuildingCost();

    // Life form buildings gain
    TechType buildingType = speciesToBuildingLifeForm.at(planet.getSpecies());
    int numberBuildingLifeForm = TechManager::instance().getNumberTechs(buildingType);
    for (int i = 0; i < numberBuildingLifeForm; i++)
    {
        const LifeFormBuilding* tech = dynamic_cast<const LifeFormBuilding*>(TechManager::instance().getTech(buildingType, i));
        int level = planet.getTechLevel(buildingType, i);
        globalGain += getGainBuilding(&planet, tech, level);
    }

    // Life form research cost & gain
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
        globalGain += getGainResearch(tech, level);
    }

    int indexAstro = static_cast<int>(ResearchType::Astrophysique);
    int levelAstro = GeneralManager::instance().getResearchLevel(ResearchType::Astrophysique) + 1;

    float time = PlayerManager::instance().getResearchTime(indexAstro, levelAstro);
    globalCost += TechManager::instance().getCost(TechType::CommonResearch, indexAstro, levelAstro);
    if (levelAstro % 2 == 0)
    {
        levelAstro++;
        time += PlayerManager::instance().getResearchTime(indexAstro, levelAstro);
        globalCost += TechManager::instance().getCost(TechType::CommonResearch, indexAstro, levelAstro);
    }

    // Units cost
    for (int i = 0; i < static_cast<int>(FixUnitType::Count); ++i)
    {
        FixUnitType unitType = static_cast<FixUnitType>(i);
        Unit unit = TechManager::instance().getFixUnit(unitType);
        int number = planet.getDefense(unitType);

        globalCost += number * unit.cost;
    }

    ProductionLevelUp levelUpAstro (planet.getName() + ", astro", levelAstro);
    levelUpAstro.rentaPerDay = globalGain;
    levelUpAstro.cost = globalCost;
    levelUpAstro.timeToCompleteDay = time;
    addNewLevelUp(std::make_unique<ProductionLevelUp>(levelUpAstro));
}

void RentabilityManager::addPlanifChgtSpecies(PlayerManager::Planification* planif, int indexPlanet)
{
    const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);
    if (!planet)
    {
        return;
    }

    Planet& planifPlanet = planif->planet;

    if (planifPlanet.getSpecies() == planet->getSpecies() || planifPlanet.getSpecies() == Species::None)
        return;

    Ressources<float> globalCost, globalGain;
    float time = 0.0f;

    // All building cost
    globalCost +=  planifPlanet.getAllBuildingCostUpgrade(planet);

    // Life form buildings gain
    TechType buildingType = speciesToBuildingLifeForm.at(planifPlanet.getSpecies());
    int numberBuildingLifeForm = TechManager::instance().getNumberTechs(buildingType);
    for (int i = 0; i < numberBuildingLifeForm; i++)
    {
        const LifeFormBuilding* tech = dynamic_cast<const LifeFormBuilding*>(TechManager::instance().getTech(buildingType, i));
        int level = planifPlanet.getTechLevel(buildingType, i);
        globalGain += getGainBuilding(&planifPlanet, tech, level);
        time += planifPlanet.getTime(buildingType, i, level);
    }

    // Life form old buildings
    TechType buildingTypeOld = speciesToBuildingLifeForm.at(planet->getSpecies());
    if (buildingTypeOld == TechType::None)
    {
        return;
    }

    int numberBuildingLifeFormOld = TechManager::instance().getNumberTechs(buildingTypeOld);
    for (int i = 0; i < numberBuildingLifeFormOld; i++)
    {
        const LifeFormBuilding* tech = dynamic_cast<const LifeFormBuilding*>(TechManager::instance().getTech(buildingTypeOld, i));
        int level = planet->getTechLevel(buildingTypeOld, i);
        globalGain -= getGainBuilding(planet, tech, level);
    }

    // Life form research cost
    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberLifeFormReseach; ++i)
    {
        Species speciesChoice = planifPlanet.getChoiceLifeFormResearch(i);

        if (speciesChoice == Species::None) {continue;}

        TechType lifeFormResearch = speciesToTechLifeForm.at(speciesChoice);
        int level = planifPlanet.getLevelLifeFormResearch(speciesChoice, i);
        int previousLevel = planet->getLevelLifeFormResearch(speciesChoice, i);
        for (int j = previousLevel + 1; j <= level; ++j)
        {
            globalCost += planifPlanet.getCost(lifeFormResearch, i, j);
        }

        if (level > previousLevel)
        {
            TechType researchLifeFormType = speciesToTechLifeForm.at(speciesChoice);
            const LifeFormTech* tech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(researchLifeFormType, i));
            globalGain += getGainResearch(tech, level - previousLevel);
        }
    }

    ProductionLevelUp levelUpChangeSpecies ("Change species", 0);
    levelUpChangeSpecies.rentaPerDay = globalGain;
    levelUpChangeSpecies.cost = globalCost;
    levelUpChangeSpecies.timeToCompleteDay = time;
    addNewLevelUp(std::make_unique<ProductionLevelUp>(levelUpChangeSpecies), indexPlanet);
}

void RentabilityManager::addMinesRentability(int indexPlanet)
{
    //const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);
    Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);

    int indexMetal = static_cast<int>(CommonBuildingType::MineMetal);
    int indexCristal = static_cast<int>(CommonBuildingType::MineCristal);
    int indexDeut = static_cast<int>(CommonBuildingType::MineDeut);

    int levelMetal = planet->getTechLevel(TechType::CommonBuilding, indexMetal) + 1;
    int levelCristal = planet->getTechLevel(TechType::CommonBuilding, indexCristal) + 1;
    int levelDeut = planet->getTechLevel(TechType::CommonBuilding, indexDeut) + 1;

    Ressources<float> prodMinesLevelUp;

    prodMinesLevelUp.metal = planet->getProduction(CommonBuildingType::MineMetal, levelMetal).metal
        - planet->getProduction(CommonBuildingType::MineMetal, levelMetal - 1).metal;
    prodMinesLevelUp.cristal = planet->getProduction(CommonBuildingType::MineCristal, levelCristal).cristal
        - planet->getProduction(CommonBuildingType::MineCristal, levelCristal - 1).cristal;
    prodMinesLevelUp.deut = planet->getProduction(CommonBuildingType::MineDeut, levelDeut).deut
        - planet->getProduction(CommonBuildingType::MineDeut, levelDeut - 1).deut;

    Ressources<float> bonusPercent;
    bonusPercent += planet->getProductionStatPercent(Planet::ProductionStatPercent::BuildingLifeFormPercent);
    bonusPercent += planet->getProductionStatPercent(Planet::ProductionStatPercent::CrawlersPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::PlasmaPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::LifeFormBonusPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::GeologPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::ClassBonusPercent);
    bonusPercent += PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::AllianceBonusPercent);

    if (planet->getName() == "Coruscant")
    {
        qDebug() << "new deut prod : " << planet->getProduction(CommonBuildingType::MineDeut, levelDeut).deut / 24;
        qDebug() << "gain deut prod : " << prodMinesLevelUp.deut / 24;

        qDebug() << "bonus life form building percent : " << QString::number(planet->getProductionStatPercent(Planet::ProductionStatPercent::BuildingLifeFormPercent).deut);
        qDebug() << "bonus crawler percent : " << QString::number(planet->getProductionStatPercent(Planet::ProductionStatPercent::CrawlersPercent).deut);
        qDebug() << "bonus plasma percent : " << QString::number(PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::PlasmaPercent).deut);
        qDebug() << "bonus life form percent : " << QString::number(PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::LifeFormBonusPercent).deut);
        qDebug() << "bonus geolog percent : " << QString::number(PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::GeologPercent).deut);
        qDebug() << "bonus class percent : " << QString::number(PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::ClassBonusPercent).deut);
        qDebug() << "bonus alli class percent : " << QString::number(PlayerManager::instance().getProductionPercent(PlayerManager::ProductionStatPercent::AllianceBonusPercent).deut);
    }

    prodMinesLevelUp += prodMinesLevelUp * bonusPercent * 0.01f;

    ProductionLevelUp levelUpMetal("Mine de métal", levelMetal);
    levelUpMetal.rentaPerDay = Ressources<float>(prodMinesLevelUp.metal, 0, 0);
    levelUpMetal.cost = planet->getCost(TechType::CommonBuilding, indexMetal, levelMetal);
    levelUpMetal.timeToCompleteDay = planet->getTime(TechType::CommonBuilding, indexMetal, levelMetal);
    addNewLevelUp(std::make_unique<ProductionLevelUp>(levelUpMetal), indexPlanet);

    ProductionLevelUp levelUpCristal("Mine de cristal", levelCristal);
    levelUpCristal.rentaPerDay = Ressources<float>(0, prodMinesLevelUp.cristal, 0);
    levelUpCristal.cost = planet->getCost(TechType::CommonBuilding, indexCristal, levelCristal);
    levelUpCristal.timeToCompleteDay = planet->getTime(TechType::CommonBuilding, indexCristal, levelCristal);
    addNewLevelUp(std::make_unique<ProductionLevelUp>(levelUpCristal), indexPlanet);

    ProductionLevelUp levelUpDeut("Mine de deut", levelDeut);
    levelUpDeut.rentaPerDay = Ressources<float>(0, 0, prodMinesLevelUp.deut);
    levelUpDeut.cost = planet->getCost(TechType::CommonBuilding, indexDeut, levelDeut);
    levelUpDeut.timeToCompleteDay = planet->getTime(TechType::CommonBuilding, indexDeut, levelDeut);
    addNewLevelUp(std::make_unique<ProductionLevelUp>(levelUpDeut), indexPlanet);
}

void RentabilityManager::addLifeFormBuilding(int indexPlanet)
{
    const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);

    Species planetSpecies = planet->getSpecies();
    if (planetSpecies == Species::None) return;

    TechType buildingType = speciesToBuildingLifeForm.at(planetSpecies);
    int numberBuildingLifeForm = TechManager::instance().getNumberTechs(buildingType);
    for (int i = 0; i < numberBuildingLifeForm; i++)
    {
        const LifeFormBuilding* tech = dynamic_cast<const LifeFormBuilding*>(TechManager::instance().getTech(buildingType, i));
        Ressources<float> bonusRessources = getGainBuilding(planet, tech);
        if (!bonusRessources.empty())
        {
            int levelUpBatiment = planet->getTechLevel(buildingType, i) + 1;

            ProductionLevelUp levelUpBuilding(tech->name, levelUpBatiment);
            levelUpBuilding.rentaPerDay = bonusRessources;
            levelUpBuilding.cost = planet->getCost(buildingType, i, levelUpBatiment);
            levelUpBuilding.timeToCompleteDay = planet->getTime(buildingType, i, levelUpBatiment);
            addNewLevelUp(std::make_unique<ProductionLevelUp>(levelUpBuilding), indexPlanet);
        }
    }
}

void RentabilityManager::addLifeFormResearch(int indexPlanet)
{
    const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);

    int numberResearchLifeForm = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberResearchLifeForm; i++)
    {
        Species speciesLifeForm = planet->getChoiceLifeFormResearch(i);
        if (speciesLifeForm == Species::None) continue;

        TechType researchLifeFormType = speciesToTechLifeForm.at(speciesLifeForm);
        const LifeFormTech* tech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(researchLifeFormType, i));

        Ressources<float> bonusRessources = getGainResearch(tech);
        if (!bonusRessources.empty())
        {
            const LifeFormTech* tech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(researchLifeFormType, i));
            int levelUpResearch = planet->getTechLevel(researchLifeFormType, i) + 1;

            ProductionLevelUp levelUpLifeFormResearch(tech->name, levelUpResearch);
            levelUpLifeFormResearch.rentaPerDay         = bonusRessources;
            levelUpLifeFormResearch.cost                = planet->getCost(researchLifeFormType, i, levelUpResearch);
            levelUpLifeFormResearch.timeToCompleteDay   = planet->getTime(researchLifeFormType, i, levelUpResearch);
            addNewLevelUp(std::make_unique<ProductionLevelUp>(levelUpLifeFormResearch), indexPlanet);
        }
    }
}

void RentabilityManager::addLevelUpLifeForm(PlayerManager::Planification* planif, int indexPlanet)
{
    const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);
    const Planet& planifPlanet = planif->planet;
    Ressources<float> globalCost;
    Ressources<float> globalGain;
    float globalTime = 0.0f;

    // All building cost
    std::vector<TechType> techTypes = planet->getAvailableBuildings();
    for (TechType techType : techTypes)
    {
        if (techType == TechType::None) { continue;}

        int numberTechs = TechManager::instance().getNumberTechs(techType);
        for (int i = 0; i < numberTechs; ++i)
        {
            int level = planet->getTechLevel(techType, i);
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

            globalGain += getGainResearch(tech);
            globalCost += planifPlanet.getCost(lifeFormResearch, i, j);
        }
    }

    ProductionLevelUp levelUpLifeForm ("Level up LF", 0);
    levelUpLifeForm.rentaPerDay         = globalGain;
    levelUpLifeForm.cost                = globalCost;
    levelUpLifeForm.timeToCompleteDay   = globalTime;
    addNewLevelUp(std::make_unique<ProductionLevelUp>(levelUpLifeForm), indexPlanet);
}

void RentabilityManager::addBuildingForEnergy(int indexPlanet)
{
    const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);

    int indexSolarCentral = static_cast<int>(CommonBuildingType::CentraleSolaire);
    int indexFusionCentral = static_cast<int>(CommonBuildingType::CentraleFusion);

    int levelSolarCentral = planet->getTechLevel(TechType::CommonBuilding, indexSolarCentral) + 1;
    int levelFusionCentral = planet->getTechLevel(TechType::CommonBuilding, indexFusionCentral) + 1;

    int solarCentralProd = planet->getProduction(CommonBuildingType::CentraleSolaire, levelSolarCentral).energy
        - planet->getProduction(CommonBuildingType::CentraleSolaire, levelSolarCentral - 1).energy;

    int fusionCentralProd = planet->getProduction(CommonBuildingType::CentraleFusion, levelFusionCentral).energy
        - planet->getProduction(CommonBuildingType::CentraleFusion, levelFusionCentral - 1).energy;

    EnergyLevelUp solarCentralLevelUp("Solar powerplant", levelSolarCentral);
    solarCentralLevelUp._energyGain = solarCentralProd;
    solarCentralLevelUp.cost = planet->getCost(TechType::CommonBuilding, indexSolarCentral, levelSolarCentral);
    solarCentralLevelUp.timeToCompleteDay = planet->getTime(TechType::CommonBuilding, indexSolarCentral, levelSolarCentral);
    addNewLevelUp(std::make_unique<EnergyLevelUp>(solarCentralLevelUp), indexPlanet);

    EnergyLevelUp fusionCentralLevelUp("Fusion powerplant", levelFusionCentral);
    fusionCentralLevelUp._energyGain = fusionCentralProd;
    fusionCentralLevelUp.cost = planet->getCost(TechType::CommonBuilding, indexFusionCentral, levelFusionCentral);
    fusionCentralLevelUp.timeToCompleteDay = planet->getTime(TechType::CommonBuilding, indexFusionCentral, levelFusionCentral);
    addNewLevelUp(std::make_unique<EnergyLevelUp>(fusionCentralLevelUp), indexPlanet);
}

void RentabilityManager::addResearchForEnergy()
{
    int indexEnergy = static_cast<int>(ResearchType::Energie);
    int indexFusionCentral = static_cast<int>(CommonBuildingType::CentraleFusion);

    int levelTargetEnergy = GeneralManager::instance().getResearchLevel(ResearchType::Energie) + 1;

    int gainEnergy = 0;
    int numberPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < numberPlanet; ++i)
    {
        const Planet* planet = PlayerManager::instance().getPlanet(i);

        int levelFusion = planet->getTechLevel(TechType::CommonBuilding, indexFusionCentral);
        int currentGain = TechManager::instance().getProductionFusionPowerplant(levelFusion, levelTargetEnergy)
            - TechManager::instance().getProductionFusionPowerplant(levelFusion, levelTargetEnergy - 1);
        gainEnergy += currentGain;
    }

    EnergyLevelUp energyLevelUp("Energy", levelTargetEnergy);
    energyLevelUp._energyGain = gainEnergy;
    energyLevelUp.cost = TechManager::instance().getCost(TechType::CommonResearch, indexEnergy, levelTargetEnergy);
    energyLevelUp.timeToCompleteDay = PlayerManager::instance().getResearchTime(indexEnergy, levelTargetEnergy);
    addNewLevelUp(std::make_unique<EnergyLevelUp>(energyLevelUp));
}

void RentabilityManager::addSpeedBuildingForBuilding(int indexPlanet)
{
    const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);

    int indexRobots = static_cast<int>(CommonBuildingType::UsineRobots);
    int indexNanites = static_cast<int>(CommonBuildingType::UsineNanite);

    int levelRobots = planet->getTechLevel(TechType::CommonBuilding, indexRobots);
    int levelNanites = planet->getTechLevel(TechType::CommonBuilding, indexNanites);

    SpeedLevelUp levelUpRobots ("Level up robots", levelRobots + 1);
    levelUpRobots._percentsGain       = 100.0f * (1.0f - ((float)(levelRobots + 1) / (float)(levelRobots + 2)));
    levelUpRobots.cost                = planet->getCost(TechType::CommonBuilding, indexRobots, levelRobots + 1);
    levelUpRobots.timeToCompleteDay   = planet->getTime(TechType::CommonBuilding, indexRobots, levelRobots + 1);
    addNewLevelUp(std::make_unique<SpeedLevelUp>(levelUpRobots), indexPlanet);

    SpeedLevelUp levelUpNanites ("Level up nanites", levelNanites + 1);
    levelUpNanites._percentsGain       = 50;
    levelUpNanites.cost                = planet->getCost(TechType::CommonBuilding, indexNanites, levelNanites + 1);
    levelUpNanites.timeToCompleteDay   = planet->getTime(TechType::CommonBuilding, indexNanites, levelNanites + 1);
    addNewLevelUp(std::make_unique<SpeedLevelUp>(levelUpNanites), indexPlanet);
}

void RentabilityManager::addSpeedBuildingForResearch(int indexPlanet)
{
    const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);

    if (!planet || !planet->hasWorkingLab()) return;

    int indexLabo = static_cast<int>(CommonBuildingType::LaboRecherche);
    int levelLabo = planet->getTechLevel(TechType::CommonBuilding, indexLabo);
    int globalLabLevel = PlayerManager::instance().getGlobalLabLevel();

    SpeedLevelUp levelUpLabo ("Level up lab", levelLabo + 1);
    levelUpLabo._percentsGain       = 100.0f * (1.0f - ((float)(globalLabLevel + 1) / (float)(globalLabLevel + 2)));
    levelUpLabo.cost                = planet->getCost(TechType::CommonBuilding, indexLabo, levelLabo + 1);
    levelUpLabo.timeToCompleteDay   = planet->getTime(TechType::CommonBuilding, indexLabo, levelLabo + 1);
    addNewLevelUp(std::make_unique<SpeedLevelUp>(levelUpLabo), indexPlanet);
}

void RentabilityManager::addSpeedResearchForResearch()
{
    int indexReseau = static_cast<int>(ResearchType::Reseau);
    int indexLab = static_cast<int>(CommonBuildingType::LaboRecherche);
    int levelReseau = GeneralManager::instance().getResearchLevel(ResearchType::Reseau);
    int globalLabLevel = PlayerManager::instance().getGlobalLabLevel();

    Ressources<float> cumulCost = TechManager::instance().getCost(TechType::CommonResearch, indexReseau, levelReseau + 1);
    float percentGain = 0.0f;
    float currentPercentCost = std::numeric_limits<float>::max();
    int levelLab = 0;

    while(true)
    {
        percentGain += 100.0f * (1.0f - ((float)(globalLabLevel + 1) / (float)(globalLabLevel + 2)));
        globalLabLevel++;
        levelLab++;
        cumulCost += TechManager::instance().getCost(TechType::CommonBuilding, indexLab, levelLab);
        float newPercentCost = cumulCost.getEquivalentDeut(GeneralManager::instance().getConversionRate()) / percentGain;
        if (newPercentCost > currentPercentCost)
        {
            break;
        }
        else
        {
            currentPercentCost = newPercentCost;
        }
    }

    SpeedLevelUp levelUpReseau ("Réseau with lab " + QString::number(levelLab), levelReseau);
    levelUpReseau._percentsGain = percentGain;
    levelUpReseau.cost = cumulCost;
    levelUpReseau.timeToCompleteDay = PlayerManager::instance().getResearchTime(indexReseau, levelReseau);
    addNewLevelUp(std::make_unique<SpeedLevelUp>(levelUpReseau));
}

void RentabilityManager::addSpeedBuildingLifeFormForResearchLifeForm(int indexPlanet)
{
    const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);

    if (!planet) return;

    Species planetSpecies = planet->getSpecies();
    if (planetSpecies == Species::None) return;

    TechType buildingType = speciesToBuildingLifeForm.at(planetSpecies);
    int numberBuildingLifeForm = TechManager::instance().getNumberTechs(buildingType);
    for (int i = 0; i < numberBuildingLifeForm; i++)
    {
        const LifeFormBuilding* tech = dynamic_cast<const LifeFormBuilding*>(TechManager::instance().getTech(buildingType, i));
        float bonusTime = getGainTimePercent(tech);
        if (bonusTime > 0.0f)
        {
            int levelUpBatiment = planet->getTechLevel(buildingType, i) + 1;

            SpeedLevelUp levelUpBuilding(tech->name, levelUpBatiment);
            levelUpBuilding._percentsGain = bonusTime;
            levelUpBuilding.cost = planet->getCost(buildingType, i, levelUpBatiment);
            levelUpBuilding.timeToCompleteDay = planet->getTime(buildingType, i, levelUpBatiment);
            addNewLevelUp(std::make_unique<SpeedLevelUp>(levelUpBuilding), indexPlanet);
        }
    }
}

void RentabilityManager::addSpeedResearchLifeFormForResearch(int indexPlanet, BonusLifeForm bonusType)
{
    const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);

    if (!planet) return;

    int nbPlanet = PlayerManager::instance().getNumberPlanets();
    int numberResearchLifeForm = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberResearchLifeForm; i++)
    {
        Species speciesLifeForm = planet->getChoiceLifeFormResearch(i);
        if (speciesLifeForm == Species::None) continue;

        TechType researchLifeFormType = speciesToTechLifeForm.at(speciesLifeForm);
        const LifeFormTech* tech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(researchLifeFormType, i));

        float bonusTime = getGainTimePercent(tech, bonusType);
        if (bonusTime > 0.0f)
        {
            const LifeFormTech* tech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(researchLifeFormType, i));
            int levelUpResearch = planet->getTechLevel(researchLifeFormType, i) + 1;

            SpeedLevelUp levelUpLifeFormResearch(tech->name, levelUpResearch);
            levelUpLifeFormResearch._percentsGain         = bonusTime * nbPlanet;
            levelUpLifeFormResearch.cost                = planet->getCost(researchLifeFormType, i, levelUpResearch);
            levelUpLifeFormResearch.timeToCompleteDay   = planet->getTime(researchLifeFormType, i, levelUpResearch);
            addNewLevelUp(std::make_unique<SpeedLevelUp>(levelUpLifeFormResearch), indexPlanet);
        }
    }
}

void RentabilityManager::onPlanetRemoved(int indexPlanet)
{
    _filterPlanet.erase(_filterPlanet.begin() + indexPlanet);
}
