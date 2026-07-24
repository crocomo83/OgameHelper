#include "playermanager.h"
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
    QString path = getSavePath();
    if (QFileInfo::exists(path)) {
        loadSave(path);
    } else {
        loadInitSave();
    }

    refresh();
}

float PlayerManager::getResearchTime(int indexTech, int level) const
{
    float timeDays = TechManager::instance().getBaseTime(TechType::CommonResearch, indexTech, level);
    timeDays /= (float)(1 + _labsLevel);
    timeDays /= (float)_universeSpecifics.at(UniverseSpecifics::ResearchBoost);

    if (_class == Class::Explorer)
    {
        timeDays *= 0.75f;
    }
    if (_officers.at(Officers::Technocrat))
    {
        timeDays *= 0.75f;
    }
    return timeDays;
}

Planet& PlayerManager::getPlanifPlanet(const Species& species)
{
    auto it = _planificationFDV.find(species);
    if (it != _planificationFDV.end())
    {
        return it->second;
    }
    else
    {
        Planet planet;
        planet.setSpecies(species);
        _planificationFDV.emplace(species, std::move(planet));
        return _planificationFDV.at(species);
    }
}

Ressources<float> PlayerManager::getPlasmaBonus() const
{
    float levelPlasma = (float)getResearchLevel(ResearchType::Plasma);
    return Ressources<float>(levelPlasma, 0.66f * levelPlasma, 0.33f * levelPlasma);
}

Ressources<float> PlayerManager::getLifeFormProdBonus() const
{
    float metalBonus    = _lifeFormBonuses.at(BonusLifeForm::Metal);
    float cristalBonus  = _lifeFormBonuses.at(BonusLifeForm::Cristal);
    float deutBonus     = _lifeFormBonuses.at(BonusLifeForm::Deuterium);
    return Ressources<float>(metalBonus, cristalBonus, deutBonus);
}

Ressources<float> PlayerManager::getGeologBonus() const
{
    if (_officers.at(Officers::Concil))
    {
        return Ressources<float>(12.0f, 12.0f, 12.0f);
    }
    else if (_officers.at(Officers::Geolog))
    {
        return Ressources<float>(10.0f, 10.0f, 10.0f);
    }
    else
    {
        return Ressources<float>();
    }
}
Ressources<float> PlayerManager::getClassBonus() const
{
    if (_class == Class::Collector)
    {
        return Ressources<float>(25.0f, 25.0f, 25.0f);
    }
    else
    {
        return Ressources<float>();
    }
}
Ressources<float> PlayerManager::getAllianceClassBonus() const
{
    if (_allianceClass == AllianceClass::Merchand)
    {
        return Ressources<float>(5.0f, 5.0f, 5.0f);
    }
    else
    {
        return Ressources<float>();
    }
}

void PlayerManager::refresh()
{
    computeLifeFormResearch();

    int numberPlanet = getNumberPlanets();
    for (int i = 0; i < numberPlanet; ++i)
    {
        Planet& planet = getPlanet(i);
        planet.refresh();
    }

    _planifAstro.refresh();
    _planifChangeSpecies.refresh();

    for (auto& [key, planet] : _planificationFDV)
    {
        planet.refresh();
    }

    computeProduction();
    computeLabsLevel();
    computeConversionRate();
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
        Planet& planet = getPlanet(i);
        planet.computeLifeFormResearch(_levelSpecies);
    }

    // Factorise
    for (int i = 0; i < numberPlanet; ++i)
    {
        const Planet& planet = getPlanet(i);
        for (int j = 0; j < static_cast<int>(BonusLifeForm::Count); ++j)
        {
            BonusLifeForm typeBonus = static_cast<BonusLifeForm>(j);
            _lifeFormBonuses[typeBonus] += planet.getLifeFormBonus(typeBonus);
        }
    }
}

