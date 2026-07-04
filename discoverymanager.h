#pragma once

#include "commons.h"

#include <chrono>

class DiscoveryManager
{
public:
    enum class DiscoveryType
    {
        Error,
        Void,
        Metal,
        Cristal,
        Deut,
        Fleat,
        Antimatter,
        Late,
        Advance,
        Combat,
        Blackhole,
        Merchand,
        Item,
        Count
    };

    QStringList listTypeDiscovery = {
        "void",
        "metal",
        "cristal",
        "deut",
        "fleat",
        "antimatter",
        "late",
        "advance",
        "combat",
        "blackhole",
        "merchand",
        "item"
    };
    std::map<DiscoveryManager::DiscoveryType, QString> discoveryTypeToString =
    {
        {DiscoveryType::Void,       listTypeDiscovery.at(0)},
        {DiscoveryType::Metal,      listTypeDiscovery.at(1)},
        {DiscoveryType::Cristal,    listTypeDiscovery.at(2)},
        {DiscoveryType::Deut,       listTypeDiscovery.at(3)},
        {DiscoveryType::Fleat,      listTypeDiscovery.at(4)},
        {DiscoveryType::Antimatter, listTypeDiscovery.at(5)},
        {DiscoveryType::Late,       listTypeDiscovery.at(6)},
        {DiscoveryType::Advance,    listTypeDiscovery.at(7)},
        {DiscoveryType::Combat,     listTypeDiscovery.at(8)},
        {DiscoveryType::Blackhole,  listTypeDiscovery.at(9)},
        {DiscoveryType::Merchand,   listTypeDiscovery.at(10)},
        {DiscoveryType::Item,       listTypeDiscovery.at(11)}
    };

    struct Discovery
    {
        DiscoveryType type;
        Ressources mean;
        int count = 0;
        std::vector<RessourceType> availableRessources;
        float bonusFactor = 1.0f;

        Discovery() : Discovery(DiscoveryType::Error) {}
        Discovery(DiscoveryType type_) : type(type_) {}

        void add(Ressources value, int number = 1)
        {
            if (number == 0) return;
            count += number;
            mean = (mean * (float)(count - number) + value / bonusFactor) / (float)count;
        }

        bool hasRessource(RessourceType type)
        {
            return std::find(availableRessources.begin(), availableRessources.end(), type) != availableRessources.end();
        }
    };

    // Basic without any bonus
    struct SummaryPerDay
    {
        Ressources meanRessourceFound;
        Ressources meanShipFound;
        Ressources meanLost;
        Ressources globalMean;
    };

public:
    static DiscoveryManager& instance();

    QString getSavePath();
    DiscoveryType extractDiscoveryType(const QString& str) const;
    void addDiscover(DiscoveryType type, Ressources value, int count = 1);
    void loadInit();
    bool loadSave(QString path);
    void loadBonusFactor(float ressourcesBonus, float shipBonus);
    bool save(QString path);
    void refresh();
    void computeRentability();
    QString getTypeStrList(DiscoveryType type) const;
    Discovery getDiscovery(DiscoveryType type) const;
    float getDiscoveryPerDay() const;
    int getDeutConsumption() const;
    int getPositionDiscovery() const;
    int getTempBonusRessources() const;
    const SummaryPerDay& getSummary() const;
    inline float getTimeToPos16() const {return timeToPos16; }
    void setDiscoveryPerDay(float value);
    void setDeutConsumption(int deut);
    void setPositionDiscovery(int position);
    void setTempBonusRessources(int bonus);

    Ressources computeReductionTimeDiscovery(float bonusSpeedPercent);

private:
    DiscoveryManager();
    ~DiscoveryManager();

    DiscoveryManager(const DiscoveryManager&) = delete;
    DiscoveryManager& operator=(const DiscoveryManager&) = delete;

    void computeDiscoverySpeed();
    float computeTimeToPos16(float bonusSpeedPercent);

private:
    std::map<DiscoveryManager::DiscoveryType, Discovery> dataDiscoveries;
    float discoveryPerDay = 0.0f;
    int deutConsumption = 0;
    int positionDiscovery = 1;
    int additionalBonusRessources = 0;

    float _bonusSpeedPercent;
    float timeToPos16;

    SummaryPerDay summary;
};

