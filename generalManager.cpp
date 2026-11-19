#include "GeneralManager.h"
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

GeneralManager& GeneralManager::instance()
{
    static GeneralManager instance;
    return instance;
}

GeneralManager::GeneralManager()
{
    clear();
    refresh();
}

bool GeneralManager::loadSave(const QString& path)
{
    QString filePath = path + "/researches.json";
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

    readUniverses(root);
    readSpecies(root);
    readClassData(root);
    readOfficersData(root);
    readConversionData(root);
    readResearches(root);

    qDebug() << "Loading done : " << filePath;

    return true;
}

int GeneralManager::getResearchLevel(ResearchType researchType) const
{
    auto it = _levelResearch.find(researchType);
    return it != _levelResearch.end() ? it->second : 0;
}

int GeneralManager::getUniverseSpecific(UniverseSpecifics universeSpecific) const
{
    auto it = _universeSpecifics.find(universeSpecific);
    return it != _universeSpecifics.end() ? it->second : 1;
}

bool GeneralManager::getOfficerValue(Officers officer) const
{
    auto it = _officers.find(officer);
    return it != _officers.end() ? it->second : false;
}

int GeneralManager::getLevelSpecies(Species species) const
{
    auto it = _levelSpecies.find(species);
    return it != _levelSpecies.end() ? it->second : 0;
}

Ressources<float> GeneralManager::getPlasmaBonus() const
{
    float levelPlasma = (float)getResearchLevel(ResearchType::Plasma);
    return Ressources<float>(levelPlasma, 0.66f * levelPlasma, 0.33f * levelPlasma);
}

Ressources<float> GeneralManager::getIngeniorBonus() const
{
    if (getOfficerValue(Officers::Engeneer))
    {
        return Ressources<float>(0.0f, 0.0f, 0.0f, 10.0f);
    }
    else
    {
        return Ressources<float>();
    }
}

Ressources<float> GeneralManager::getGeologBonus() const
{
    if (getOfficerValue(Officers::Geolog))
    {
        return Ressources<float>(10.0f, 10.0f, 10.0f);
    }
    else
    {
        return Ressources<float>();
    }
}

Ressources<float> GeneralManager::getCouncilBonus() const
{
    if (getOfficerValue(Officers::Concil))
    {
        return Ressources<float>(2.0f, 2.0f, 2.0f, 2.0f);
    }
    else
    {
        return Ressources<float>();
    }
}

Ressources<float> GeneralManager::getClassBonus() const
{
    if (getClass() == Class::Collector)
    {
        return Ressources<float>(25.0f, 25.0f, 25.0f, 10.0f);
    }
    else
    {
        return Ressources<float>();
    }
}
Ressources<float> GeneralManager::getAllianceClassBonus() const
{
    if (getAllianceClass() == AllianceClass::Merchand)
    {
        return Ressources<float>(5.0f, 5.0f, 5.0f, 5.0f);
    }
    else
    {
        return Ressources<float>();
    }
}

void GeneralManager::computePoints()
{
    _investedPoints.clear();

    // Common research
    for (auto itr =_levelResearch.begin(); itr != _levelResearch.end(); ++itr)
    {
        int level = itr->second;
        int indexResearch = static_cast<int>(itr->first);
        Ressources<float> cumulCost = TechManager::instance().getCumulativeCost(TechType::CommonResearch, indexResearch, level);
        _investedPoints += cumulCost / 1000;
    }
}

void GeneralManager::refresh()
{
    computePoints();
}

bool GeneralManager::clear()
{
    _class = Class::None;
    _allianceClass = AllianceClass::None;
    _scrapRate = 35;

    for (int i = 0; i < static_cast<int>(UniverseSpecifics::Count); ++i) {
        _universeSpecifics[static_cast<UniverseSpecifics>(i)] = 1;
    }

    for (int i = 1; i < static_cast<int>(Species::Count); ++i) {
        _levelSpecies[static_cast<Species>(i)] = 0;
    }

    for (int i = 0; i < static_cast<int>(Officers::Count); ++i) {
        _officers[static_cast<Officers>(i)] = false;
    }

    for (int i = 0; i < static_cast<int>(ResearchType::Count); ++i) {
        _levelResearch[static_cast<ResearchType>(i)] = 0;
    }

    setConversionRate(Ressources(3.0f, 2.0f, 1.0f));
    return true;
}

