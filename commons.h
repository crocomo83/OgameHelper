#pragma once

#include <QString>
#include <QStringList>

#include <map>

enum RessourceType
{
    Metal,
    Cristal,
    Deut,
    Energy,
    Antimatter
};
inline static const QStringList ressourceToString = {"Métal", "Cristal", "Deut", "Energy", "Antimatter"};

template <typename T>
struct Ressources {
    T metal     = 0.f;
    T cristal   = 0.f;
    T deut      = 0.f;
    T energy    = 0.f;
    T antimatter = 0.f;

    Ressources(T metal_ = 0.f, T cristal_ = 0.f,
               T deut_ = 0.f, T energy_ = 0.f, T antimatter_ = 0.f)
        : metal(metal_), cristal(cristal_), deut(deut_), energy(energy_), antimatter(antimatter_) {}

    template <typename U>
    Ressources(const Ressources<U>& other)
        : metal(static_cast<T>(other.metal))
        , cristal(static_cast<T>(other.cristal))
        , deut(static_cast<T>(other.deut))
        , energy(static_cast<T>(other.energy))
        , antimatter(static_cast<T>(other.antimatter))
    {}

    Ressources& operator+=(const Ressources& other) {
        metal   += other.metal;
        cristal += other.cristal;
        deut    += other.deut;
        energy  += other.energy;
        antimatter += other.antimatter;
        return *this;
    }

    Ressources& operator-=(const Ressources& other) {
        metal   -= other.metal;
        cristal -= other.cristal;
        deut    -= other.deut;
        energy  -= other.energy;
        antimatter -= other.antimatter;
        return *this;
    }

    Ressources& operator*=(T scalar) {
        metal   *= scalar;
        cristal *= scalar;
        deut    *= scalar;
        energy  *= scalar;
        antimatter *= scalar;
        return *this;
    }

    Ressources& operator*=(Ressources other) {
        metal   *= other.metal;
        cristal *= other.cristal;
        deut    *= other.deut;
        energy  *= other.energy;
        antimatter *= other.antimatter;
        return *this;
    }

    Ressources& operator/(T diviser) {
        metal   /= diviser;
        cristal /= diviser;
        deut    /= diviser;
        energy  /= diviser;
        antimatter /= diviser;
        return *this;
    }

    friend Ressources operator+(Ressources lhs, const Ressources& rhs) {
        lhs += rhs;
        return lhs;
    }

    friend Ressources operator-(Ressources lhs, const Ressources& rhs) {
        lhs -= rhs;
        return lhs;
    }

    friend Ressources operator*(Ressources lhs, const Ressources& rhs) {
        lhs *= rhs;
        return lhs;
    }

    friend Ressources operator*(Ressources lhs, T scalar) {
        lhs *= scalar;
        return lhs;
    }

    friend Ressources operator*(T scalar, Ressources rhs) {
        rhs *= scalar;
        return rhs;
    }

    bool empty() const
    {
        return metal == 0.0f && cristal == 0.0f && deut == 0.0f && energy == 0.0f && antimatter == 0.0f;
    }

    T getEquivalentDeut(Ressources trade) const {
        return  metal / trade.metal +
                cristal / trade.cristal +
                deut / trade.deut +
                antimatter / trade.antimatter;
    }

    T getRessource(RessourceType type) const
    {
        switch (type)
        {
            case RessourceType::Metal:
                return this->metal;
            case RessourceType::Cristal:
                return this->cristal;
            case RessourceType::Deut:
                return this->deut;
            case RessourceType::Energy:
                return this->energy;
            case RessourceType::Antimatter:
                return this->antimatter;
            default:
                return -1;
        }
    }

    void setRessource(RessourceType type, T value)
    {
        switch (type)
        {
        case RessourceType::Metal:
            this->metal = value;
        case RessourceType::Cristal:
            this->cristal = value;
        case RessourceType::Deut:
            this->deut = value;
        case RessourceType::Energy:
            this->energy = value;
        case RessourceType::Antimatter:
            this->antimatter = value;
        }
    }
};

