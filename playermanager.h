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
        Council,
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

    enum class PlanificationType{
        NewColony,
        LevelUpFDV,
        ChangeSpecies,
        Count
    };

    const QStringList _planificationStr = {
        "NewColony",
        "LevelUpFDV",
        "ChangeSpecies"
    };

    struct Planification{
        PlanificationType type;
        Planet planet;

        Planification() : type(PlanificationType::NewColony), planet() {}
        Planification(PlanificationType _type) : type(_type), planet() {}
        Planification(PlanificationType _type, const Planet& _planet) : type(_type), planet(_planet) {}
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

    bool                    loadSave(const QString& path);

    inline int              getNumberPlanets() const {return _planets.size();}
    inline int              getNumberPlanifs() const {return _planifPlanets.size();}
    inline const QString&   getPlanetName(int index) const {return _planets.at(index).getName();}
    inline float            getLifeFormBonus(BonusLifeForm bonus) const {return _lifeFormBonuses.at(bonus);}
    inline Ressources<float> getScrapValue() const {return _scrapValue;}

    Planet*                 getPlanet(int index);
    Planification*          getPlanif(int index);
    Ressources<float>       getLifeFormProdBonus() const;
    float                   getResearchTime(int indexTech, int level) const;
    int                     getShip(MovingUnitType unitType) const;
    int                     getScrapShip(MovingUnitType unitType) const;
    inline float            getDiscoveryPerDay() const {return _discoveryPerDay;}
    inline int              getDeutConsumption() const {return _deutConsumption;}
    inline int              getPositionDiscovery() const {return _positionDiscovery;}
    inline int              getTempBonusRessources() const {return _additionalBonusRessources;}
    inline int              getGlobalLabLevel() const {return _labsLevel;}

    void                    refresh();
    void                    computeLifeFormResearch();
    void                    computeProduction();
    void                    computeLabsLevel();
    void                    computeConversionRate();
    void                    computeScrapValue();
    void                    computePoints();

    inline const Ressources<float>& getProduction(ProductionStat stat) const {return _productionStats.at(stat);}
    inline const Ressources<float>& getProductionPercent(ProductionStatPercent stat) const {return _productionStatsPercent.at(stat);}
    inline const QString& getProductionStr(ProductionStat stat) const {return _prods[static_cast<int>(stat)];}
    inline const Ressources<int>& getAllInvestedRessources() const {return _allInvestedPoints;}
    inline const Ressources<int>& getUsefullInvestedRessources() const {return _usefullInvestedPoints;}
    inline const Ressources<int>& getFleetRessources() const {return _fleetPoints;}

    inline void     setShip(MovingUnitType unitType, int value) {_fleet[unitType] = value;}
    inline void     setScrapShip(MovingUnitType unitType, int value) {_scrapFleet[unitType] = value;}
    inline void     setDiscoveryPerDay(float value) {_discoveryPerDay = value;}
    inline void     setDeutConsumption(int deut) {_deutConsumption = deut;}
    inline void     setPositionDiscovery(int position) {_positionDiscovery = position;}
    inline void     setTempBonusRessources(int bonus) {_additionalBonusRessources = bonus;}

    void            addPlanet(const QString& name, const PlanetPosition& position, int temperature, Species species = Species::None);
    inline void     removePlanet(int indexPlanet) {_planetToRemove = indexPlanet;}
    inline void     addPlanif(PlanificationType type, const Planet& planet) {_planifPlanets.push_back(Planification(type, planet));}
    inline void     removePlanif(int indexPlanif) {_planifToRemove = indexPlanif;}
    void            duplicatePlanet();

    bool            saveGameData(const QString &path);
    bool            clear();

private:
    void            readAllPlanetData(const QJsonObject& parent);
    void            readFleetData(const QJsonObject& parent);
    Planet          readPlanetData(const QJsonObject& planetObj);
    void            readDiscoveries(const QJsonObject& parent);

    void            writeAllPlanetData(QJsonObject& parent);
    void            writeFleetData(QJsonObject& parent);
    QJsonObject     writePlanetData(const Planet& planet);
    void            writeDiscoveries(QJsonObject& parent);

    QJsonDocument   generateGameDataJson();

private:
    PlayerManager();
    ~PlayerManager() = default;

    PlayerManager(const PlayerManager&) = delete;
    PlayerManager& operator=(const PlayerManager&) = delete;

    std::vector<Planet>                 _planets;
    std::vector<Planification>          _planifPlanets;
    std::optional<int>                  _planetToRemove;
    std::optional<int>                  _planifToRemove;
    std::map<BonusLifeForm, float>      _lifeFormBonuses;
    std::map<MovingUnitType, int>       _fleet;
    std::map<MovingUnitType, int>       _scrapFleet;
    Ressources<float>                   _scrapValue;
    int                                 _labsLevel {0};

    std::map<ProductionStat, Ressources<float>> _productionStats;
    std::map<ProductionStatPercent, Ressources<float>> _productionStatsPercent;
    Ressources<int>                      _allInvestedPoints;
    Ressources<int>                      _usefullInvestedPoints;
    Ressources<int>                      _fleetPoints;

    // Discoveries
    float _discoveryPerDay {0.0f};
    int _deutConsumption {0};
    int _positionDiscovery {1};
    int _additionalBonusRessources {0};
};