void PlayerManager::computeProduction()
{
    int numberPlanet = getNumberPlanets();
    Ressources<int> base, prodMines, prodCrawler, prodBuildingLifeForm;
    for (int i = 0; i < numberPlanet; ++i)
    {
        const Planet& planet = getPlanet(i);

        base                    += planet.getProductionStat(Planet::ProductionStat::Base);
        prodMines               += planet.getProductionStat(Planet::ProductionStat::Mines);
        prodBuildingLifeForm    += planet.getProductionStat(Planet::ProductionStat::BuildingLifeForm);
        prodCrawler             += planet.getProductionStat(Planet::ProductionStat::Crawlers);
    }

    _productionStats[ProductionStat::Base]                  = base;
    _productionStats[ProductionStat::Mines]                 = prodMines;
    _productionStats[ProductionStat::BuildingLifeFormTotal] = prodBuildingLifeForm;
    _productionStats[ProductionStat::CrawlersTotal]         = prodCrawler;
    _productionStats[ProductionStat::Plasma]                = Ressources<float>(prodMines) * getPlasmaBonus() * 0.01f;
    _productionStats[ProductionStat::LifeFormBonus]         = Ressources<float>(prodMines) * getLifeFormProdBonus() * 0.01f;
    _productionStats[ProductionStat::Geolog]                = Ressources<float>(prodMines) * getGeologBonus() * 0.01f;
    _productionStats[ProductionStat::ClassBonus]            = Ressources<float>(prodMines) * getClassBonus() * 0.01f;
    _productionStats[ProductionStat::AllianceBonus]         = Ressources<float>(prodMines) * getAllianceClassBonus() * 0.01f;

    _productionStats[ProductionStat::Total] = Ressources<float>();
    for (int i = 0; i < static_cast<int>(ProductionStat::Total); i++)
    {
        ProductionStat stat = static_cast<ProductionStat>(i);
        _productionStats[ProductionStat::Total] +=_productionStats.at(stat);
    }

    _productionStatsPercent[ProductionStatPercent::PlasmaPercent]         = getPlasmaBonus();
    _productionStatsPercent[ProductionStatPercent::LifeFormBonusPercent]  = getLifeFormProdBonus();
    _productionStatsPercent[ProductionStatPercent::GeologPercent]         = getGeologBonus();
    _productionStatsPercent[ProductionStatPercent::ClassBonusPercent]     = getClassBonus();
    _productionStatsPercent[ProductionStatPercent::AllianceBonusPercent]  = getAllianceClassBonus();
}

void PlayerManager::computeLabsLevel()
{
    std::vector<int> labLevels;
    for (int i = 0; i < getNumberPlanets(); ++i)
    {
        const Planet& planet = getPlanet(i);
        int labIndex = static_cast<int>(CommonBuildingType::LaboRecherche);
        labLevels.push_back(planet.getTechLevel(TechType::CommonBuilding, labIndex));
    }
    std::sort(labLevels.begin(), labLevels.end(), std::greater<int>());

    _labsLevel = 0;
    int levelNetwork = getResearchLevel(ResearchType::Reseau);
    int maxIndex = std::min(levelNetwork + 1, (int)labLevels.size());
    for (int i = 0; i < maxIndex; ++i)
    {
        _labsLevel += labLevels.at(i);
    }
}

void PlayerManager::computeConversionRate()
{
    int costFullMetal = 42000;
    float benefMetal = getProduction(PlayerManager::ProductionStat::Total).metal;
    float amToMetal = (float)costFullMetal / benefMetal;
    _conversionRates.antimatter = amToMetal / _conversionRates.deut * _conversionRates.metal;
}

bool PlayerManager::loadInitSave()
{
    for (int i = 0; i < static_cast<int>(UniverseSpecifics::Count); ++i) {
        _universeSpecifics[static_cast<UniverseSpecifics>(i)] = 1;
    }

    for (int i = 1; i < static_cast<int>(Species::Count); ++i) {
        _levelSpecies[static_cast<Species>(i)] = 0;
    }

    for (int i = 0; i < static_cast<int>(ResearchType::Count); ++i) {
        _levelResearch[static_cast<ResearchType>(i)] = 0;
    }

    for (int i = 0; i < static_cast<int>(Officers::Count); ++i) {
        _officers[static_cast<Officers>(i)] = false;
    }

    _class = Class::None;
    _allianceClass = AllianceClass::None;
    _scrapRate = 35;

    setConversionRate(Ressources(3.0f, 2.0f, 1.0f));

    addPlanet("Planète mère", {1, 1, 1}, 0, Species::None);

    return true;
}

