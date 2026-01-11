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
    loadConfig(TechType::CommonBuilding, ":/config/buildings.json");
    loadConfig(TechType::CommonResearch, ":/config/researches.json");
}

bool TechManager::loadConfig(TechType techType, QString path)
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

        Ressources cost;
        cost.metal   = obj["metal"].toInt(0);
        cost.cristal = obj["cristal"].toInt(0);
        cost.deut    = obj["deut"].toInt(0);
        cost.energy  = obj["energie"].toInt(0);

        CommonTech research;
        research.name = name;
        research.baseCost = cost;
        research.increaseFactor = (float)obj["increaseFactor"].toDouble(2.0);

        _techs[techType][index++] = research;
    }

    return true;
}

int TechManager::getNumberTechs(TechType techType) const
{
    return _techs.at(techType).size();
}

const CommonTech& TechManager::getTech(TechType techType, int type) const
{
    static const CommonTech invalidTech{};

    auto it = _techs.find(techType);
    if (it == _techs.end())
    {
        qWarning() << "tech type : " << static_cast<int>(techType) << " is out of range";
        return invalidTech;
    }

    const auto& list = it->second;
    if (type < 0 || type >= static_cast<int>(list.size()))
    {
        qWarning() << "type : " << type << " is out of range";
        return invalidTech;
    }

    const CommonTech& tech = list.at(type);
    return tech;
}
