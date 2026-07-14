#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "utils.h"
#include "techManager.h"
#include "playerManager.h"
#include "rentabilityManager.h"
#include "discoverymanager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

#include <chrono>

static constexpr int NUMBER_RENTA_MAX = 100;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Q_ASSERT(ui->overviewTab);
    Q_ASSERT(ui->renta);
    Q_ASSERT(ui->commonTab);
    Q_ASSERT(ui->planetsTab);
    Q_ASSERT(ui->discoveryTab);
    Q_ASSERT(ui->lvlUpFdvTab);

    _overviewTable = ui->overviewTab;
    _rentaTable = ui->renta;
    _generalTable = ui->commonTab;
    _planetTable = ui->planetsTab;
    _discoveryTab = ui->discoveryTab;
    _lvlUpFdvTab = ui->lvlUpFdvTab;

    _rentaTable->setRowCount(NUMBER_RENTA_MAX + 1);
    initTable(_overviewTable);
    initTable(_generalTable);

    _lvlUpFdvTab->verticalHeader()->setVisible(false);
    _lvlUpFdvTab->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _lvlUpFdvTab->setFrameShape(QFrame::NoFrame);
    _lvlUpFdvTab->setSelectionMode(QAbstractItemView::NoSelection);
    _lvlUpFdvTab->setFocusPolicy(Qt::NoFocus);

    _planetTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    _planetTable->horizontalHeader()->setStretchLastSection(false);
    _planetTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    _planetTable->verticalHeader()->setStretchLastSection(false);
    _planetTable->setFrameShape(QFrame::NoFrame);
    _planetTable->setSelectionMode(QAbstractItemView::NoSelection);
    _planetTable->setFocusPolicy(Qt::NoFocus);

    buildGeneralImputs(_generalTable, 0);
    buildResearchImputs(_generalTable, 1);
    buildSpecialisationImputs(_generalTable, 2);
    buildTradeImputs(_generalTable, 3);
    buildDiscoveryImputs(_discoveryTab);
    buildLevelUpLifeForm(_lvlUpFdvTab);

    onPlanetsChanged();
    onTechChanged();

    buildResumeOutputs(_overviewTable);
    buildRentaOutputs(_rentaTable);

    QPushButton* saveButton = ui->saveButton;
    connect(saveButton, &QPushButton::clicked, this, []() {
        PlayerManager::instance().saveGameData();
    });

    QPushButton* addPlanet = ui->addPlanet;
    connect(addPlanet, &QPushButton::clicked, this, [this]() {
        PlayerManager::instance().addPlanet("Colonie", {1, 1, 1}, 0, Species::None);
        emit planetsChanged();
    });

    QPushButton* duplicatePlanet = ui->duplicatePlanet;
    connect(duplicatePlanet, &QPushButton::clicked, this, [this]() {
        PlayerManager::instance().duplicatePlanet();
        emit planetsChanged();
    });

    connect(this, &MainWindow::planetsChanged, this, &MainWindow::onPlanetsChanged);
    connect(this, &MainWindow::techChanged, this, &MainWindow::onTechChanged);
    connect(this, &MainWindow::rentaChanged, this, &MainWindow::onRentaChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initTable(QTableWidget* tableWidget)
{
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setFrameShape(QFrame::NoFrame);
    tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    tableWidget->setFocusPolicy(Qt::NoFocus);
}

QLabel* MainWindow::addLabel(QTableWidget* tableWidget, QString str, int row, int column)
{
    QLabel* label = new QLabel(str);
    tableWidget->setCellWidget(row, column, label);
    return label;
}

Item* MainWindow::addSpinBoxItem(QTableWidget* tableWidget, QString str, int row, int column, int defaultValue, int minValue, int maxValue)
{
    Item *item = new Item(str);
    item->setMinValue(minValue);
    item->setMaxValue(maxValue);
    item->setValue(defaultValue);
    tableWidget->setCellWidget(row, column, item);
    return item;
}

DoubleSpinBoxItem* MainWindow::addDoubleSpinBoxItem(QTableWidget* tableWidget, QString str, int row, int column, double defaultValue, double minValue, double maxValue)
{
    DoubleSpinBoxItem* doubleSpinBox = new DoubleSpinBoxItem(str);
    doubleSpinBox->setMinValue(minValue);
    doubleSpinBox->setMaxValue(maxValue);
    doubleSpinBox->setValue(defaultValue);
    doubleSpinBox->setLocale(QLocale::C);
    tableWidget->setCellWidget(row, column, doubleSpinBox);
    return doubleSpinBox;
}

CheckBoxItem* MainWindow::addCheckBoxItem(QTableWidget* tableWidget, QString str, int row, int column, bool defaultValue)
{
    CheckBoxItem* checkBoxItem = new CheckBoxItem(str);
    checkBoxItem->setValue(defaultValue);
    tableWidget->setCellWidget(row, column, checkBoxItem);
    return checkBoxItem;
}

ComboBoxItem* MainWindow::addComboBoxItem(QTableWidget* tableWidget, QString str, QStringList names, int row, int column, int defaultValue)
{
    ComboBoxItem* comboBoxItem = new ComboBoxItem(str, names);
    if (!names.empty())
    {
        comboBoxItem->setValue(defaultValue);
    }
    tableWidget->setCellWidget(row, column, comboBoxItem);
    return comboBoxItem;
}

PositionItem* MainWindow::addPositionItem(QTableWidget* tableWidget, int row, int column, const PlanetPosition& planetPosition)
{
    PositionItem* positionItem = new PositionItem(this);
    positionItem->setValue(planetPosition.galaxy, planetPosition.solarSystem, planetPosition.position);
    tableWidget->setCellWidget(row, column, positionItem);
    return positionItem;
}

LifeFormSelectItem* MainWindow::addLifeFormSelectItem(QTableWidget* tableWidget, int row, int column, Species species, int level)
{
    LifeFormSelectItem* lifeFormItem = new LifeFormSelectItem(speciesToString, this);
    lifeFormItem->setIndex(static_cast<int>(species));
    lifeFormItem->setLevel(level);
    lifeFormItem->setEnabled(species != Species::None);
    tableWidget->setCellWidget(row, column, lifeFormItem);
    return lifeFormItem;
}

std::unordered_set<Species> MainWindow::getAllAvailableSpecies()
{
    std::unordered_set<Species> result;
    for (int i = 0; i < PlayerManager::instance().getNumberPlanets(); ++i)
    {
        const Planet& planet = PlayerManager::instance().getPlanet(i);
        Species species = planet.getSpecies();
        if (species != Species::None)
        {
            result.insert(species);
        }
    }
    return result;
}

QLabel* MainWindow::createPlanetsLabel(std::vector<int> indexPlanets)
{
    QString displayedName = "";
    if (indexPlanets.size() > 1)
    {
        displayedName = "[" + QString::number(indexPlanets.size()) + "]";
    }
    else if (indexPlanets.size() == 1)
    {
        const Planet& planet = PlayerManager::instance().getPlanet(indexPlanets.at(0));
        displayedName = planet.getName();
    }

    QLabel* labelPlanets = new QLabel(displayedName);
    if (indexPlanets.size() > 1)
    {
        QString listPlanets;
        for (int i = 0; i < indexPlanets.size(); ++i)
        {
            int indexPlanet = indexPlanets.at(i);
            const Planet& planet = PlayerManager::instance().getPlanet(indexPlanet);
            listPlanets += planet.getName();
            if (i != indexPlanets.size() - 1)
            {
                listPlanets += '\n';
            }
        }
        labelPlanets->setToolTip(listPlanets);
    }
    return labelPlanets;
}

void MainWindow::buildResumeOutputs(QTableWidget* tableWidget)
{
    tableWidget->clear();
    int index = 0;

    addLabel(tableWidget, "Productions", index, 0);
    addLabel(tableWidget, "Métal", index, 1);
    addLabel(tableWidget, "Cristal", index, 2);
    addLabel(tableWidget, "Deuterium", index, 3);
    index++;

    const Ressources<float>& productionStat = PlayerManager::instance().getProduction(PlayerManager::ProductionStat::Total);
    addLabel(tableWidget, "Mines", index, 0);
    addLabel(tableWidget, utils::formatRessource(productionStat.metal), index, 1);
    addLabel(tableWidget, utils::formatRessource(productionStat.cristal), index, 2);
    addLabel(tableWidget, utils::formatRessource(productionStat.deut), index, 3);
    index++;

    const Ressources<float>& discoveryRessources = DiscoveryManager::instance().getSummary().globalMean;
    addLabel(tableWidget, "Discovery", index, 0);
    addLabel(tableWidget, utils::formatRessource(discoveryRessources.metal), index, 1);
    addLabel(tableWidget, utils::formatRessource(discoveryRessources.cristal), index, 2);
    addLabel(tableWidget, utils::formatRessource(discoveryRessources.deut), index, 3);
    index++;

    const Ressources<float>& totalRessources = productionStat + discoveryRessources;
    addLabel(tableWidget, "Total", index, 0);
    addLabel(tableWidget, utils::formatRessource(totalRessources.metal), index, 1);
    addLabel(tableWidget, utils::formatRessource(totalRessources.cristal), index, 2);
    addLabel(tableWidget, utils::formatRessource(totalRessources.deut), index, 3);
    index++;
}

void MainWindow::buildRentaOutputs(QTableWidget* tableWidget)
{
    tableWidget->clear();

    addLabel(tableWidget, "A augmenter", 0, 0);
    addLabel(tableWidget, "Planète", 0, 1);
    addLabel(tableWidget, "Temps de recouvrement", 0, 2);
    addLabel(tableWidget, "Temps de construction", 0, 3);
    addLabel(tableWidget, "Planet filter", 0, 5);
    addLabel(tableWidget, "Type filter", 0, 6);

    int nb = RentabilityManager::instance().refresh();
    for (int i = 0; i < std::min(NUMBER_RENTA_MAX, nb); ++i)
    {
        const RentabilityManager::LevelUp& levelUp = RentabilityManager::instance().getLevelUp(i);

        QLabel* labelPlanets = createPlanetsLabel(levelUp.indexPlanets);
        tableWidget->setCellWidget(i+1, 1, labelPlanets);

        addLabel(tableWidget, levelUp.name + " : " + QString::number(levelUp.levelToUpdate), i+1, 0);
        addLabel(tableWidget, levelUp.timeToRecoverStr, i+1, 2);
        addLabel(tableWidget, QString::number(levelUp.timeToCompleteDay), i+1, 3);
    }

    for (int i = 0; i < PlayerManager::instance().getNumberPlanets(); ++i)
    {
        const Planet& planet = PlayerManager::instance().getPlanet(i);
        bool state = RentabilityManager::instance().getFilterPlanet(i);
        CheckBoxItem* planetFilterItem = addCheckBoxItem(tableWidget, planet.getName(), i + 1, 5, state);
        planetFilterItem->setOnValueChanged([this, i](bool value) {
            RentabilityManager::instance().setFilterPlanet(i, value);
            emit rentaChanged();
        });
    }

    for (int i = 0; i < static_cast<int>(RentabilityManager::TypeFilter::Count); ++i)
    {
        RentabilityManager::TypeFilter typeFilter = static_cast<RentabilityManager::TypeFilter>(i);
        bool state = RentabilityManager::instance().getFilterType(typeFilter);

        CheckBoxItem* typeFilterItem = addCheckBoxItem(tableWidget, RentabilityManager::typeFilterString.at(i), i + 1, 6, state);
        typeFilterItem->setOnValueChanged([this, typeFilter](bool value) {
            RentabilityManager::instance().setFilterType(typeFilter, value);
            emit rentaChanged();
        });
    }
}

void MainWindow::buildGeneralImputs(QTableWidget* tableWidget, int column)
{
    addLabel(tableWidget, "General : ", 0, column);

    int speed = PlayerManager::instance().getUniverseSpecific(UniverseSpecifics::EconnomicSpeed);
    Item* universeSpeed = addSpinBoxItem(tableWidget, "Vitesse univers", 1, column, 1, speed);
    universeSpeed->setOnValueChanged([](int value) {
        PlayerManager::instance().setUniverseSpecific(UniverseSpecifics::EconnomicSpeed, value);
    });

    int researchSpeedValue = PlayerManager::instance().getUniverseSpecific(UniverseSpecifics::ResearchBoost);
    Item* researchSpeed = addSpinBoxItem(tableWidget, "Vitesse recherches", 2, column, 1, researchSpeedValue);
    researchSpeed->setOnValueChanged([](int value) {
        PlayerManager::instance().setUniverseSpecific(UniverseSpecifics::ResearchBoost, value);
    });

    addLabel(tableWidget, "Niveaux races : ", 4, column);
    for (int i = 1; i < static_cast<int>(Species::Count); ++i) {
        int levelRace = PlayerManager::instance().getSpecies(static_cast<Species>(i));
        Item* speciesLevel = addSpinBoxItem(tableWidget, "Niveau " + speciesToString[i], i+5, column, levelRace, 0, 100);
        speciesLevel->setOnValueChanged([this, i](int value) {
            PlayerManager::instance().setSpecies(static_cast<Species>(i), value);
            emit techChanged();
        });
    }
}

void MainWindow::buildResearchImputs(QTableWidget* tableWidget, int column)
{
    addLabel(tableWidget, "Recherches : ", 0, column);

    int numberOfResearch = TechManager::instance().getNumberTechs(TechType::CommonResearch);

    for (int i = 0; i < numberOfResearch; ++i) {
        const CommonTech* research = TechManager::instance().getTech(TechType::CommonResearch, i);

        ResearchType type = static_cast<ResearchType>(i);

        int level = PlayerManager::instance().getResearchLevel(type);
        Item* researchItem = addSpinBoxItem(tableWidget, research->name, i+1, column, level);
        researchItem->setOnValueChanged([this, column, i, type](int value) {
            PlayerManager::instance().setResearchLevel(type, value);
            emit techChanged();
        });
    }
}

void MainWindow::buildSpecialisationImputs(QTableWidget* tableWidget, int column)
{
    addLabel(tableWidget, "Classes : ", 0, column);

    int indexClass = static_cast<int>(PlayerManager::instance().getClass());
    ComboBoxItem* classBox = addComboBoxItem(tableWidget, "Classe", classToString, 1, column, indexClass);
    classBox->setOnValueChanged([this](int index) {
        PlayerManager::instance().setClass(static_cast<Class>(index));
        emit techChanged();
    });

    int indexAllianceClass = static_cast<int>(PlayerManager::instance().getAllianceClass());
    ComboBoxItem* alliClassBox = addComboBoxItem(tableWidget, "Classe alli", allianceClassToString, 2, column, indexAllianceClass);
    alliClassBox->setOnValueChanged([this](int index) {
        PlayerManager::instance().setAllianceClass(static_cast<AllianceClass>(index));
        emit techChanged();
    });

    addLabel(tableWidget, "Officiers : ", 4, column);

    for (int i = 0; i < static_cast<int>(Officers::Count); ++i)
    {
        Officers officer = static_cast<Officers>(i);
        bool initValue = PlayerManager::instance().getOfficerValue(officer);
        CheckBoxItem* officerCheckBox = addCheckBoxItem(tableWidget, officerToString[i], i+5, column, initValue);
        officerCheckBox->setOnValueChanged([this, officer](bool value) {
            PlayerManager::instance().setOfficerActivated(officer, value);
            emit techChanged();
        });
    }
}

void MainWindow::buildTradeImputs(QTableWidget* tableWidget, int column)
{
    addLabel(tableWidget, "Taux de conversion : ", 0, column);

    double metalRateValue = PlayerManager::instance().getConversionRate().metal;
    DoubleSpinBoxItem* metalRate = addDoubleSpinBoxItem(tableWidget, "Métal", 1, column, metalRateValue, 1.0);
    metalRate->setOnValueChanged([this](double value) {
        PlayerManager::instance().setConversionRateAt(RessourceType::Metal, value);
        emit techChanged();
    });

    double cristalRateValue = PlayerManager::instance().getConversionRate().cristal;
    DoubleSpinBoxItem* cristalRate = addDoubleSpinBoxItem(tableWidget, "Cristal", 2, column, cristalRateValue, 1.0);
    cristalRate->setOnValueChanged([this](double value) {
        PlayerManager::instance().setConversionRateAt(RessourceType::Cristal, value);
        emit techChanged();
    });

    double deutRateValue = PlayerManager::instance().getConversionRate().deut;
    DoubleSpinBoxItem* deutRate = addDoubleSpinBoxItem(tableWidget, "Deut", 3, column, deutRateValue, 1.0);
    deutRate->setOnValueChanged([this](double value) {
        PlayerManager::instance().setConversionRateAt(RessourceType::Deut, value);
        emit techChanged();
    });

    int rate = PlayerManager::instance().getScrapRate();
    Item* scrapRate = addSpinBoxItem(tableWidget, "Taux ferraille (%)", 5, column, rate, 35, 100);
    scrapRate->setOnValueChanged([this](int value) {
        PlayerManager::instance().setScrapRate(value);
        emit techChanged();
    });
}

void MainWindow::buildPlanetImputs(QTableWidget* tableWidget, int column)
{
    int currentRow = 0;
    Planet& planet = PlayerManager::instance().getPlanet(column);

    // Name
    QLineEdit* lineEdit = new QLineEdit(planet.getName(), tableWidget);
    connect(lineEdit, &QLineEdit::textChanged, this,
            [&planet](const QString &text) {
                planet.setName(text);
    });
    tableWidget->setCellWidget(currentRow++, column, lineEdit);

    // Position
    PositionItem* positionItem = addPositionItem(tableWidget, currentRow++, column, planet.getPosition());
    positionItem->setOnValueChanged([this, &planet](int g, int s, int p) {
        planet.setPosition(PlanetPosition(g, s, p));
        emit techChanged();
    });

    // Temperature
    Item* temperatureItem = addSpinBoxItem(tableWidget, "Temp max : ", currentRow++, column, planet.getTemperatureMax(), -99, 99);
    temperatureItem->setOnValueChanged([this, &planet](int value){
        planet.setTemperatureMax(value);
        emit techChanged();
    });

    // Life form
    currentRow++;
    addLabel(tableWidget, "Forme de vie", currentRow++, column);
    int speciesIndex = static_cast<int>(planet.getSpecies());
    ComboBoxItem* speciesItem = addComboBoxItem(tableWidget, "Choix : ", speciesToString, currentRow++, column, speciesIndex);
    speciesItem->setOnValueChanged([this, &planet](int value) {
        planet.setSpecies(static_cast<Species>(value));
        emit techChanged();
        emit planetsChanged();
    });

    currentRow++; //blank

    // Buildings
    std::vector<TechType> techTypes = planet.getAvailableBuildings();
    for (TechType techType : techTypes)
    {
        if (techType == TechType::None) { continue;}

        addLabel(tableWidget, techTypeToString[static_cast<int>(techType)] + " : ", currentRow++, column);

        int numberTechs = TechManager::instance().getNumberTechs(techType);
        for (int i = 0; i < numberTechs; ++i)
        {
            int level = planet.getTechLevel(techType, i);
            const CommonTech* tech = TechManager::instance().getTech(techType, i);
            if (tech->isValid())
            {
                Item* buildingItem = addSpinBoxItem(tableWidget, tech->name, currentRow++, column, level);
                buildingItem->setOnValueChanged([this, techType, &planet, i](int value) {
                    planet.setTechLevel(techType, i, value);
                    emit techChanged();
                });
            }
            else
            {
                qWarning() << "Tech not found : " << i;
            }
        }
        currentRow++; // blank
    }

    // Life form research
    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberLifeFormReseach; ++i)
    {
        Species speciesChoice = planet.getChoiceLifeFormResearch(i);
        int levelInit = planet.getLevelLifeFormResearch(speciesChoice, i);
        LifeFormSelectItem* item = addLifeFormSelectItem(tableWidget, currentRow++, column, speciesChoice, levelInit);
        item->setOnIndexChanged([this, &planet, i, item](int value) {
            Species speciesSelected = static_cast<Species>(value);
            planet.setChoiceLifeFormResearch(i, speciesSelected);
            item->setEnabled(speciesSelected != Species::None);
            item->setLevel(speciesSelected != Species::None ? planet.getLevelLifeFormResearch(speciesSelected, i) : 0);
            emit techChanged();
        });
        item->setOnLevelChanged([this, &planet, i](int level) {
            Species speciesSelected = planet.getChoiceLifeFormResearch(i);
            planet.setLevelLifeFormResearch(speciesSelected, i, level);
            emit techChanged();
        });

        if ((i+1)%6 == 0)
            currentRow++;
    }

    QString label = "Foreuses (max=" + QString::number(planet.getMaxActiveCrawler()) + ")";
    Item* crawlerItem = addSpinBoxItem(tableWidget, label, currentRow++, column, planet.getCrawlerNumber(), 0, 9999);
    crawlerItem->setOnValueChanged([this, &planet](int value) {
        planet.setCrawlerNumber(value);
        emit techChanged();
    });
}

