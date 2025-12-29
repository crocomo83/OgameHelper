#include "researchHandler.h"

#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

ResearchHandler::ResearchHandler()
{
    QFile file(":/config/researches.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Fichier introuvable";
        return;
    }

    QJsonParseError err;
    QByteArray raw = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);

    if (err.error != QJsonParseError::NoError) {
        qWarning() << "Erreur JSON :" << err.errorString();
        qDebug() << "Contenu du fichier :" << raw;
        return;
    }

    int index = 0;

    QJsonArray array = doc.array();
    for (const QJsonValue &val : array) {
        QJsonObject obj = val.toObject();
        QString name = obj["name"].toString();

        Cost cost;
        cost.metal   = obj["metal"].toInt(0);
        cost.cristal = obj["cristal"].toInt(0);
        cost.deut    = obj["deut"].toInt(0);
        cost.energy  = obj["energie"].toInt(0);

        Research research;
        research.name = name;
        research.baseCost = cost;
        research.increaseFactor = (float)obj["increaseFactor"].toDouble(2.0);
        research.currentLevel = 0;

        ResearchType type = static_cast<ResearchType>(index++);
        _researches[type] = research;
    }
}

int ResearchHandler::getNumberResearch() const
{
    return _researches.size();
}

const ResearchHandler::Research& ResearchHandler::getResearch(ResearchType type) const
{
    return _researches.at(type);
}
