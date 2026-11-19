#include "playermanager.h"
#include "generalManager.h"
#include "techManager.h"
#include "discoveryManager.h"

#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

PlayerManager& PlayerManager::instance()
{
    static PlayerManager instance;
    return instance;
}

PlayerManager::PlayerManager()
{
    clear();
    refresh();
}

bool PlayerManager::loadSave(const QString& path)
{
    QString filePath = path + "/save.json";

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Impossible d'ouvrir le fichier :" << filePath;
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Erreur JSON :" << parseError.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "Le JSON racine n'est pas un objet";
        return false;
    }

    QJsonObject root = doc.object();

    readAllPlanetData(root);
    readFleetData(root);
    readDiscoveries(root);

    qDebug() << "Loading done : " << filePath;

    return true;
}

float PlayerManager::getResearchTime(int indexTech, int level) const
{
    float timeDays = TechManager::instance().getBaseTime(TechType::CommonResearch, indexTech, level);
    timeDays /= (float)(1 + _labsLevel);
    timeDays /= (float)GeneralManager::instance().getUniverseSpecific(UniverseSpecifics::ResearchBoost);

    float factorTimeReduction = 1.0f - _lifeFormBonuses.at(BonusLifeForm::ResearchDuration) / 100.0f;
    factorTimeReduction = std::max(0.0f, factorTimeReduction);
    timeDays *= factorTimeReduction;

    if (GeneralManager::instance().getClass() == Class::Explorer)
    {
        timeDays *= 0.75f;
    }
    if (GeneralManager::instance().getOfficerValue(Officers::Technocrat))
    {
        timeDays *= 0.75f;
    }
    return timeDays;
}

int PlayerManager::getShip(MovingUnitType unitType) const
{
    auto it = _fleet.find(unitType);
    return it == _fleet.end() ? 0 : it->second;
}

int PlayerManager::getScrapShip(MovingUnitType unitType) const
{
    auto it = _scrapFleet.find(unitType);
    return it == _scrapFleet.end() ? 0 : it->second;
}

Planet* PlayerManager::getPlanet(int index)
{
    if (index < _planets.size())
    {
        return &_planets[index];
    }
    else
    {
        qCritical() << __FUNCTION__ << " planet not available : " << index << " >= " << _planets.size();
        return nullptr;
    }
}
PlayerManager::Planification* PlayerManager::getPlanif(int index)
{
    if (index < _planifPlanets.size())
    {
        return &_planifPlanets[index];
    }
    else
    {
        qCritical() << __FUNCTION__ << " planif not available : " << index << " >= " << _planifPlanets.size();
        return nullptr;
    }
}

Ressources<float> PlayerManager::getLifeFormProdBonus() const
{
    float metalBonus    = _lifeFormBonuses.at(BonusLifeForm::Metal);
    float cristalBonus  = _lifeFormBonuses.at(BonusLifeForm::Cristal);
    float deutBonus     = _lifeFormBonuses.at(BonusLifeForm::Deuterium);
    return Ressources<float>(metalBonus, cristalBonus, deutBonus);
}

void PlayerManager::refresh()
{
    if (_planetToRemove)
    {
        _planets.erase(_planets.begin() + *_planetToRemove);
        _planetToRemove = std::nullopt;
    }

    if (_planifToRemove)
    {
        _planifPlanets.erase(_planifPlanets.begin() + *_planifToRemove);
        _planifToRemove = std::nullopt;
    }

    computeLifeFormResearch(); //needed for computing planets productions, must be computed first

    int numberPlanet = getNumberPlanets();
    for (int i = 0; i < numberPlanet; ++i)
    {
        Planet* planet = getPlanet(i);
        planet->refresh(getLifeFormProdBonus());
    }

    int numberPlanif = getNumberPlanifs();
    for (int i = 0; i < numberPlanif; ++i)
    {
        Planification* planif = getPlanif(i);
        planif->planet.refresh(getLifeFormProdBonus());
    }

    computeProduction();
    computeLabsLevel();
    computeConversionRate();
    computeScrapValue();
}

