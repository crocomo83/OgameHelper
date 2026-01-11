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
        loadSave(":/config/save.json");
    }
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

double PlayerManager::getConversionRate(ConversionRate conversionRate) const
{
    return _conversionRates.at(conversionRate);
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

int PlayerManager::getTechLevel(int indexPlanet, TechType type, int indexTech) const
{
    // Check planet index
    if (indexPlanet < 0 || indexPlanet >= static_cast<int>(_planets.size()))
    {
        qWarning() << "index planet invalid : " << indexPlanet << " / " << static_cast<int>(_planets.size());
        return -1;
    }

    return _planets[indexPlanet]->getTechLevel(type, indexTech);
}

Ressources PlayerManager::getPlasmaBonus() const
{
    float levelPlasma = (float)getResearchLevel(ResearchType::Plasma);
    return Ressources(levelPlasma / 100.0f, 0.66f * levelPlasma / 100.0f, 0.33f * levelPlasma / 100.0f);
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

void PlayerManager::setConversionRate(ConversionRate conversionRate, double rate)
{
    _conversionRates[conversionRate] = rate;
}

void PlayerManager::setResearchLevel(ResearchType researchType, int level)
{
    _levelResearch[researchType] = level;
}

void PlayerManager::setSpecies(Species species, int level)
{
    _levelSpecies[species] = level;
}

void PlayerManager::setTechLevel(int indexPlanet, TechType type, int indexTech, int level)
{
    _planets[indexPlanet]->setTechLevel(type, indexTech, level);
}

bool PlayerManager::loadSave(const QString& path)
{
    qDebug() << "Loading " << path;
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

    return true;
}

void PlayerManager::addPlanet(QString name, std::array<int, 3> position, int temperature, Species species)
{
    Planet* planet = new Planet(name, position, temperature, species);
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

    int indexSpecies = 0;
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
    for (const QJsonValue& value : merchandsRate) {
        QJsonObject rate = value.toObject();
        double level = rate["level"].toDouble();

        _conversionRates[static_cast<ConversionRate>(index)] = level;
        index++;
    }

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

        Planet* planet = new Planet(name, {1, 1, 1}, 0, static_cast<Species>(species));

        // Buildings
        int indexBuilding = 0;
        QJsonArray buildings = planetObj["buildings"].toArray();
        for (const QJsonValue& b : buildings) {
            QJsonObject building = b.toObject();
            int level = building["level"].toInt();

            planet->setTechLevel(TechType::CommonBuilding, indexBuilding, level);
            indexBuilding++;
        }

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
        const CommonTech& tech = TechManager::instance().getTech(TechType::CommonResearch, static_cast<int>(itr->first));
        QJsonObject obj;
        obj["label"] = tech.name;
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

    int index = 0;
    for (auto itr = _conversionRates.begin(); itr != _conversionRates.end(); ++itr) {
        QJsonObject obj;
        obj["label"] = conversionRateToString[index];
        obj["level"] = itr->second;
        conversionArray.append(obj);
        index++;
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
    planetObj["species"] = static_cast<int>(planet->getSpecies());

    // Buildings
    QJsonArray buildingsArray;
    for (int i = 0; i < planet->getNumberTech(TechType::CommonBuilding); ++i)
    {
        const CommonTech& tech = TechManager::instance().getTech(TechType::CommonBuilding, i);
        int level = planet->getTechLevel(TechType::CommonBuilding, i);

        QJsonObject obj;
        obj["label"] = tech.name;
        obj["level"] = level;
        buildingsArray.append(obj);
    }

    planetObj["buildings"] = buildingsArray;

    // Empty arrays
    planetObj["buildingsHumans"]  = QJsonArray{};
    planetObj["buildingsMechs"]   = QJsonArray{};
    planetObj["buildingsKaelesh"] = QJsonArray{};
    planetObj["buildingsRoctas"]  = QJsonArray{};
    planetObj["researchHuman"]    = QJsonArray{};
    planetObj["researchMechs"]    = QJsonArray{};
    planetObj["researchKaelesh"]  = QJsonArray{};
    planetObj["researchRoctas"]   = QJsonArray{};
    planetObj["defenses"]         = QJsonArray{};

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
