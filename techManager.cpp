#include "techManager.h"

#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <cmath>

TechManager& TechManager::instance()
{
    static TechManager instance;
    return instance;
}

TechManager::TechManager()
{
    loadConfig(TechType::CommonBuilding,    ":/config/buildings.json");
    loadConfig(TechType::CommonResearch,    ":/config/researches.json");
    loadConfig(TechType::HumanBuilding,     ":/config/lifeFormBuildingsHuman.json");
    loadConfig(TechType::KaeleshBuilding,   ":/config/lifeFormBuildingsKaelesh.json");
    loadConfig(TechType::MechBuilding,      ":/config/lifeFormBuildingsMech.json");
    loadConfig(TechType::RoctasBuilding,    ":/config/lifeFormBuildingsRoctas.json");
    loadConfig(TechType::HumanResearch,     ":/config/lifeFormResearchesHuman.json");
    loadConfig(TechType::KaeleshResearch,   ":/config/lifeFormResearchesKaelesh.json");
    loadConfig(TechType::MechResearch,      ":/config/lifeFormResearchesMech.json");
    loadConfig(TechType::RoctasResearch,    ":/config/lifeFormResearchesRoctas.json");
    loadUnits(":/config/units.json");
}

bool TechManager::loadConfig(TechType techType, const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Fichier introuvable : " << path;
        return false;
    }

    QJsonParseError err;
    QByteArray raw = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);

    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Erreur JSON : " << path << " : " << err.errorString();
        return false;
    }

    int index = 0;
    QJsonArray array = doc.array();
    for (const QJsonValue &val : array) {
        QJsonObject obj = val.toObject();
        QString name = obj["name"].toString();

        Ressources<float> cost;
        cost.metal   = obj["metal"].toInt(0);
        cost.cristal = obj["cristal"].toInt(0);
        cost.deut    = obj["deut"].toInt(0);
        cost.energy  = obj["energie"].toInt(0);

        if (isLifeFormBuilding(techType))
        {
            Species species = getAssociatedSpecies(techType);

            LifeFormBuilding lifeFormBuilding;
            lifeFormBuilding.name = name;
            lifeFormBuilding.baseCost = cost;
            lifeFormBuilding.increaseFactor = (float)obj["increaseFactor"].toDouble(1.0);
            lifeFormBuilding.durationBase = obj["durationBase"].toInt(0);
            lifeFormBuilding.durationFactor = (float)obj["durationFactor"].toDouble(1.0);
            lifeFormBuilding.species = species;
            lifeFormBuilding.bonuses = extractBonusesBuilding(obj);

            _techs[techType][index++] = std::make_unique<LifeFormBuilding>(lifeFormBuilding);
        }
        else if (isLifeFormResearch(techType))
        {
            Species species = getAssociatedSpecies(techType);

            LifeFormTech lifeFormTech;
            lifeFormTech.name = name;
            lifeFormTech.baseCost = cost;
            lifeFormTech.increaseFactor = (float)obj["increaseFactor"].toDouble(2.0);
            lifeFormTech.durationBase = obj["durationBase"].toInt(0);
            lifeFormTech.durationFactor = (float)obj["durationFactor"].toDouble(1.0);
            lifeFormTech.species = species;
            lifeFormTech.bonuses = extractBonuses(obj);

            _techs[techType][index++] = std::make_unique<LifeFormTech>(lifeFormTech);
        }
        else
        {
            CommonTech tech;
            tech.name = name;
            tech.baseCost = cost;
            tech.increaseFactor = (float)obj["increaseFactor"].toDouble(2.0);
            _techs[techType][index++] = std::make_unique<CommonTech>(tech);
        }
    }

    return true;
}

bool TechManager::loadUnits(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Fichier introuvable : " << path;
        return false;
    }

    QJsonParseError err;
    QByteArray raw = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);

    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Erreur JSON : " << path << " : " << err.errorString();
        return false;
    }

    int index = 0;
    QJsonArray array = doc.array();
    for (const QJsonValue &val : array) {
        UnitType type = static_cast<UnitType>(index);
        QJsonObject obj = val.toObject();

        Unit unit;
        unit.name           = obj["name"].toString();
        unit.cost.metal     = obj["metal"].toInt(0);
        unit.cost.cristal   = obj["cristal"].toInt(0);
        unit.cost.deut      = obj["deut"].toInt(0);
        unit.speed          = obj["speed"].toInt(0);

        _units[type] = std::move(unit);
        index++;
    }
    return true;
}

int TechManager::getNumberTechs(TechType techType) const
{
    if (_techs.contains(techType))
    {
        return _techs.at(techType).size();
    }
    else
    {
        qDebug() << "ERROR : no tech type : " << (int)techType;
        return 0;
    }

}

const CommonTech* TechManager::getTech(TechType techType, int type) const
{
    auto it = _techs.find(techType);
    if (it == _techs.end())
    {
        qWarning() << "tech type : " << static_cast<int>(techType) << " is out of range";
        return nullptr;
    }

    const auto& list = it->second;
    if (type < 0 || type >= static_cast<int>(list.size()))
    {
        qWarning() << "type : " << type << " is out of range";
        return nullptr;
    }

    const CommonTech* tech = list.at(type).get();
    return tech;
}

