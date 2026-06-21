#include "playermanager.h"
#include "techManager.h"

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

    computeLifeFormResearch();
    computeProduction();
}

int PlayerManager::getNumberPlanets() const
{
    return _planets.size();
}

int PlayerManager::getNumberResearch() const
{
    return _levelResearch.size();
}

const QString& PlayerManager::getPlanetName(int index) const
{
    return _planets.at(index)->getName();
}

Class PlayerManager::getClass() const
{
    return _class;
}

AllianceClass PlayerManager::getAllianceClass() const
{
    return _allianceClass;
}

int PlayerManager::getScrapRate() const
{
    return _scrapRate;
}

int PlayerManager::getUniverseSpecific(UniverseSpecifics universeSpecific) const
{
    return _universeSpecifics.at(universeSpecific);
}

bool PlayerManager::getOfficerValue(Officers officer) const
{
    return _officers.at(officer);
}

Ressources PlayerManager::getConversionRate() const
{
    return _conversionRates;
}

int PlayerManager::getResearchLevel(ResearchType researchType) const
{
    return _levelResearch.at(researchType);
}

int PlayerManager::getSpecies(Species species) const
{
    return _levelSpecies.at(species);
}

Planet* PlayerManager::getPlanet(int index) const
{
    return _planets.at(index);
}

Ressources PlayerManager::getPlasmaBonus() const
{
    float levelPlasma = (float)getResearchLevel(ResearchType::Plasma);
    return Ressources(levelPlasma, 0.66f * levelPlasma, 0.33f * levelPlasma);
}

float PlayerManager::getLifeFormBonus(BonusLifeForm bonus) const
{
    return _lifeFormBonuses.at(bonus);
}

Ressources PlayerManager::getLifeFormProdBonus() const
{
    float metalBonus    = _lifeFormBonuses.at(BonusLifeForm::Metal);
    float cristalBonus  = _lifeFormBonuses.at(BonusLifeForm::Cristal);
    float deutBonus     = _lifeFormBonuses.at(BonusLifeForm::Deuterium);
    return Ressources(metalBonus, cristalBonus, deutBonus);
}

Ressources PlayerManager::getGeologBonus() const
{
    if (_officers.at(Officers::Concil))
    {
        return Ressources(12.0f, 12.0f, 12.0f);
    }
    else if (_officers.at(Officers::Geolog))
    {
        return Ressources(10.0f, 10.0f, 10.0f);
    }
    else
    {
        return Ressources(0.0f, 0.0f, 0.0f);
    }
}
Ressources PlayerManager::getClassBonus() const
{
    if (_class == Class::Collector)
    {
        return Ressources(25.0f, 25.0f, 25.0f);
    }
    else
    {
        return Ressources(0.0f, 0.0f, 0.0f);
    }
}
Ressources PlayerManager::getAllianceClassBonus() const
{
    if (_allianceClass == AllianceClass::Merchand)
    {
        return Ressources(5.0f, 5.0f, 5.0f);
    }
    else
    {
        return Ressources(0.0f, 0.0f, 0.0f);
    }
}

void PlayerManager::computeLifeFormResearch()
{
    _lifeFormBonuses.clear();
    for (int i = 0; i < static_cast<int>(BonusLifeForm::Count); i++)
    {
        _lifeFormBonuses[static_cast<BonusLifeForm>(i)];
    }

    int numberPlanet = getNumberPlanets();
    for (int i = 0; i < numberPlanet; ++i)
    {
        const Planet* planet = getPlanet(i);

        int lifeFormBuildingNumber = TechManager::instance().getNumberTechs(TechType::HumanResearch);
        for (int j = 0; j < lifeFormBuildingNumber; j++)
        {
            Species choice = planet->getChoiceLifeFormResearch(j);
            if (choice != Species::None)
            {
                TechType lifeFormReasearch = speciesToTechLifeForm.at(choice);
                int level = planet->getTechLevel(lifeFormReasearch, j);

                const LifeFormTech* lifeFormTech = dynamic_cast<const LifeFormTech*>(TechManager::instance().getTech(lifeFormReasearch, j));
                for (auto itr = lifeFormTech->bonuses.begin(); itr != lifeFormTech->bonuses.end(); ++itr)
                {
                    float bonusLevelSpecies = 1.0f + (float)_levelSpecies[lifeFormTech->species] * 0.001f;
                    float rawBonus = level * itr->second * bonusLevelSpecies;
                    float bonus = std::round(rawBonus * 100.0f) / 100.0f;
                    _lifeFormBonuses[itr->first] += bonus;
                }
            }
        }
    }
}

