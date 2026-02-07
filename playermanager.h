#pragma once

#include "commons.h"
#include "planet.h"

#include <map>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QStringList>

class PlayerManager
{
public:
    static PlayerManager& instance();

    enum ProductionStat{
        Base,
        Mines,
        BuildingLifeFormTotal,
        CrawlersTotal,
        Plasma,
        LifeFormBonus,
        Geolog,
        ClassBonus,
        AllianceBonus,
        Count
    };

    const QStringList _prods = {
        "Base",
        "Mines",
        "Building life form",
        "Crawlers",
        "Plasma",
        "Research life form",
        "Geolog",
        "Class bonus",
        "Alliance bonus"
    };

    int             getNumberPlanets() const;
    int             getNumberResearch() const;
    const QString&  getPlanetName(int index) const;
    Planet *        getPlanet(int index) const;

    Ressources      getPlasmaBonus() const;
    Ressources      getLifeFormProdBonus() const;
    Ressources      getGeologBonus() const;
    Ressources      getClassBonus() const;
    Ressources      getAllianceClassBonus() const;

    Class           getClass() const;
    AllianceClass   getAllianceClass() const;
    int             getScrapRate() const;
    int             getUniverseSpecific(UniverseSpecifics universeSpecific) const;
    bool            getOfficerValue(Officers officer) const;
    Ressources      getConversionRate() const;
    int             getResearchLevel(ResearchType researchType) const;
    int             getSpecies(Species species) const;

    void            computeProduction();
    const Ressources&   getProduction(ProductionStat stat) const;
    const QString& getProductionStr(ProductionStat stat) const;

    void            setClass(Class globalClass);
    void            setAllianceClass(AllianceClass allianceClass);
    void            setScrapRate(int scrapRate);
    void            setUniverseSpecific(UniverseSpecifics universeSpecific, int speed);
    void            setOfficerActivated(Officers officer, bool activated);
    void            setConversionRate(Ressources conversionRate);
    void            setConversionRateAt(RessourceType type, float conversionRate);
    void            setResearchLevel(ResearchType researchType, int level);
    void            setSpecies(Species species, int level);

    void            addPlanet(const QString& name, const PlanetPosition& position, int temperature, Species species = Species::None);
    void            duplicatePlanet();

    bool            saveGameData();

private:
    void            readUniverses(const QJsonObject& parent);
    void            readSpecies(const QJsonObject& parent);
    void            readResearches(const QJsonObject& parent);
    void            readClassData(const QJsonObject& parent);
    void            readOfficersData(const QJsonObject& parent);
    void            readConversionData(const QJsonObject& parent);
    void            readPlanetData(const QJsonObject& parent);

    void            writeUniverses(QJsonObject& parent);
    void            writeSpecies(QJsonObject& parent);
    void            writeResearches(QJsonObject& parent);
    void            writeClassData(QJsonObject& parent);
    void            writeOfficersData(QJsonObject& parent);
    void            writeConversionData(QJsonObject& parent);
    void            writePlanetData(QJsonArray& parent, int indexPlanet);

    QJsonDocument   generateGameDataJson();
    QString         getSavePath();

    bool            loadInitSave();
    bool            loadSave(const QString& path);

private:
    PlayerManager();
    ~PlayerManager() = default;

    PlayerManager(const PlayerManager&) = delete;
    PlayerManager& operator=(const PlayerManager&) = delete;

    Class                                   _class;
    AllianceClass                           _allianceClass;
    int                                     _scrapRate;

    std::map<UniverseSpecifics, int>        _universeSpecifics;
    std::map<Officers, bool>                _officers;
    Ressources                              _conversionRates;
    std::map<ResearchType, int>             _levelResearch;
    std::map<Species, int>                  _levelSpecies;
    std::vector<Planet*>                    _planets;
    std::map<BonusLifeForm, float>          _lifeFormBonuses;

    std::map<ProductionStat, Ressources>    _productionStats;
};
