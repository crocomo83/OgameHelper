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

    enum class ProductionStat{
        Base,
        Mines,
        BuildingLifeFormTotal,
        CrawlersTotal,
        Plasma,
        LifeFormBonus,
        Geolog,
        ClassBonus,
        AllianceBonus,
        Total,
        Count
    };

    enum class ProductionStatPercent{
        PlasmaPercent,
        LifeFormBonusPercent,
        GeologPercent,
        ClassBonusPercent,
        AllianceBonusPercent,
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
        "Alliance bonus",
        "Total"
    };

    const QStringList _prodsPercent = {
        "Plasma percent",
        "Research life form percent",
        "Geolog percent",
        "Class bonus percent",
        "Alliance bonus percent"
    };

    inline int              getNumberPlanets() const {return _planets.size();}
    inline int              getNumberResearch() const {return _levelResearch.size();}
    inline const QString&   getPlanetName(int index) const {return _planets.at(index).getName();}
    inline Planet&          getPlanet(int index) {return _planets[index];}
    inline float            getLifeFormBonus(BonusLifeForm bonus) const {return _lifeFormBonuses.at(bonus);}
    inline Class            getClass() const {return _class;}
    inline AllianceClass    getAllianceClass() const {return _allianceClass;}
    inline int              getScrapRate() const {return _scrapRate;}
    inline int              getUniverseSpecific(UniverseSpecifics universeSpecific) const { return _universeSpecifics.at(universeSpecific);}
    inline bool             getOfficerValue(Officers officer) const {return _officers.at(officer);}
    inline Ressources<float> getConversionRate() const {return _conversionRates;}
    inline int              getResearchLevel(ResearchType researchType) const {return _levelResearch.at(researchType);}
    inline int              getSpecies(Species species) const {return _levelSpecies.at(species);}

    Ressources<float>       getPlasmaBonus() const;
    Ressources<float>       getLifeFormProdBonus() const;
    Ressources<float>       getGeologBonus() const;
    Ressources<float>       getClassBonus() const;
    Ressources<float>       getAllianceClassBonus() const;
    float                   getResearchTime(int indexTech, int level) const;
    Planet&                 getPlanifPlanet(const Species& species);
    inline Planet&          getPlanifAstro() {return _planifAstro;}

    void                    refresh();
    void                    computeLifeFormResearch();
    void                    computeProduction();
    void                    computeLabsLevel();
    void                    computeConversionRate();

    inline const Ressources<float>& getProduction(ProductionStat stat) const {return _productionStats.at(stat);}
    inline const Ressources<float>& getProductionPercent(ProductionStatPercent stat) const {return _productionStatsPercent.at(stat);}
    inline const QString& getProductionStr(ProductionStat stat) const {return _prods[static_cast<int>(stat)];}

    inline void     setClass(Class globalClass) {_class = globalClass;}
    inline void     setAllianceClass(AllianceClass allianceClass) {_allianceClass = allianceClass;}
    inline void     setScrapRate(int scrapRate) {_scrapRate = scrapRate;}
    inline void     setUniverseSpecific(UniverseSpecifics universeSpecific, int speed) {_universeSpecifics[universeSpecific] = speed;}
    inline void     setOfficerActivated(Officers officer, bool activated) {_officers[officer] = activated;}
    inline void     setConversionRate(Ressources<float> conversionRate) {_conversionRates = conversionRate;}
    inline void     setConversionRateAt(RessourceType type, float conversionRate) {_conversionRates.setRessource(type, conversionRate);}
    inline void     setResearchLevel(ResearchType researchType, int level) {_levelResearch[researchType] = level;}
    inline void     setSpecies(Species species, int level) {_levelSpecies[species] = level;}

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
    void            readAllPlanetData(const QJsonObject& parent);
    Planet          readPlanetData(const QJsonObject& planetObj);

    void            writeUniverses(QJsonObject& parent);
    void            writeSpecies(QJsonObject& parent);
    void            writeResearches(QJsonObject& parent);
    void            writeClassData(QJsonObject& parent);
    void            writeOfficersData(QJsonObject& parent);
    void            writeConversionData(QJsonObject& parent);
    QJsonObject     writePlanetData(const Planet& planet);

    QJsonDocument   generateGameDataJson();
    QString         getSavePath();

    bool            loadInitSave();
    bool            loadSave(const QString& path);

private:
    PlayerManager();
    ~PlayerManager() = default;

    PlayerManager(const PlayerManager&) = delete;
    PlayerManager& operator=(const PlayerManager&) = delete;

    Class                               _class {Class::None};
    AllianceClass                       _allianceClass {AllianceClass::None};
    int                                 _scrapRate {35};
    int                                 _labsLevel {0};

    std::map<UniverseSpecifics, int>    _universeSpecifics;
    std::map<Officers, bool>            _officers;
    Ressources<float>                   _conversionRates;
    std::map<ResearchType, int>         _levelResearch;
    std::map<Species, int>              _levelSpecies;
    std::vector<Planet>                 _planets;
    Planet                              _planifAstro {};
    std::unordered_map<Species, Planet> _planificationFDV;
    std::map<BonusLifeForm, float>      _lifeFormBonuses;

    std::map<ProductionStat, Ressources<float>> _productionStats;
    std::map<ProductionStatPercent, Ressources<float>> _productionStatsPercent;
};