void PlayerManager::computeLifeFormResearch()
{
    // Clear
    _lifeFormBonuses.clear();
    for (int i = 0; i < static_cast<int>(BonusLifeForm::Count); i++)
    {
        _lifeFormBonuses[static_cast<BonusLifeForm>(i)];
    }

    // Compute for each planet
    int numberPlanet = getNumberPlanets();
    for (int i = 0; i < numberPlanet; ++i)
    {
        Planet* planet = getPlanet(i);
        planet->computeLifeFormResearch();
    }

    // Compute for each planif
    int numberPlanif = getNumberPlanifs();
    for (int i = 0; i < numberPlanif; ++i)
    {
        Planification* planif = getPlanif(i);
        planif->planet.computeLifeFormResearch();
    }

    // Factorise
    for (int i = 0; i < numberPlanet; ++i)
    {
        const Planet* planet = getPlanet(i);
        for (int j = 0; j < static_cast<int>(BonusLifeForm::Count); ++j)
        {
            BonusLifeForm typeBonus = static_cast<BonusLifeForm>(j);
            _lifeFormBonuses[typeBonus] += planet->getLifeFormBonus(typeBonus);
        }
    }
}

void PlayerManager::computeProduction()
{
    _productionStats.clear();
    int numberPlanet = getNumberPlanets();
    for (int i = 0; i < numberPlanet; ++i)
    {
        const Planet* planet = getPlanet(i);

        _productionStats[ProductionStat::Base]                  += planet->getProductionStat(Planet::ProductionStat::Base);
        _productionStats[ProductionStat::Mines]                 += planet->getProductionStat(Planet::ProductionStat::Mines);
        _productionStats[ProductionStat::BuildingLifeFormTotal] += planet->getProductionStat(Planet::ProductionStat::BuildingLifeForm);
        _productionStats[ProductionStat::CrawlersTotal]         += planet->getProductionStat(Planet::ProductionStat::Crawlers);
        _productionStats[ProductionStat::Plasma]                += planet->getProductionStat(Planet::ProductionStat::Plasma);
        _productionStats[ProductionStat::Geolog]                += planet->getProductionStat(Planet::ProductionStat::Geolog);
        _productionStats[ProductionStat::Council]               += planet->getProductionStat(Planet::ProductionStat::Council);
        _productionStats[ProductionStat::ClassBonus]            += planet->getProductionStat(Planet::ProductionStat::Class);
        _productionStats[ProductionStat::AllianceBonus]         += planet->getProductionStat(Planet::ProductionStat::AllianceClass);
        _productionStats[ProductionStat::LifeFormBonus]         += planet->getProductionStat(Planet::ProductionStat::LifeFormResearch);
        _productionStats[ProductionStat::Total]                 += planet->getProductionStat(Planet::ProductionStat::Total);
    }

    _productionStatsPercent[ProductionStatPercent::PlasmaPercent]         = GeneralManager::instance().getPlasmaBonus();
    _productionStatsPercent[ProductionStatPercent::LifeFormBonusPercent]  = getLifeFormProdBonus();
    _productionStatsPercent[ProductionStatPercent::GeologPercent]         = GeneralManager::instance().getGeologBonus();
    _productionStatsPercent[ProductionStatPercent::ClassBonusPercent]     = GeneralManager::instance().getClassBonus();
    _productionStatsPercent[ProductionStatPercent::AllianceBonusPercent]  = GeneralManager::instance().getAllianceClassBonus();
}

void PlayerManager::computeLabsLevel()
{
    struct PlanetLab
    {
        Planet* planet;
        int level {0};
    };

    std::vector<PlanetLab> labLevels;
    for (int i = 0; i < getNumberPlanets(); ++i)
    {
        Planet* planet = getPlanet(i);
        int labIndex = static_cast<int>(CommonBuildingType::LaboRecherche);
        int labLevel = planet->getTechLevel(TechType::CommonBuilding, labIndex);

        PlanetLab planetLab{
            .planet = planet,
            .level = labLevel
        };
        labLevels.push_back(planetLab);
    }
    std::sort(labLevels.begin(), labLevels.end(), [](const PlanetLab& a, const PlanetLab& b)
    {
        return a.level > b.level;
    });

    _labsLevel = 0;
    int levelNetwork = GeneralManager::instance().getResearchLevel(ResearchType::Reseau);
    for (int i = 0; i < labLevels.size(); ++i)
    {
        if (i <= levelNetwork)
        {
            labLevels.at(i).planet->setWorkingLab(true);
            _labsLevel += labLevels.at(i).level;
        }
        else
        {
            labLevels.at(i).planet->setWorkingLab(false);
        }
    }
}