int TechManager::getProductionMine(CommonBuildingType mineType, int level, float bonus, int temperatureMax) const
{
    int prodHours;
    switch(mineType)
    {
    case CommonBuildingType::MineMetal:
        prodHours = (bonus * 30.0f * (float)level * std::pow(1.1, level));
        break;
    case CommonBuildingType::MineCristal:
        prodHours = (bonus * 20.0f * (float)level * std::pow(1.1, level));
        break;
    case CommonBuildingType::MineDeut:
        prodHours = (bonus * 10.0f * (float)level * std::pow(1.1f, level) * (1.44f - 0.004f * (float)temperatureMax));
        break;
    default:
        return 0;
    }
    return prodHours * 24;
}

Ressources<float> TechManager::getCost(TechType techType, int indexTech, int level) const
{
    const CommonTech* commonTech = getTech(techType, indexTech);
    switch (techType)
    {
        case TechType::CommonBuilding:
        case TechType::CommonResearch:
            return commonTech->baseCost * std::pow(commonTech->increaseFactor, level - 1);
        case TechType::HumanBuilding:
        case TechType::MechBuilding:
        case TechType::KaeleshBuilding:
        case TechType::RoctasBuilding:
        case TechType::HumanResearch:
        case TechType::MechResearch:
        case TechType::KaeleshResearch:
        case TechType::RoctasResearch:
            return level * commonTech->baseCost * std::pow(commonTech->increaseFactor, level - 1);
        default:
            return Ressources<float>();
    }
}

float TechManager::getBaseTime(TechType techType, int indexTech, int level) const
{
    float timeHours;
    switch (techType)
    {
        case TechType::CommonBuilding:
        {
            Ressources<float> basicCost = getCost(techType, indexTech, level);
            timeHours = (basicCost.metal + basicCost.cristal) / (2500.0f * (float)std::max(4 - level/2, 1));
            break;
        }
        case TechType::CommonResearch:
        {
            Ressources<float> basicCost = getCost(techType, indexTech, level);
            timeHours = (basicCost.metal + basicCost.cristal) / 1000.0f;
            break;
        }
        case TechType::HumanBuilding:
        case TechType::MechBuilding:
        case TechType::KaeleshBuilding:
        case TechType::RoctasBuilding:
        case TechType::HumanResearch:
        case TechType::MechResearch:
        case TechType::KaeleshResearch:
        case TechType::RoctasResearch:
        {
            const CommonTech* tech = getTech(techType, indexTech);
            const LifeFormTech* research = static_cast<const LifeFormTech*>(tech);
            float timeSeconds = (float)level * (float)research->durationBase * (float)std::pow(research->durationFactor, level);
            timeHours = timeSeconds / 3600.0f;
            break;
        }
    }

    return timeHours / 24.0f;
}

bool TechManager::isLifeFormBuilding(TechType techType) const
{
    switch(techType)
    {
        case TechType::HumanBuilding:
        case TechType::MechBuilding:
        case TechType::KaeleshBuilding:
        case TechType::RoctasBuilding:
            return true;
        default:
            return false;
    }
}

bool TechManager::isLifeFormResearch(TechType techType) const
{
    switch(techType)
    {
        case TechType::HumanResearch:
        case TechType::MechResearch:
        case TechType::KaeleshResearch:
        case TechType::RoctasResearch:
            return true;
        default:
            return false;
    }
}

Species TechManager::getAssociatedSpecies(TechType techType) const
{
    switch(techType)
    {
        case TechType::HumanBuilding:
        case TechType::HumanResearch:
            return Species::Humans;

        case TechType::MechBuilding:
        case TechType::MechResearch:
            return Species::Mechs;

        case TechType::KaeleshBuilding:
        case TechType::KaeleshResearch:
            return Species::Kaeleshs;

        case TechType::RoctasBuilding:
        case TechType::RoctasResearch:
            return Species::Roctas;

        default:
            return Species::None;
    }
}

TechType TechManager::getBuildingTech(Species species) const
{
    switch(species)
    {
        case Species::Humans:
            return TechType::HumanBuilding;
        case Species::Mechs:
            return TechType::MechBuilding;
        case Species::Kaeleshs:
            return TechType::KaeleshBuilding;
        case Species::Roctas:
            return TechType::RoctasBuilding;
        default:
            return TechType::None;
    }
}

TechType TechManager::getResearchTech(Species species) const
{
    switch(species)
    {
    case Species::Humans:
        return TechType::HumanResearch;
    case Species::Mechs:
        return TechType::MechResearch;
    case Species::Kaeleshs:
        return TechType::KaeleshResearch;
    case Species::Roctas:
        return TechType::RoctasResearch;
    default:
        return TechType::None;
    }
}

const Unit& TechManager::getUnit(UnitType type) const
{
    return _units.at(type);
}

std::map<BonusLifeFormBuilding, double> TechManager::extractBonusesBuilding(const QJsonObject& obj) const
{
    std::map<BonusLifeFormBuilding, double> result;
    for (auto itr = bonusLifeFormBuildingStr.begin(); itr != bonusLifeFormBuildingStr.end(); ++itr)
    {
        BonusLifeFormBuilding bonusLifeForm = itr->first;
        if (obj[itr->second].isDouble())
        {
            result[bonusLifeForm] = obj[itr->second].toDouble(0.0);
        }
    }
    return result;
}

std::map<BonusLifeForm, double> TechManager::extractBonuses(const QJsonObject& obj) const
{
    std::map<BonusLifeForm, double> result;
    for (auto itr = bonusLifeFormStr.begin(); itr != bonusLifeFormStr.end(); ++itr)
    {
        BonusLifeForm bonusLifeForm = itr->first;
        if (obj[itr->second].isDouble())
        {
            result[bonusLifeForm] = obj[itr->second].toDouble(0.0);
        }
    }
    return result;
}
