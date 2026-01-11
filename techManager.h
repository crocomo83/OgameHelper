#pragma once

#include "commons.h"

#include <map>
#include <QString>

class TechManager
{
public:
    static TechManager& instance();

    bool loadConfig(TechType techType, QString path);



    int getNumberTechs(TechType techType) const;
    const CommonTech& getTech(TechType techType, int type) const;

private:
    TechManager();
    ~TechManager() = default;

    TechManager(const TechManager&) = delete;
    TechManager& operator=(const TechManager&) = delete;

    std::map<TechType, std::map<int, CommonTech>> _techs;
};
