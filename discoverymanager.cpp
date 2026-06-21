#include "discoverymanager.h"
#include "techManager.h"
#include "playermanager.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

DiscoveryManager& DiscoveryManager::instance()
{
    static DiscoveryManager instance;
    return instance;
}

DiscoveryManager::DiscoveryManager()
{
    QString path = getSavePath();
    if (QFileInfo::exists(path)) {
        loadSave(path);
    } else {
        loadInit();
    }
}

DiscoveryManager::~DiscoveryManager()
{
    save(getSavePath());
}

QString DiscoveryManager::getSavePath()
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(basePath);
    return basePath + "/discovery.json";
}

void DiscoveryManager::loadInit()
{
    for (int i = 1; i < static_cast<int>(DiscoveryType::Count); ++i)
    {
        DiscoveryType type = static_cast<DiscoveryType>(i);
        dataDiscoveries[type] = Discovery(type);
    }

    dataDiscoveries[DiscoveryType::Metal].availableRessources.push_back(RessourceType::Metal);
    dataDiscoveries[DiscoveryType::Cristal].availableRessources.push_back(RessourceType::Cristal);
    dataDiscoveries[DiscoveryType::Deut].availableRessources.push_back(RessourceType::Deut);
    dataDiscoveries[DiscoveryType::Antimatter].availableRessources.push_back(RessourceType::Antimatter);

    dataDiscoveries[DiscoveryType::Fleat].availableRessources.push_back(RessourceType::Metal);
    dataDiscoveries[DiscoveryType::Fleat].availableRessources.push_back(RessourceType::Cristal);
    dataDiscoveries[DiscoveryType::Fleat].availableRessources.push_back(RessourceType::Deut);

    dataDiscoveries[DiscoveryType::Combat].availableRessources.push_back(RessourceType::Metal);
    dataDiscoveries[DiscoveryType::Combat].availableRessources.push_back(RessourceType::Cristal);
    dataDiscoveries[DiscoveryType::Combat].availableRessources.push_back(RessourceType::Deut);

    dataDiscoveries[DiscoveryType::Blackhole].availableRessources.push_back(RessourceType::Metal);
    dataDiscoveries[DiscoveryType::Blackhole].availableRessources.push_back(RessourceType::Cristal);
    dataDiscoveries[DiscoveryType::Blackhole].availableRessources.push_back(RessourceType::Deut);
}

DiscoveryManager::DiscoveryType DiscoveryManager::extractDiscoveryType(const QString& str) const
{
    auto it = std::find_if(discoveryTypeToString.begin(), discoveryTypeToString.end(),
        [str](const auto& key){return key.second == str;
    });

    if (it != discoveryTypeToString.end())
    {
        return it->first;
    }
    else
    {
        return DiscoveryType::Error;
    }
}

void DiscoveryManager::addDiscover(DiscoveryManager::DiscoveryType type, Ressources value, int count)
{
    dataDiscoveries[type].add(value, count);
}

bool DiscoveryManager::loadSave(QString path)
{
    // Ouvre le fichier JSON
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Impossible d'ouvrir le fichier";
        return false;
    }

    // Lecture du contenu
    QByteArray jsonData = file.readAll();
    file.close();

    // Conversion en document JSON
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (!doc.isObject()) {
        qDebug() << "JSON invalide";
        return false;
    }

    // Objet principal
    QJsonObject rootObj = doc.object();

    // Tableau "discoveries"
    discoveryPerDay = (float)rootObj["discoveryPerDay"].toDouble();
    deutConsumption = rootObj["deutConsumption"].toInt();
    additionalBonusRessources = rootObj["tempBonus"].toInt();
    positionDiscovery = rootObj["positionDiscovery"].toInt();

    // Parcours du tableau
    QJsonArray discoveries = rootObj["discoveries"].toArray();
    for (const QJsonValue &value : discoveries) {
        QJsonObject obj = value.toObject();

        DiscoveryType type = extractDiscoveryType(obj["type"].toString());

        Discovery discovery(type);
        discovery.type = type;
        discovery.count = obj["count"].toInt();

        Ressources meanLoaded;
        if (obj.contains("meanMetal"))
        {
            meanLoaded.metal = obj["meanMetal"].toDouble();
            discovery.availableRessources.push_back(RessourceType::Metal);
        }
        if (obj.contains("meanCristal"))
        {
            meanLoaded.cristal = obj["meanCristal"].toDouble();
            discovery.availableRessources.push_back(RessourceType::Cristal);
        }
        if (obj.contains("meanDeut"))
        {
            meanLoaded.deut = obj["meanDeut"].toDouble();
            discovery.availableRessources.push_back(RessourceType::Deut);
        }
        if (obj.contains("meanAM"))
        {
            meanLoaded.antimatter = obj["meanAM"].toDouble();
            discovery.availableRessources.push_back(RessourceType::Antimatter);
        }
        discovery.mean = meanLoaded;

        dataDiscoveries[type] = discovery;
    }
    return true;
}

