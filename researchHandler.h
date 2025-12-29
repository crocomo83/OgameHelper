#pragma once

#include "commons.h"

#include <map>
#include <QString>

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
    Coque
};

class ResearchHandler
{
public:
    struct Research{
        QString name;
        Cost baseCost;
        int currentLevel;
        float increaseFactor;
    };
public:
    ResearchHandler();

    int getNumberResearch() const;
    const Research& getResearch(ResearchType type) const;

private:
    std::map<ResearchType, Research> _researches;
};
