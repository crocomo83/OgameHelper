#pragma once

#include "commons.h"

#include <map>
#include <QString>

class QJsonObject;
class TechManager
{
public:
    static TechManager& instance();

    bool loadConfig(TechType techType, QString path);

    int getNumberTechs(TechType techType) const;
    const CommonTech* getTech(TechType techType, int type) const;
    int getProductionMine(CommonBuildingType mineType, int level, float bonus, int temperatureMax = 0) const;
    Ressources getCost(TechType techType, int indexTech, int level);
    bool isLifeFormResearch(TechType techType) const;
    std::map<BonusLifeForm, double> extractBonuses(const QJsonObject& obj) const;

private:
    TechManager();
    ~TechManager() = default;

    TechManager(const TechManager&) = delete;
    TechManager& operator=(const TechManager&) = delete;

    std::map<TechType, std::map<int, std::unique_ptr<CommonTech>>> _techs;
};
