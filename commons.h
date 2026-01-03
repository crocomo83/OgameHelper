#pragma once

#include <QString>
#include <QStringList>

#include <map>

struct Cost{
    int metal   = 0;
    int cristal = 0;
    int deut    = 0;
    int energy  = 0;
};

struct CommonTech{
    QString name = "";
    Cost baseCost;
    int currentLevel = 0;
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
inline static const QStringList speciesToString = {"None", "Humains", "Mecas", "Kaeleshs", "Roctas"};

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