void GeneralManager::readUniverses(const QJsonObject &parent)
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

void GeneralManager::readSpecies(const QJsonObject& parent)
{
    _levelSpecies.clear();
    QJsonArray species = parent["species"].toArray();

    int indexSpecies = 1;
    for (const QJsonValue& value : species) {
        QJsonObject race = value.toObject();
        int level = race["level"].toInt();

        _levelSpecies[static_cast<Species>(indexSpecies)] = level;
        indexSpecies++;
    }
}

void GeneralManager:: readClassData(const QJsonObject& parent)
{
    if (!parent.contains("classData") || !parent["classData"].isArray())
        return;

    QJsonArray classDatas = parent["classData"].toArray();

    if (classDatas.size() < 2)
        return;

    if (!classDatas[0].isObject() || !classDatas[1].isObject())
        return;

    QJsonObject classData = classDatas[0].toObject();
    QJsonObject allianceData = classDatas[1].toObject();

    int indexClass = classData["index"].toInt(0);
    int indexAlliance = allianceData["index"].toInt(0);

    _class = static_cast<Class>(indexClass);
    _allianceClass = static_cast<AllianceClass>(indexAlliance);
}

void GeneralManager:: readOfficersData(const QJsonObject& parent)
{
    _officers.clear();
    QJsonArray officers = parent["officers"].toArray();

    int indexOfficers = 0;
    for (const QJsonValue& value : officers) {
        QJsonObject research = value.toObject();
        bool activated = research["active"].toBool();

        _officers[static_cast<Officers>(indexOfficers)] = activated;
        indexOfficers++;
    }
}

void GeneralManager::readResearches(const QJsonObject& parent)
{
    _levelResearch.clear();
    QJsonArray researches = parent["researches"].toArray();

    int indexResearch = 0;
    for (const QJsonValue& value : researches) {
        QJsonObject research = value.toObject();
        int level = research["level"].toInt();

        _levelResearch[static_cast<ResearchType>(indexResearch)] = level;
        indexResearch++;
    }
}

void GeneralManager::readConversionData(const QJsonObject& parent)
{
    // Conversion rates
    Ressources<float> conversionRate{};

    if (parent.contains("merchandsRate") && parent["merchandsRate"].isArray())
    {
        QJsonArray merchandsRate = parent["merchandsRate"].toArray();

        for (const QJsonValue& value : merchandsRate)
        {
            if (!value.isObject())
                continue;

            QJsonObject rate = value.toObject();

            double level = rate["level"].toDouble(0.0);
            QString name = rate["label"].toString("");

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
            else if (!name.isEmpty())
            {
                qWarning() << "'" << name << "' is not a valid resource name";
            }
        }
    }

    setConversionRate(conversionRate);

    // Scrap rate
    if (!parent.contains("scrapRate") || !parent["scrapRate"].isArray())
        return;

    QJsonArray scrapDatas = parent["scrapRate"].toArray();

    if (scrapDatas.isEmpty() || !scrapDatas[0].isObject())
        return;

    QJsonObject scrapData = scrapDatas[0].toObject();

    _scrapRate = scrapData["level"].toInt(0);
}

QJsonDocument GeneralManager::generateGameDataJson()
{
    QJsonObject root;

    writeUniverses(root);
    writeSpecies(root);
    writeClassData(root);
    writeOfficersData(root);
    writeConversionData(root);
    writeResearches(root);

    return QJsonDocument(root);
}

void GeneralManager::writeUniverses(QJsonObject& parent)
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

void GeneralManager::writeSpecies(QJsonObject& parent)
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

void GeneralManager::writeClassData(QJsonObject& parent)
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

void GeneralManager::writeOfficersData(QJsonObject& parent)
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

void GeneralManager::writeConversionData(QJsonObject& parent)
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

void GeneralManager::writeResearches(QJsonObject& parent)
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

bool GeneralManager::saveGameData(const QString& path)
{
    QJsonDocument doc = generateGameDataJson();

    QString playerSavePath = path + "/researches.json";
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