void DiscoveryManager::loadBonusFactor(float ressourcesBonus, float shipBonus)
{
    float ressourceFactor = 1.0f + (ressourcesBonus + (float)additionalBonusRessources) / 100.0f;
    float shipFactor = 1.0f + shipBonus / 100.0f;

    dataDiscoveries[DiscoveryType::Metal].bonusFactor = ressourceFactor;
    dataDiscoveries[DiscoveryType::Cristal].bonusFactor = ressourceFactor;
    dataDiscoveries[DiscoveryType::Deut].bonusFactor = ressourceFactor;
    dataDiscoveries[DiscoveryType::Antimatter].bonusFactor = ressourceFactor;

    dataDiscoveries[DiscoveryType::Fleat].bonusFactor = shipFactor;
}

bool DiscoveryManager::save(QString path)
{
    // Création du tableau discoveries
    QJsonArray discoveriesArray;

    for (int i = 1; i < static_cast<int>(DiscoveryType::Count); i++)
    {
        DiscoveryType type = static_cast<DiscoveryType>(i);
        QString typeStr = discoveryTypeToString.at(type);
        Discovery discovery = dataDiscoveries.at(type);

        QJsonObject obj;
        obj["type"] = typeStr;
        obj["count"] = discovery.count;

        if (discovery.hasRessource(RessourceType::Metal)){
            obj["meanMetal"] = discovery.mean.metal;
        }
        if (discovery.hasRessource(RessourceType::Cristal))
        {
            obj["meanCristal"] = discovery.mean.cristal;
        }
        if (discovery.hasRessource(RessourceType::Deut))
        {
            obj["meanDeut"] = discovery.mean.deut;
        }
        if (discovery.hasRessource(RessourceType::Antimatter))
        {
            obj["meanAM"] = discovery.mean.antimatter;
        }

        discoveriesArray.append(obj);
    }

    // Objet JSON principal
    QJsonObject rootObj;
    rootObj["discoveries"] = discoveriesArray;
    rootObj["discoveryPerDay"] = discoveryPerDay;
    rootObj["deutConsumption"] = deutConsumption;
    rootObj["tempBonus"] = additionalBonusRessources;
    rootObj["positionDiscovery"] = positionDiscovery;

    // Conversion en document JSON
    QJsonDocument doc(rootObj);

    // Ouverture du fichier
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Impossible d'ouvrir le fichier en écriture";
        return false;
    }

    // Écriture dans le fichier
    file.write(doc.toJson(QJsonDocument::Indented));

    file.close();

    return true;
}

void DiscoveryManager::refresh()
{
    computeDiscoverySpeed();
    timeToPos16 = computeTimeToPos16();
}

