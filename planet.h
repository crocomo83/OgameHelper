#pragma once

#include "commons.h"

#include <QString>

class Planet
{
public:
    Planet(QString name, Species species = Species::None);

    void addTech(TechType type, int level = 0);

    const QString& getName() const;
    Species getSpecies() const;
    int getNumberTech(TechType type) const;
    int getTechLevel(TechType type, int index) const;

    void setName(QString name);
    void setSpecies(Species species);
    void setTechLevel(TechType type, int index, int level);

private:
    QString                                 _name;
    Species                                 _species;
    std::map<TechType, std::vector<int>>    _techs;
};
