#pragma once

#include <QString>
#include <QStringList>

#include <map>

struct Ressources {
    float metal   = 0;
    float cristal = 0;
    float deut    = 0;
    float energy  = 0;

    Ressources(float metal_ = 0, float cristal_ = 0, float deut_ = 0, float energy_ = 0)
        : metal(metal_), cristal(cristal_), deut(deut_), energy(energy_) {}

    Ressources& operator+=(const Ressources& other) {
        metal   += other.metal;
        cristal += other.cristal;
        deut    += other.deut;
        energy  += other.energy;
        return *this;
    }

    Ressources& operator-=(const Ressources& other) {
        metal   -= other.metal;
        cristal -= other.cristal;
        deut    -= other.deut;
        energy  -= other.energy;
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
};

struct CommonTech{
    QString name = "";
    Ressources baseCost;
    float increaseFactor = 2.0;

    bool isValid() const {return name != "";}
};

enum class UniverseSpecifics
{
    EconnomicSpeed,
    ResearchBoost
};
inline static const QStringList universeSpecificsToString = {"Vitesse économique", "Boost recherches"};

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

enum class TechType
{
    CommonBuilding,
    CommonResearch,
    LifeFormBuilding,
    LifeFormResearch,
    Count
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

enum class Class
{
    None,
    Collector,
    Explorer,
    General,
    Count
};
inline static const QStringList classToString = {"Aucune", "Collecteur", "Général", "Explorateur"};

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

enum class ConversionRate
{
    Metal,
    Cristal,
    Deut,
    Count
};
inline static const QStringList conversionRateToString = {"Métal", "Cristal", "Deut"};