void DiscoveryManager::computeRentability()
{
    int globalCount = 0;
    for (auto itr = dataDiscoveries.begin(); itr != dataDiscoveries.end(); ++itr)
    {
        globalCount += itr->second.count;
    }

    const Discovery& metalDiscovery = dataDiscoveries.at(DiscoveryType::Metal);
    float factorMetal = (float)metalDiscovery.count / (float)globalCount * discoveryPerDay;
    summary.meanRessourceFound.metal = factorMetal * metalDiscovery.mean.metal;

    const Discovery& cristalDiscovery = dataDiscoveries.at(DiscoveryType::Cristal);
    float factorCristal = (float)cristalDiscovery.count / (float)globalCount * discoveryPerDay;
    summary.meanRessourceFound.cristal = factorCristal * cristalDiscovery.mean.cristal;

    const Discovery& deutDiscovery = dataDiscoveries.at(DiscoveryType::Deut);
    float factorDeut = (float)deutDiscovery.count / (float)globalCount * discoveryPerDay;
    summary.meanRessourceFound.deut = factorDeut * deutDiscovery.mean.deut;

    const Discovery& shipDiscovery = dataDiscoveries.at(DiscoveryType::Fleat);
    float factorShip = (float)shipDiscovery.count / (float)globalCount * discoveryPerDay;
    summary.meanShipFound = factorShip * shipDiscovery.mean;

    summary.meanLost = dataDiscoveries.at(DiscoveryType::Combat).mean;
    summary.meanLost += dataDiscoveries.at(DiscoveryType::Blackhole).mean;
    summary.meanLost += Ressources(0.0f, 0.0f, (float)deutConsumption);
}

QString DiscoveryManager::getTypeStrList(DiscoveryType type) const
{
    return discoveryTypeToString.at(type);
}

DiscoveryManager::Discovery DiscoveryManager::getDiscovery(DiscoveryType type) const
{
    return dataDiscoveries.at(type);
}

float DiscoveryManager::getDiscoveryPerDay() const
{
    return discoveryPerDay;
}

int DiscoveryManager::getDeutConsumption() const
{
    return deutConsumption;
}

int DiscoveryManager::getPositionDiscovery() const
{
    return positionDiscovery;
}

int DiscoveryManager::getTempBonusRessources() const
{
    return additionalBonusRessources;
}

const DiscoveryManager::SummaryPerDay& DiscoveryManager::getSummary() const
{
    return summary;
}

void DiscoveryManager::setDiscoveryPerDay(float value)
{
    discoveryPerDay = value;
}

void DiscoveryManager::setDeutConsumption(int deut)
{
    deutConsumption = deut;
}

void DiscoveryManager::setPositionDiscovery(int position)
{
    positionDiscovery = position;
}

void DiscoveryManager::setTempBonusRessources(int bonus)
{
    additionalBonusRessources = bonus;
}

void DiscoveryManager::computeDiscoverySpeed()
{
    int levelPropCombustion = PlayerManager::instance().getResearchLevel(ResearchType::PropCombusion);
    Class classPlayer = PlayerManager::instance().getClass();
    AllianceClass allianceClass = PlayerManager::instance().getAllianceClass();

    bonusSpeedPercent = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::LargeCargoUpdate);
    bonusSpeedPercent += PlayerManager::instance().getLifeFormBonus(BonusLifeForm::SpeedCivilianShips);
    bonusSpeedPercent += 10.0f * levelPropCombustion;

    if (classPlayer == Class::Collector)
    {
        bonusSpeedPercent += 100.0f;
    }
    if (allianceClass == AllianceClass::Merchand)
    {
        bonusSpeedPercent += 10.0f;
    }
}

std::chrono::seconds DiscoveryManager::computeTimeToPos16()
{
    const Unit& largeCargo = TechManager::instance().getUnit(UnitType::LargeCargo);
    int initSpeed = largeCargo.speed;
    float speed = (float)initSpeed * (1.0f + bonusSpeedPercent / 100.0f);

    float percentSpeed = 100.0f;
    float distance = (float)(16 - positionDiscovery);
    int time =  std::round(10.0f + 35000.0f/percentSpeed * std::sqrt((1000000.0f + distance * 5000) / speed));
    return std::chrono::seconds(time);
}