void PlayerManager::computeProduction()
{
    int numberPlanet = getNumberPlanets();
    Ressources base, prodMines, prodCrawler, prodBuildingLifeForm;
    for (int i = 0; i < numberPlanet; ++i)
    {
        const Planet* planet = getPlanet(i);

        base += planet->getProductionStat(Planet::ProductionStatPlanet::Base);
        prodMines += planet->getProductionStat(Planet::ProductionStatPlanet::Mines);
        prodBuildingLifeForm += planet->getLifeFormBuildingProduction();
        prodCrawler += planet->getCrawlerProduction();
    }

    _productionStats[Base]                  = base;
    _productionStats[Mines]                 = prodMines;
    _productionStats[BuildingLifeFormTotal] = prodBuildingLifeForm;
    _productionStats[CrawlersTotal]         = prodCrawler;
    _productionStats[PlasmaPercent]         = getPlasmaBonus();
    _productionStats[LifeFormBonusPercent]  = getLifeFormProdBonus();
    _productionStats[GeologPercent]         = getGeologBonus();
    _productionStats[ClassBonusPercent]     = getClassBonus();
    _productionStats[AllianceBonusPercent]  = getAllianceClassBonus();
    _productionStats[Plasma]                = prodMines * getPlasmaBonus() * 0.01f;
    _productionStats[LifeFormBonus]         = prodMines * getLifeFormProdBonus() * 0.01f;
    _productionStats[Geolog]                = prodMines * getGeologBonus() * 0.01f;
    _productionStats[ClassBonus]            = prodMines * getClassBonus() * 0.01f;
    _productionStats[AllianceBonus]         = prodMines * getAllianceClassBonus() * 0.01f;
}

const Ressources& PlayerManager::getProduction(ProductionStat stat) const
{
    return _productionStats.at(stat);
}

const QString& PlayerManager::getProductionStr(ProductionStat stat) const
{
    return _prods[static_cast<int>(stat)];
}

void PlayerManager::setClass(Class globalClass)
{
    _class = globalClass;
}

void PlayerManager::setAllianceClass(AllianceClass allianceClass)
{
    _allianceClass = allianceClass;
}

void PlayerManager::setScrapRate(int scrapRate)
{
    _scrapRate = scrapRate;
}

void PlayerManager::setUniverseSpecific(UniverseSpecifics universeSpecific, int speed)
{
    _universeSpecifics[universeSpecific] = speed;
}

void PlayerManager::setOfficerActivated(Officers officer, bool activated)
{
    _officers[officer] = activated;
}

void PlayerManager::setConversionRate(Ressources conversionRate)
{
    _conversionRates = conversionRate;
}

void PlayerManager::setConversionRateAt(RessourceType type, float conversionRate)
{
    _conversionRates.setRessource(type, conversionRate);
}

void PlayerManager::setResearchLevel(ResearchType researchType, int level)
{
    _levelResearch[researchType] = level;
}

void PlayerManager::setSpecies(Species species, int level)
{
    _levelSpecies[species] = level;
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
    readPlanetData(root);

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
    Ressources conversionRate;
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

void PlayerManager:: readPlanetData(const QJsonObject& parent)
{
    QJsonArray planets = parent["planets"].toArray();

    int indexPlanet = -1;

    for (const QJsonValue& value : planets) {
        indexPlanet++;
        QJsonObject planetObj = value.toObject();

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
        Planet* planet = new Planet(name, planetPosition, temperatureMax, static_cast<Species>(species));

        for (int i = 1; i < static_cast<int>(TechType::Count); ++i)
        {
            TechType techType = static_cast<TechType>(i);
            if (techType == TechType::CommonResearch) continue;

            QJsonArray techArray = planetObj[techTypeToString[i]].toArray();
            int index = 0;
            for (const QJsonValue& b : techArray) {
                planet->setTechLevel(techType, index, b.toInt());
                index++;
            }
        }

        QJsonArray choiceLifeFormResearchArray = planetObj["choiceLifeFormResearch"].toArray();
        int index = 0;
        for (const QJsonValue& b : choiceLifeFormResearchArray) {
            planet->setChoiceLifeFormResearch(index, static_cast<Species>(b.toInt()));
            index++;
        }

        planet->setCrawlerNumber(planetObj["crawlers"].toInt(0));
        _planets.push_back(planet);
    }
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
        writePlanetData(planetsArray, i);
    }

    root["planets"] = planetsArray;

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

void PlayerManager::writePlanetData(QJsonArray &parent, int indexPlanet)
{
    Planet* planet = _planets.at(indexPlanet);

    QJsonObject planetObj;
    planetObj["name"] = planet->getName();

    PlanetPosition planetPosition = planet->getPosition();
    QJsonArray positionArray;
    positionArray << planetPosition.galaxy << planetPosition.solarSystem << planetPosition.position;
    planetObj["position"] = positionArray;

    planetObj["species"] = static_cast<int>(planet->getSpecies());
    planetObj["temperatureMax"] = static_cast<int>(planet->getTemperatureMax());

    for (int i = 1; i < static_cast<int>(TechType::Count); ++i)
    {
        TechType techType = static_cast<TechType>(i);
        if (techType == TechType::CommonResearch) continue;

        QJsonArray techArray;

        for (int j = 0; j < planet->getNumberTech(techType); ++j)
        {
            techArray << planet->getTechLevel(techType, j);
        }

        planetObj[techTypeToString[i]] = techArray;
    }

    QJsonArray choiceLifeFormResearchArray;
    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberLifeFormReseach; ++i)
    {
        Species choice = planet->getChoiceLifeFormResearch(i);
        choiceLifeFormResearchArray << static_cast<int>(choice);
    }
    planetObj["choiceLifeFormResearch"] = choiceLifeFormResearchArray;

    planetObj["crawlers"] = planet->getCrawlerNumber();

    planetObj["defenses"] = QJsonArray{};

    parent.append(planetObj);
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
