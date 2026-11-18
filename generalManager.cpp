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

    readResearches(root);

    qDebug() << "Loading done : " << filePath;

    return true;
}

void GeneralManager::refresh()
{
}

bool GeneralManager::clear()
{
    for (int i = 0; i < static_cast<int>(ResearchType::Count); ++i) {
        _levelResearch[static_cast<ResearchType>(i)] = 0;
    }
    return true;
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

QJsonDocument GeneralManager::generateGameDataJson()
{
    QJsonObject root;

    writeResearches(root);

    return QJsonDocument(root);
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
