#include "planet.h"

Planet::Planet(QString name, Species species)
    : _name(name)
    , _species(species)
{

}

void Planet::addTech(TechType type, int level)
{
    _techs[type].push_back(level);
}

const QString& Planet::getName() const
{
    return _name;
}

Species Planet::getSpecies() const
{
    return _species;
}

int Planet::getNumberTech(TechType type) const
{
    return _techs.at(type).size();
}

int Planet::getTechLevel(TechType type, int index) const
{
    return _techs.at(type).at(index);
}

void Planet::setName(QString name)
{
    _name = name;
}

void Planet::setSpecies(Species species)
{
    _species = species;
}

void Planet::setTechLevel(TechType type, int index, int level)
{
    _techs[type][index] = level;
}