void MainWindow::buildDiscoveryImputs(QTableWidget* tableWidget)
{
    DiscoveryManager::instance().refresh();

    // Line 1
    float expePerDay = DiscoveryManager::instance().getDiscoveryPerDay();
    DoubleSpinBoxItem* expeItem = addDoubleSpinBoxItem(tableWidget, "Expé/j : ", 0, 0, expePerDay, 0, 999.9);
    expeItem->setOnValueChanged([this](double value) {
        DiscoveryManager::instance().setDiscoveryPerDay((float)value);
    });

    float bonusTemp = DiscoveryManager::instance().getTempBonusRessources();
    Item* tempBonus = addSpinBoxItem(tableWidget, "Bonus R : ", 0, 1, bonusTemp, 0, 99);
    tempBonus->setOnValueChanged([this](int value) {
        float bonusRessources = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::ExpeditionRessourcesIncrease);
        float bonusFleat = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::ExpeditionShipIncrease);
        float bonusAM = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::Antimatter);
        float bonusDiscover = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::ExploratorClass);
        DiscoveryManager::instance().setTempBonusRessources(value);
        DiscoveryManager::instance().loadBonusFactor(bonusRessources, bonusFleat, bonusAM, bonusDiscover);
    });

    float deutConsumption = DiscoveryManager::instance().getDeutConsumption();
    Item* deutConso = addSpinBoxItem(tableWidget, "Deut cons : ", 0, 2, deutConsumption, 0, 99999);
    deutConso->setOnValueChanged([this](int value) {
        DiscoveryManager::instance().setDeutConsumption(value);
    });

    float positionDiscovery = DiscoveryManager::instance().getPositionDiscovery();
    Item* posDiscoveryItem = addSpinBoxItem(tableWidget, "Pos discovery : ", 0, 3, positionDiscovery, 1, 15);
    posDiscoveryItem->setOnValueChanged([this](int value) {
        DiscoveryManager::instance().setPositionDiscovery(value);
    });

    // Line 2
    float bonusRessources = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::ExpeditionRessourcesIncrease);
    addLabel(tableWidget, "Ressources : " + QString::number(bonusRessources, 'f', 1) + "%", 1, 0);

    float bonusFleat = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::ExpeditionShipIncrease);
    addLabel(tableWidget, "Fleat : " + QString::number(bonusFleat, 'f', 1) + "%", 1, 1);

    float bonusAM = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::Antimatter);
    addLabel(tableWidget, "AM : " + QString::number(bonusAM, 'f', 1) + "%", 1, 2);

    float bonusDiscover = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::ExploratorClass);
    addLabel(tableWidget, "Class bonus : " + QString::number(bonusDiscover, 'f', 1) + "%", 1, 3);

    DiscoveryManager::instance().loadBonusFactor(bonusRessources, bonusFleat, bonusAM, bonusDiscover);

    const int secondsToPos16 = std::round(DiscoveryManager::instance().getTimeToPos16());
    int min = secondsToPos16 / 60;
    int sec = secondsToPos16 % 60;
    addLabel(tableWidget, "Min : " + QString::number(min) + ", Sec : " + QString::number(sec), 1, 4);

    // Add discover
    int index = 0;
    int indexCol = 2;
    addLabel(tableWidget, "Ajout : ",           indexCol, index++);
    addLabel(tableWidget, "Type : ",            indexCol, index++);
    addLabel(tableWidget, "Number : ",          indexCol, index++);
    addLabel(tableWidget, "Metal (M) : ",       indexCol, index++);
    addLabel(tableWidget, "Cristal (M) : ",     indexCol, index++);
    addLabel(tableWidget, "Deut (M) : ",        indexCol, index++);
    addLabel(tableWidget, "AM (milliers) : ",   indexCol, index++);

    for (int i = 1; i < static_cast<int>(DiscoveryManager::DiscoveryType::Count); ++i)
    {
        DiscoveryManager::DiscoveryType type = static_cast<DiscoveryManager::DiscoveryType>(i);
        QString strType = DiscoveryManager::instance().getTypeStrList(type);
        addLabel(tableWidget, strType, i + indexCol, 1);

        tableWidget->setCellWidget(i + indexCol, 2, new QSpinBox());

        DiscoveryManager::Discovery discovery = DiscoveryManager::instance().getDiscovery(type);

        QDoubleValidator* validator = new QDoubleValidator(0.0, 1000.0, 3, this);
        validator->setNotation(QDoubleValidator::StandardNotation);
        validator->setLocale(QLocale::C);

        QLineEdit* metalValue = new QLineEdit();
        metalValue->setValidator(validator);
        metalValue->setEnabled(discovery.hasRessource(RessourceType::Metal));
        tableWidget->setCellWidget(i + indexCol, 3, metalValue);

        QLineEdit* cristalValue = new QLineEdit();
        cristalValue->setValidator(validator);
        cristalValue->setEnabled(discovery.hasRessource(RessourceType::Cristal));
        tableWidget->setCellWidget(i + indexCol, 4, cristalValue);

        QLineEdit* deutValue = new QLineEdit();
        deutValue->setValidator(validator);
        deutValue->setEnabled(discovery.hasRessource(RessourceType::Deut));
        tableWidget->setCellWidget(i + indexCol, 5, deutValue);

        QLineEdit* amValue = new QLineEdit();
        amValue->setValidator(validator);
        amValue->setEnabled(discovery.hasRessource(RessourceType::Antimatter));
        tableWidget->setCellWidget(i + indexCol, 6, amValue);
    }

    QPushButton* addButton = new QPushButton("Ajouter");
    tableWidget->setCellWidget(2, 0, addButton);
    connect(addButton, &QPushButton::clicked, this, [tableWidget]() {
        for (int i = 1; i < static_cast<int>(DiscoveryManager::DiscoveryType::Count); ++i)
        {
            DiscoveryManager::DiscoveryType type = static_cast<DiscoveryManager::DiscoveryType>(i);
            Ressources<float> ressources;

            QSpinBox* number = static_cast<QSpinBox*>(tableWidget->cellWidget(i+1, 2));
            QLineEdit* metalValue = static_cast<QLineEdit*>(tableWidget->cellWidget(i+1, 3));
            QLineEdit* cristalValue = static_cast<QLineEdit*>(tableWidget->cellWidget(i+1, 4));
            QLineEdit* deutValue = static_cast<QLineEdit*>(tableWidget->cellWidget(i+1, 5));
            QLineEdit* amValue = static_cast<QLineEdit*>(tableWidget->cellWidget(i+1, 6));

            ressources.metal = metalValue->text().toFloat() * 1000000;
            ressources.cristal = cristalValue->text().toFloat() * 1000000;
            ressources.deut = deutValue->text().toFloat() * 1000000;
            ressources.antimatter = amValue->text().toFloat() * 1000;

            DiscoveryManager::instance().addDiscover(type, ressources, number->value());

            number->clear();
            metalValue->clear();
            cristalValue->clear();
            deutValue->clear();
            amValue->clear();
        }
    });
}