enum class Species
{
    None,
    Humans,
    Mechs,
    Kaeleshs,
    Roctas,
    Count
};
inline static const QStringList speciesToString = {"Aucune", "Humains", "Mecas", "Kaeleshs", "Roctas"};

enum class BonusLifeFormBuilding{
    Metal,
    Cristal,
    Deut,
    Energy,
    ReducMineCostPercent,
    ReducLifeFormBuildingCostPercent,
    ReducLifeFormBuildingDurationPercent,
    ReducLifeFormResearchCostPercent,
    ReducLifeFormResearchDurationPercent,
    Count
};

inline static const std::map<BonusLifeFormBuilding, QString> bonusLifeFormBuildingStr =
{
    {BonusLifeFormBuilding::Metal, "metalBonus"},
    {BonusLifeFormBuilding::Cristal, "cristalBonus"},
    {BonusLifeFormBuilding::Deut, "deutBonus"},
    {BonusLifeFormBuilding::Energy, "energy"},
    {BonusLifeFormBuilding::ReducMineCostPercent, "reducMineCostPercent"},
    {BonusLifeFormBuilding::ReducLifeFormBuildingCostPercent, "reducLifeFormBuildingCostPercent"},
    {BonusLifeFormBuilding::ReducLifeFormBuildingDurationPercent, "reducLifeFormBuildingDurationPercent"},
    {BonusLifeFormBuilding::ReducLifeFormResearchCostPercent, "reducLifeFormResearchCostPercent"},
    {BonusLifeFormBuilding::ReducLifeFormResearchDurationPercent, "reducLifeFormResearchDurationPercent"}
};

struct CommonTech{
    QString name = "";
    Ressources<float> baseCost;
    float increaseFactor = 2.0;

    CommonTech() = default;
    virtual ~CommonTech() = default;
    bool isValid() const {return name != "";}
};

enum class BonusLifeForm
{
    Metal,
    Cristal,
    Deuterium,
    Energy,
    Antimatter,
    HiddenRessources,
    BlackHoleDecrease,
    ExpeditionShipIncrease,
    ExpeditionRessourcesIncrease,
    SpeedToExpedition,
    Fret,
    Phallenge,
    SpeedAllShips,
    SpeedCivilianShips,
    SpeedDiscovery,
    DeuteriumSaving,
    DeutSavingOnRecall,
    LightFighterUpdate,
    HeavyFighterUpdate,
    CruiserUpdate,
    BattleshipUpdate,
    BattlecruiserUpdate,
    BomberUpdate,
    DestroyerUpdate,
    LargeCargoUpdate,
    CrawlerBonus,
    CrawlerEnergySaving,
    RecyclingShipUpdate,
    DefenseBonus,
    TerraformerCostReduction,
    TerraformerDurationReduction,
    DepotCostReduction,
    DepotDurationReduction,
    ResearchDuration,
    LifeFormResearchDuration,
    AstrophysicDuration,
    EnergyResearchCost,
    EnergyDurationCost,
    SpyResearchCost,
    SpyDurationCost,
    WeaponShieldSpyResearchCost,
    WeaponShieldDurationCost,
    ShieldSpyResearchCost,
    ShieldDurationCost,
    ShellResearchCost,
    ShellDurationCost,
    CollectorClass,
    ExploratorClass,
    GeneralClass,
    Count
};