void PlayerManager::computeConversionRate()
{
    int costFullMetal = 42000;
    float benefMetal = getProduction(PlayerManager::ProductionStat::Total).metal;
    float amToMetal = (float)costFullMetal / benefMetal;
    Ressources<float> conversionRate = GeneralManager::instance().getConversionRate();

    conversionRate.antimatter = amToMetal / conversionRate.deut * conversionRate.metal;

    GeneralManager::instance().setConversionRate(conversionRate);
}

void PlayerManager::computeScrapValue()
{
    _scrapValue = Ressources<float>();
    for (const auto& [unitType, numberShip] : _scrapFleet)
    {
        Unit unit = TechManager::instance().getMovingUnit(unitType);
        _scrapValue += numberShip * unit.cost;
    }
    _scrapValue *= GeneralManager::instance().getScrapRate() / 100.0f;
}

bool PlayerManager::clear()
{
    _planets.clear();
    _planifPlanets.clear();
    _lifeFormBonuses.clear();
    _fleet.clear();
    _scrapFleet.clear();
    _discoveryPerDay = 0.0f;
    _deutConsumption = 0;
    _positionDiscovery = 1;
    _additionalBonusRessources = 0;

    addPlanet("Planète mère", {1, 1, 1}, 0, Species::None);

    return true;
}


void PlayerManager::addPlanet(const QString& name, const PlanetPosition& position, int temperature, Species species)
{
    Planet* planet = new Planet(name, position, temperature, species);
    _planets.push_back(planet);
}

void PlayerManager::duplicatePlanet()
{
    Planet* planet = new Planet(_planets.back());
    _planets.push_back(planet);
}

void PlayerManager:: readAllPlanetData(const QJsonObject& parent)
{
    _planets.clear();
    QJsonArray planets = parent["planets"].toArray();
    for (const QJsonValue& value : planets) {
        QJsonObject planetObj = value.toObject();
        Planet planet = readPlanetData(planetObj);
        _planets.push_back(std::move(planet));
    }

    _planifPlanets.clear();
    QJsonArray planifs = parent["planification"].toArray();
    for (const QJsonValue& value : planifs) {
        PlanificationType type = static_cast<PlanificationType>(value["type"].toInt());
        QJsonObject planetObj = value["planet"].toObject();
        Planet planet = readPlanetData(planetObj);
        _planifPlanets.emplace_back(type, std::move(planet));
    }
}

void PlayerManager::readFleetData(const QJsonObject& parent)
{
    QJsonArray fleetArray = parent["fleet"].toArray();
    int index = 0;
    for (const QJsonValue& val : fleetArray) {
        setShip(static_cast<MovingUnitType>(index), val.toInt(0));
        index++;
    }

    QJsonArray scrapFleetArray = parent["scrapFleet"].toArray();
    index = 0;
    for (const QJsonValue& val : scrapFleetArray) {
        setScrapShip(static_cast<MovingUnitType>(index), val.toInt(0));
        index++;
    }
}

Planet PlayerManager::readPlanetData(const QJsonObject& planetObj)
{
    QString name = planetObj["name"].toString();
    int species = planetObj["species"].toInt();
    int temperatureMax = planetObj["temperatureMax"].toInt();

    std::array<int, 3> filePosition;
    QJsonArray positionArray = planetObj["position"].toArray();
    int indexPos = 0;
    for (const QJsonValue& b : positionArray) {
        filePosition[indexPos++] = b.toInt();
    }

    PlanetPosition planetPosition(filePosition[0], filePosition[1], filePosition[2]);
    Planet planet(name, planetPosition, temperatureMax, static_cast<Species>(species));

    for (int i = 1; i < static_cast<int>(TechType::Count); ++i)
    {
        TechType techType = static_cast<TechType>(i);
        if (techType == TechType::CommonResearch) continue;

        QJsonArray techArray = planetObj[techTypeToString[i]].toArray();
        int index = 0;
        for (const QJsonValue& b : techArray) {
            planet.setTechLevel(techType, index, b.toInt(0));
            index++;
        }
    }

    QJsonArray choiceLifeFormResearchArray = planetObj["choiceLifeFormResearch"].toArray();
    int index = 0;
    for (const QJsonValue& b : choiceLifeFormResearchArray) {
        planet.setChoiceLifeFormResearch(index, static_cast<Species>(b.toInt(0)));
        index++;
    }

    QJsonArray defensesArray = planetObj["defenses"].toArray();
    index = 0;
    for (const QJsonValue& val : defensesArray) {
        planet.setDefense(static_cast<FixUnitType>(index), val.toInt(0));
        index++;
    }

    return planet;
}