void MainWindow::buildLevelUpLifeForm(QTableWidget* tableWidget)
{
    int column = 0;
    std::unordered_set<Species> availableSpecies = getAllAvailableSpecies();

    for (const Species& currentSpecies : availableSpecies) {
        int currentRow = 0;
        TechType techType = TechManager::instance().getBuildingTech(currentSpecies);
        Planet& planifPlanet = PlayerManager::instance().getPlanifPlanet(currentSpecies);

        addLabel(tableWidget, "Planification " + speciesToString.at(static_cast<int>(currentSpecies)) + " : ", currentRow++, column);

        // Common buildings
        addLabel(tableWidget, "Common buildings : ", currentRow++, column);

        int indexRobots = static_cast<int>(CommonBuildingType::UsineRobots);
        int baseLevelRobots = planifPlanet.getTechLevel(TechType::CommonBuilding, indexRobots);
        Item* robotItem = addSpinBoxItem(tableWidget, "Robots", currentRow++, column, baseLevelRobots);
        robotItem->setOnValueChanged([this, &planifPlanet, indexRobots](int value) {
            planifPlanet.setTechLevel(TechType::CommonBuilding, indexRobots, value);
            emit techChanged();
        });

        int indexNanites = static_cast<int>(CommonBuildingType::UsineNanite);
        int baseLevelNanites = planifPlanet.getTechLevel(TechType::CommonBuilding, indexNanites);
        Item* nanitesItem = addSpinBoxItem(tableWidget, "Nanites", currentRow++, column, baseLevelNanites);
        nanitesItem->setOnValueChanged([this, &planifPlanet, indexNanites](int value) {
            planifPlanet.setTechLevel(TechType::CommonBuilding, indexNanites, value);
            emit techChanged();
        });

        // Life form buildings
        currentRow++;
        addLabel(tableWidget, "Life form buildings : ", currentRow++, column);

        int numberTechs = TechManager::instance().getNumberTechs(techType);
        for (int i = 0; i < numberTechs; ++i)
        {
            const CommonTech* tech = TechManager::instance().getTech(techType, i);
            if (tech->isValid())
            {
                int baseLevel = planifPlanet.getTechLevel(techType, i);
                Item* buildingItem = addSpinBoxItem(tableWidget, tech->name, currentRow++, column, baseLevel);
                buildingItem->setOnValueChanged([this, &planifPlanet, techType, i](int value) {
                    planifPlanet.setTechLevel(techType, i, value);
                    emit techChanged();
                });
            }
            else
            {
                qWarning() << "Tech not found : " << i;
            }
        }

        // Life form research
        addLabel(tableWidget, "Life form research : ", currentRow++, column);
        int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
        for (int i = 0; i < numberLifeFormReseach; ++i)
        {
            Species speciesChoice = planifPlanet.getChoiceLifeFormResearch(i);
            int levelInit = planifPlanet.getLevelLifeFormResearch(speciesChoice, i);
            LifeFormSelectItem* item = addLifeFormSelectItem(tableWidget, currentRow++, column, speciesChoice, levelInit);
            item->setOnIndexChanged([this, &planifPlanet, i, item](int value) {
                Species speciesSelected = static_cast<Species>(value);
                planifPlanet.setChoiceLifeFormResearch(i, speciesSelected);
                item->setEnabled(speciesSelected != Species::None);
                item->setLevel(speciesSelected != Species::None ? planifPlanet.getLevelLifeFormResearch(speciesSelected, i) : 0);
                emit techChanged();
            });
            item->setOnLevelChanged([this, &planifPlanet, i](int level) {
                Species speciesSelected = planifPlanet.getChoiceLifeFormResearch(i);
                planifPlanet.setLevelLifeFormResearch(speciesSelected, i, level);
                emit techChanged();
            });

            if ((i+1)%6 == 0)
                currentRow++;
        }
        column++;
    }

}

void MainWindow::onPlanetsChanged()
{
    _planetTable->clear();

    int numberPlanet = PlayerManager::instance().getNumberPlanets();
    _planetTable->setColumnCount(numberPlanet);

    for (int i = 0; i < numberPlanet; ++i)
    {
        buildPlanetImputs(_planetTable, i);
    }

    for (int col = 0; col < _planetTable->columnCount(); ++col) {
        _planetTable->setColumnWidth(col, 160); // width in pixel
    }
}

void MainWindow::onTechChanged()
{
    int numberPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < numberPlanet; ++i)
    {
        Planet& planet = PlayerManager::instance().getPlanet(i);
        planet.refresh();
    }

    PlayerManager::instance().refresh();
    DiscoveryManager::instance().refresh();

    buildResumeOutputs(_overviewTable);
    buildRentaOutputs(_rentaTable);
    buildDiscoveryImputs(_discoveryTab);
}

void MainWindow::onRentaChanged()
{
    buildRentaOutputs(_rentaTable);
}