inline static const std::map<BonusLifeForm, QString> bonusLifeFormStr =
{
    {BonusLifeForm::Metal,"metalBonus"},
    {BonusLifeForm::Cristal,"cristalBonus"},
    {BonusLifeForm::Deuterium,"deuteriumBonus"},
    {BonusLifeForm::Energy,"energyBonus"},
    {BonusLifeForm::Antimatter,"antimatter"},
    {BonusLifeForm::HiddenRessources,"ressourceHiddenBonus"},
    {BonusLifeForm::BlackHoleDecrease,"blackHoleDecrease"},
    {BonusLifeForm::ExpeditionShipIncrease,"expeditionShipIncrease"},
    {BonusLifeForm::ExpeditionRessourcesIncrease,"expeditionRessourcesIncrease"},
    {BonusLifeForm::SpeedToExpedition,"speedToExpedition"},
    {BonusLifeForm::Fret,"fret"},
    {BonusLifeForm::Phallenge,"phallenge"},
    {BonusLifeForm::SpeedAllShips,"speedAllShip"},
    {BonusLifeForm::SpeedCivilianShips,"speedCiviliansShip"},
    {BonusLifeForm::SpeedDiscovery,"speedDiscovery"},
    {BonusLifeForm::DeuteriumSaving,"deuteriumSaving"},
    {BonusLifeForm::DeutSavingOnRecall,"deutSavingOnRecall"},
    {BonusLifeForm::LightFighterUpdate,"lightFighterFullBonus"},
    {BonusLifeForm::HeavyFighterUpdate,"heavyFighterFullBonus"},
    {BonusLifeForm::CruiserUpdate,"cruiserFullBonus"},
    {BonusLifeForm::BattleshipUpdate,"battleshipFullBonus"},
    {BonusLifeForm::BattlecruiserUpdate,"battlecruiserFullBonus"},
    {BonusLifeForm::BomberUpdate,"bomberFullBonus"},
    {BonusLifeForm::DestroyerUpdate,"destroyerFullBonus"},
    {BonusLifeForm::LargeCargoUpdate,"largeCargoFullBonus"},
    {BonusLifeForm::CrawlerBonus,"crawlerBonus"},
    {BonusLifeForm::CrawlerEnergySaving,"crawlerEnergySaving"},
    {BonusLifeForm::RecyclingShipUpdate,"recyclingShipFullBonus"},
    {BonusLifeForm::DefenseBonus,"defenseBonus"},
    {BonusLifeForm::TerraformerCostReduction,"terraformerCostReduction"},
    {BonusLifeForm::TerraformerDurationReduction,"terraformerDurationReduction"},
    {BonusLifeForm::DepotCostReduction,"depotCostReduction"},
    {BonusLifeForm::DepotDurationReduction,"depotDurationReduction"},
    {BonusLifeForm::ResearchDuration,"researchSpeed"},
    {BonusLifeForm::LifeFormResearchDuration,"lifeFormResearchSpeed"},
    {BonusLifeForm::AstrophysicDuration,"astrophysicDurationReduction"},
    {BonusLifeForm::EnergyResearchCost,"energyResearchCostReduction"},
    {BonusLifeForm::EnergyDurationCost,"energyResearchDurationReduction"},
    {BonusLifeForm::SpyResearchCost,"spyResearchCostReduction"},
    {BonusLifeForm::SpyDurationCost,"spyResearchDurationReduction"},
    {BonusLifeForm::WeaponShieldSpyResearchCost,"weaponResearchCostReduction"},
    {BonusLifeForm::WeaponShieldDurationCost,"weaponResearchDurationReduction"},
    {BonusLifeForm::ShieldSpyResearchCost,"shieldResearchCostReduction"},
    {BonusLifeForm::ShieldDurationCost,"shieldResearchDurationReduction"},
    {BonusLifeForm::ShellResearchCost,"shellResearchCostReduction"},
    {BonusLifeForm::ShellDurationCost,"shellResearchDurationReduction"},
    {BonusLifeForm::CollectorClass,"collectorBonus"},
    {BonusLifeForm::ExploratorClass,"exploratorClass"},
    {BonusLifeForm::GeneralClass,"generalClass"}
};

struct LifeFormBuilding : public CommonTech{
    Species species;
    std::map<BonusLifeFormBuilding, double> bonuses;
    int durationBase = 0;
    float durationFactor = 1.0f;
};

struct LifeFormTech : public CommonTech{
    Species species;
    std::map<BonusLifeForm, double> bonuses;
    int durationBase = 0;
    float durationFactor = 1.0f;
};

