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

    bool                    loadSave(const QString& path);

    inline int              getNumberResearch() const {return _levelResearch.size();}
    int                     getResearchLevel(ResearchType researchType) const;
    inline Class            getClass() const {return _class;}
    inline AllianceClass    getAllianceClass() const {return _allianceClass;}
    inline int              getScrapRate() const {return _scrapRate;}
    int                     getUniverseSpecific(UniverseSpecifics universeSpecific) const;
    bool                    getOfficerValue(Officers officer) const;
    inline Ressources<float> getConversionRate() const {return _conversionRates;}
    inline const std::map<Species, int>& getAllSpecies() const {return _levelSpecies;}
    int                     getLevelSpecies(Species species) const;
    inline const Ressources<int>& getInvestedRessources() const {return _investedPoints;}

    Ressources<float>       getPlasmaBonus() const;
    Ressources<float>       getIngeniorBonus() const;
    Ressources<float>       getGeologBonus() const;
    Ressources<float>       getCouncilBonus() const;
    Ressources<float>       getClassBonus() const;
    Ressources<float>       getAllianceClassBonus() const;

    inline void             setClass(Class globalClass) {_class = globalClass;}
    inline void             setAllianceClass(AllianceClass allianceClass) {_allianceClass = allianceClass;}
    inline void             setScrapRate(int scrapRate) {_scrapRate = scrapRate;}
    inline void             setUniverseSpecific(UniverseSpecifics universeSpecific, int speed) {_universeSpecifics[universeSpecific] = speed;}
    inline void             setOfficerActivated(Officers officer, bool activated) {_officers[officer] = activated;}
    inline void             setConversionRate(Ressources<float> conversionRate) {_conversionRates = conversionRate;}
    inline void             setConversionRateAt(RessourceType type, float conversionRate) {_conversionRates.setRessource(type, conversionRate);}
    inline void             setSpecies(Species species, int level) {_levelSpecies[species] = level;}
    inline void             setResearchLevel(ResearchType researchType, int level) {_levelResearch[researchType] = level;}

    bool                    saveGameData(const QString &path);
    void                    computePoints();
    void                    refresh();
    bool                    clear();

private:
    void                    readUniverses(const QJsonObject& parent);
    void                    readSpecies(const QJsonObject& parent);
    void                    readClassData(const QJsonObject& parent);
    void                    readOfficersData(const QJsonObject& parent);
    void                    readConversionData(const QJsonObject& parent);
    void                    readResearches(const QJsonObject& parent);

    void                    writeUniverses(QJsonObject& parent);
    void                    writeSpecies(QJsonObject& parent);
    void                    writeClassData(QJsonObject& parent);
    void                    writeOfficersData(QJsonObject& parent);
    void                    writeConversionData(QJsonObject& parent);
    void                    writeResearches(QJsonObject& parent);

    QJsonDocument           generateGameDataJson();

private:
    GeneralManager();
    ~GeneralManager() = default;

    GeneralManager(const GeneralManager&) = delete;
    GeneralManager& operator=(const GeneralManager&) = delete;

    Class                               _class {Class::None};
    AllianceClass                       _allianceClass {AllianceClass::None};
    int                                 _scrapRate {35};

    std::map<UniverseSpecifics, int>    _universeSpecifics;
    std::map<Officers, bool>            _officers;
    Ressources<float>                   _conversionRates;
    std::map<Species, int>              _levelSpecies;
    std::map<ResearchType, int>         _levelResearch;

    Ressources<int>                     _investedPoints;
};