bool PlayerManager::loadSave(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Impossible d'ouvrir le fichier :" << path;
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

    readUniverses(root);
    readSpecies(root);
    readResearches(root);
    readClassData(root);
    readOfficersData(root);
    readConversionData(root);
    readAllPlanetData(root);

    qDebug() << "Loading done : " << path;

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

void PlayerManager::readUniverses(const QJsonObject &parent)
{
    QJsonArray universes = parent["universe"].toArray();

    int indexUniverse = 0;
    for (const QJsonValue& value : universes) {
        QJsonObject universe = value.toObject();
        int level = universe["level"].toInt();

        _universeSpecifics[static_cast<UniverseSpecifics>(indexUniverse)] = level;
        indexUniverse++;
    }
}

void PlayerManager::readSpecies(const QJsonObject& parent)
{
    QJsonArray species = parent["species"].toArray();

    int indexSpecies = 1;
    for (const QJsonValue& value : species) {
        QJsonObject race = value.toObject();
        int level = race["level"].toInt();

        _levelSpecies[static_cast<Species>(indexSpecies)] = level;
        indexSpecies++;
    }
}

void PlayerManager::readResearches(const QJsonObject& parent)
{
    QJsonArray researches = parent["researches"].toArray();

    int indexResearch = 0;
    for (const QJsonValue& value : researches) {
        QJsonObject research = value.toObject();
        int level = research["level"].toInt();

        _levelResearch[static_cast<ResearchType>(indexResearch)] = level;
        indexResearch++;
    }
}

void PlayerManager:: readClassData(const QJsonObject& parent)
{
    QJsonArray classDatas = parent["classData"].toArray();

    QJsonObject classData = classDatas[0].toObject();
    QJsonObject allianceData = classDatas[1].toObject();

    int indexClass = classData["index"].toInt();
    int indexAlliance = allianceData["index"].toInt();

    _class = static_cast<Class>(indexClass);
    _allianceClass = static_cast<AllianceClass>(indexAlliance);
}

void PlayerManager:: readOfficersData(const QJsonObject& parent)
{
    QJsonArray officers = parent["officers"].toArray();

    int indexOfficers = 0;
    for (const QJsonValue& value : officers) {
        QJsonObject research = value.toObject();
        bool activated = research["active"].toBool();

        _officers[static_cast<Officers>(indexOfficers)] = activated;
        indexOfficers++;
    }
}

void PlayerManager::readConversionData(const QJsonObject& parent)
{
    QJsonArray merchandsRate = parent["merchandsRate"].toArray();

    int index = 0;
    Ressources<float> conversionRate;
    for (const QJsonValue& value : merchandsRate) {
        QJsonObject rate = value.toObject();
        double level = rate["level"].toDouble();
        QString name = rate["label"].toString();

        if (name == "Métal")
        {
            conversionRate.metal = level;
        }
        else if (name == "Cristal")
        {
            conversionRate.cristal = level;
        }
        else if (name == "Deut")
        {
            conversionRate.deut = level;
        }
        else
        {
            qWarning() << "'" << name << "' is not a valid ressource name";
        }

        index++;
    }

    setConversionRate(conversionRate);

    QJsonArray scrapDatas = parent["scrapRate"].toArray();

    QJsonObject scrapData = scrapDatas[0].toObject();

    int level = scrapData["level"].toInt();
    _scrapRate = level;
}

void PlayerManager:: readAllPlanetData(const QJsonObject& parent)
{
    QJsonArray planets = parent["planets"].toArray();

    for (const QJsonValue& value : planets) {
        QJsonObject planetObj = value.toObject();
        Planet planet = readPlanetData(planetObj);
        _planets.push_back(std::move(planet));
    }

    QJsonArray planetsPlanif = parent["planetPlanification"].toArray();
    for (const QJsonValue& value : planetsPlanif) {
        QJsonObject planetObj = value.toObject();
        Planet planet = readPlanetData(planetObj);
        Species species = planet.getSpecies();
        _planificationFDV[species] = std::move(planet);
    }

    _planifChangeSpecies = readPlanetData(parent["planetPlanificationChangeSpecies"].toObject());
    _planifAstro = readPlanetData(parent["planetPlanifAstro"].toObject());
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

QJsonDocument PlayerManager::generateGameDataJson()
{
    QJsonObject root;

    writeUniverses(root);
    writeSpecies(root);
    writeResearches(root);
    writeClassData(root);
    writeOfficersData(root);
    writeConversionData(root);

    // Planets array
    QJsonArray planetsArray;
    for (int i = 0; i < _planets.size(); ++i)
    {
        const Planet& planet = _planets.at(i);
        QJsonObject planetObj = writePlanetData(planet);
        planetsArray.append(planetObj);
    }
    root["planets"] = planetsArray;

    // Planets planification life form update array
    root["planetPlanifAstro"] = writePlanetData(_planifAstro);
    root["planetPlanificationChangeSpecies"] = writePlanetData(_planifChangeSpecies);

    QJsonArray planetsPlanifArray;
    for (const auto& [key, planet] : _planificationFDV)
    {
        QJsonObject planetObj = writePlanetData(planet);
        planetsPlanifArray.append(planetObj);
    }
    root["planetPlanification"] = planetsPlanifArray;

    return QJsonDocument(root);
}

void PlayerManager::writeUniverses(QJsonObject& parent)
{
    QJsonArray universeStats;

    int index = 0;
    for (auto itr = _universeSpecifics.begin(); itr != _universeSpecifics.end(); ++itr) {
        QJsonObject obj;
        obj["label"] = universeSpecificsToString[index];
        obj["level"] = itr->second;
        universeStats.append(obj);
        index++;
    }

    parent["universe"] = universeStats;
}

void PlayerManager::writeSpecies(QJsonObject& parent)
{
    QJsonArray speciesLevel;

    int index = 0;
    for (auto itr = _levelSpecies.begin(); itr != _levelSpecies.end(); ++itr) {
        QJsonObject obj;
        obj["label"] = speciesToString[index];
        obj["level"] = itr->second;
        speciesLevel.append(obj);
        index++;
    }

    parent["species"] = speciesLevel;
}

void PlayerManager::writeResearches(QJsonObject& parent)
{
    QJsonArray researchesArray;

    for (auto itr = _levelResearch.begin(); itr != _levelResearch.end(); ++itr) {
        const CommonTech* tech = TechManager::instance().getTech(TechType::CommonResearch, static_cast<int>(itr->first));
        QJsonObject obj;
        obj["label"] = tech->name;
        obj["level"] = itr->second;
        researchesArray.append(obj);
    }

    parent["researches"] = researchesArray;
}

void PlayerManager::writeClassData(QJsonObject& parent)
{
    QJsonArray classData;

    QJsonObject classe;
    classe["label"] = "Classe";
    classe["index"] = static_cast<int>(_class);
    classData.append(classe);

    QJsonObject allianceClass;
    allianceClass["label"] = "Classe alli";
    allianceClass["index"] = static_cast<int>(_allianceClass);
    classData.append(allianceClass);

    parent["classData"] = classData;
}

void PlayerManager::writeOfficersData(QJsonObject& parent)
{
    QJsonArray officerArray;

    int index = 0;
    for (auto itr = _officers.begin(); itr != _officers.end(); ++itr) {
        QJsonObject obj;
        obj["label"] = officerToString[index];
        obj["active"] = itr->second;
        officerArray.append(obj);
        index++;
    }

    parent["officers"] = officerArray;
}

void PlayerManager::writeConversionData(QJsonObject& parent)
{
    QJsonArray conversionArray;

    for (int index = 0; index < 3; ++index)
    {
        QJsonObject obj;
        obj["label"] = ressourceToString[index];
        obj["level"] = _conversionRates.getRessource(static_cast<RessourceType>(index));
        conversionArray.append(obj);
    }
    parent["merchandsRate"] = conversionArray;

    QJsonArray scrapParent;
    QJsonObject scrapRate;
    scrapRate["label"] = "scrapRate";
    scrapRate["level"] = _scrapRate;
    scrapParent.append(scrapRate);

    parent["scrapRate"] = scrapParent;
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

QString PlayerManager::getSavePath()
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(basePath);
    return basePath + "/save.json";
}

bool PlayerManager::saveGameData()
{
    QJsonDocument doc = generateGameDataJson();

    QString path = getSavePath();
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "Can't open " << path << " error : " << file.errorString();
        return false;
    }

    qDebug() << "Saved to " << path;

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}