struct Unit{
    QString name = "";
    Ressources<int> cost;
    int speed;
};

enum class UniverseSpecifics
{
    EconnomicSpeed,
    ResearchBoost,
    Count
};
inline static const QStringList universeSpecificsToString = {"Vitesse économique", "Boost recherches"};

enum class TechType
{
    None,
    CommonBuilding,
    CommonResearch,
    HumanBuilding,
    MechBuilding,
    KaeleshBuilding,
    RoctasBuilding,
    HumanResearch,
    MechResearch,
    KaeleshResearch,
    RoctasResearch,
    Count
};
inline static const QStringList techTypeToString = {"None", "CommonBuilding", "CommonResearch", "HumanBuilding", "MechBuilding",
    "KaeleshBuilding", "RoctasBuilding", "HumanResearch", "MechResearch", "KaeleshResearch", "RoctasResearch"};
inline static const std::map<Species, TechType> speciesToTechLifeForm =
    {
        {Species::None,     TechType::None},
        {Species::Humans,   TechType::HumanResearch},
        {Species::Mechs,    TechType::MechResearch},
        {Species::Kaeleshs, TechType::KaeleshResearch},
        {Species::Roctas,   TechType::RoctasResearch}
};

inline static const std::map<Species, TechType> speciesToBuildingLifeForm =
    {
        {Species::None,     TechType::None},
        {Species::Humans,   TechType::HumanBuilding},
        {Species::Mechs,    TechType::MechBuilding},
        {Species::Kaeleshs, TechType::KaeleshBuilding},
        {Species::Roctas,   TechType::RoctasBuilding}
};

enum class CommonBuildingType
{
    MineMetal,
    MineCristal,
    MineDeut,
    CentraleSolaire,
    CentraleFusion,
    HangarMetal,
    HangarCristal,
    HangarDeut,
    UsineRobots,
    ChantierSpatial,
    LaboRecherche,
    DepotRavitaillement,
    SiloMissiles,
    UsineNanite,
    Terraformer,
    DockSpatial,
    Count
};

enum class ResearchType
{
    Energie,
    Laser,
    Ion,
    Hyperespace,
    Plasma,
    PropCombusion,
    PropImpulsion,
    PropHyperespace,
    Espionnage,
    Ordinateur,
    Astrophysique,
    Reseau,
    Graviton,
    Armes,
    Boucliers,
    Coque,
    Count
};

enum class MovingUnitType
{
    LightFighter,
    HeavyFighter,
    Cruiser,
    Battleship,
    Battlecruiser,
    Bomber,
    Destroyer,
    Deathstar,
    Reaper,
    Pathfinder,
    SmallCargo,
    LargeCargo,
    ColonyShip,
    Recycler,
    EspionageProbe,
    Count
};

enum class FixUnitType
{
    SolarSatellite,
    Crawler,
    MissileLauncher,
    LightLaser,
    HeavyLaser,
    Gauss,
    Ion,
    Plasma,
    SmallShield,
    LargeShield,
    InterceptionMissile,
    InterplanetMissile,
    Count
};

enum class Class
{
    None,
    Collector,
    Explorer,
    General,
    Count
};
inline static const QStringList classToString = {"Aucune", "Collecteur", "Explorateur", "Général"};

enum class AllianceClass
{
    None,
    Merchand,
    Finder,
    Warrior,
    Count
};
inline static const QStringList allianceClassToString = {"Aucune", "Marchand", "Guerrier", "Chercheur"};

enum class Officers
{
    Concil,
    Amiral,
    Engeneer,
    Geolog,
    Technocrat,
    Count
};
inline static const QStringList officerToString = {"Conseil", "Amiral", "Ingénieur", "Géologue", "Technocrate"};

struct PlanetPosition
{
    int galaxy {1};
    int solarSystem {1};
    int position {1};

    PlanetPosition() = default;
    PlanetPosition(int g, int s, int p) : galaxy(g), solarSystem(s), position(p) {}
};
