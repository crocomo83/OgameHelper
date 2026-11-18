#pragma once

#include "commons.h"

#include <map>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QStringList>

class GeneralManager
{
public:
    static GeneralManager& instance();

    bool            loadSave(const QString& path);

    inline int      getNumberResearch() const {return _levelResearch.size();}
    inline int      getResearchLevel(ResearchType researchType) const {return _levelResearch.at(researchType);}
    inline void     setResearchLevel(ResearchType researchType, int level) {_levelResearch[researchType] = level;}

    bool            saveGameData(const QString &path);
    void            refresh();
    bool            clear();

private:
    void            readResearches(const QJsonObject& parent);

    void            writeResearches(QJsonObject& parent);

    QJsonDocument   generateGameDataJson();

private:
    GeneralManager();
    ~GeneralManager() = default;

    GeneralManager(const GeneralManager&) = delete;
    GeneralManager& operator=(const GeneralManager&) = delete;

    std::map<ResearchType, int>         _levelResearch;
};