void PlayerManager::readDiscoveries(const QJsonObject& parent)
{
    _discoveryPerDay = (float)parent["discoveryPerDay"].toDouble();
    _deutConsumption = parent["deutConsumption"].toInt();
    _additionalBonusRessources = parent["tempBonus"].toInt();
    _positionDiscovery = parent["positionDiscovery"].toInt();
}

QJsonDocument PlayerManager::generateGameDataJson()
{
    QJsonObject root;

    writeAllPlanetData(root);
    writeFleetData(root);
    writeDiscoveries(root);

    return QJsonDocument(root);
}

void PlayerManager::writeAllPlanetData(QJsonObject& parent)
{
    // Planets array
    QJsonArray planetsArray;
    for (int i = 0; i < _planets.size(); ++i)
    {
        const Planet& planet = _planets.at(i);
        QJsonObject planetObj = writePlanetData(planet);
        planetsArray.append(planetObj);
    }
    parent["planets"] = planetsArray;

    // Planets planification life form update array
    QJsonArray planifArray;
    for (int i = 0; i < _planifPlanets.size(); ++i)
    {
        QJsonObject planifObj;
        const Planification& planif = _planifPlanets.at(i);
        QJsonObject planetObj = writePlanetData(planif.planet);
        planifObj["type"] = static_cast<int>(planif.type);
        planifObj["planet"] = planetObj;
        planifArray.append(planifObj);
    }
    parent["planification"] = planifArray;
}

void PlayerManager::writeFleetData(QJsonObject& parent)
{
    QJsonArray fleet;
    for (int i = 0; i < static_cast<int>(MovingUnitType::Count); ++i)
    {
        MovingUnitType unitType = static_cast<MovingUnitType>(i);
        fleet << getShip(unitType);
    }
    parent["fleet"] = fleet;

    QJsonArray scrapFleet;
    for (int i = 0; i < static_cast<int>(MovingUnitType::Count); ++i)
    {
        MovingUnitType unitType = static_cast<MovingUnitType>(i);
        scrapFleet << getScrapShip(unitType);
    }
    parent["scrapFleet"] = scrapFleet;
}

QJsonObject PlayerManager::writePlanetData(const Planet& planet)
{
    QJsonObject planetObj;

    planetObj["name"] = planet.getName();

    PlanetPosition planetPosition = planet.getPosition();
    QJsonArray positionArray;
    positionArray << planetPosition.galaxy << planetPosition.solarSystem << planetPosition.position;
    planetObj["position"] = positionArray;

    planetObj["species"] = static_cast<int>(planet.getSpecies());
    planetObj["temperatureMax"] = static_cast<int>(planet.getTemperatureMax());

    for (int i = 1; i < static_cast<int>(TechType::Count); ++i)
    {
        TechType techType = static_cast<TechType>(i);
        if (techType == TechType::CommonResearch) continue;

        QJsonArray techArray;

        for (int j = 0; j < planet.getNumberTech(techType); ++j)
        {
            techArray << planet.getTechLevel(techType, j);
        }

        planetObj[techTypeToString[i]] = techArray;
    }

    // Choice life form
    QJsonArray choiceLifeFormResearchArray;
    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberLifeFormReseach; ++i)
    {
        Species choice = planet.getChoiceLifeFormResearch(i);
        choiceLifeFormResearchArray << static_cast<int>(choice);
    }
    planetObj["choiceLifeFormResearch"] = choiceLifeFormResearchArray;

    // Defenses
    QJsonArray defenses;
    for (int i = 0; i < static_cast<int>(FixUnitType::Count); ++i)
    {
        FixUnitType unitType = static_cast<FixUnitType>(i);
        defenses << planet.getDefense(unitType);
    }
    planetObj["defenses"] = defenses;

    return std::move(planetObj);
}

void PlayerManager::writeDiscoveries(QJsonObject& parent)
{
    parent["discoveryPerDay"] = _discoveryPerDay;
    parent["deutConsumption"] = _deutConsumption;
    parent["tempBonus"] = _additionalBonusRessources;
    parent["positionDiscovery"] = _positionDiscovery;
}

bool PlayerManager::saveGameData(const QString& path)
{
    QJsonDocument doc = generateGameDataJson();

    QString playerSavePath = path + "/save.json";
    QFile file(playerSavePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "Can't open " << playerSavePath << " error : " << file.errorString();
        return false;
    }

    qDebug() << "Saved to " << playerSavePath;

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}
