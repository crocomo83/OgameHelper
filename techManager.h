#pragma once

#include "commons.h"

#include <map>
#include <QString>

class QJsonObject;
class TechManager
{
public:
    static TechManager& instance();

    bool loadConfig(TechType techType, const QString& path);
    bool loadUnits(const QString& path);

    int getNumberTechs(TechType techType) const;
    const CommonTech* getTech(TechType techType, int type) const;
    int getProductionMine(CommonBuildingType mineType, int level, float bonus, int temperatureMax = 0) const;
    Ressources<float> getCost(TechType techType, int indexTech, int level) const;
    float getBaseTime(TechType techType, int indexTech, int level) const;
    bool isLifeFormBuilding(TechType techType) const;
    bool isLifeFormResearch(TechType techType) const;
    Species getAssociatedSpecies(TechType techType) const;
    TechType getBuildingTech(Species species) const;
    TechType getResearchTech(Species species) const;
    const Unit& getUnit(UnitType type) const;

    std::map<BonusLifeFormBuilding, double> extractBonusesBuilding(const QJsonObject& obj) const;
    std::map<BonusLifeForm, double> extractBonuses(const QJsonObject& obj) const;

private:
    TechManager();
    ~TechManager() = default;

    TechManager(const TechManager&) = delete;
    TechManager& operator=(const TechManager&) = delete;

    std::map<TechType, std::map<int, std::unique_ptr<CommonTech>>> _techs;
    std::map<UnitType, Unit> _units;
};
