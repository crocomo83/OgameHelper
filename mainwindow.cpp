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
    Q_ASSERT(ui->planificationTab);

    _overviewTable = ui->overviewTab;
    _rentaTable = ui->renta;
    _generalTable = ui->commonTab;
    _planetTable = ui->planetsTab;
    _discoveryTab = ui->discoveryTab;
    _planificationTab = ui->planificationTab;

    _rentaTable->setRowCount(NUMBER_RENTA_MAX + 1);
    initTable(_overviewTable);
    initTable(_generalTable);

    _planificationTab->verticalHeader()->setVisible(false);
    _planificationTab->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _planificationTab->setFrameShape(QFrame::NoFrame);
    _planificationTab->setSelectionMode(QAbstractItemView::NoSelection);
    _planificationTab->setFocusPolicy(Qt::NoFocus);

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
    int planifCol = 0;
    buildPlanificationAstro(_planificationTab, planifCol);
    buildPlanificationFdv(_planificationTab, planifCol);
    buildPlanificationChangeSpecies(_planificationTab, planifCol);

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
    connect(this, &MainWindow::planifChanged, this, &MainWindow::onPlanifChanged);
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

void MainWindow::createPlanetLifeFormChoice(QTableWidget* tableWidget, Planet &planet, int& row, int column)
{
    addLabel(tableWidget, "Forme de vie", row++, column);
    int speciesIndex = static_cast<int>(planet.getSpecies());
    ComboBoxItem* speciesItem = addComboBoxItem(tableWidget, "Choix : ", speciesToString, row++, column, speciesIndex);
    speciesItem->setOnValueChanged([this, &planet](int value) {
        planet.setSpecies(static_cast<Species>(value));
        emit techChanged();
        emit planetsChanged();
        emit planifChanged();
    });
    row++; //blank
}

void MainWindow::createPlanetCommonBuilding(QTableWidget* tableWidget, Planet &planet, int& row, int column)
{
    TechType techType = TechType::CommonBuilding;
    addLabel(tableWidget, techTypeToString[static_cast<int>(techType)] + " : ", row++, column);

    int numberTechs = TechManager::instance().getNumberTechs(techType);
    for (int i = 0; i < numberTechs; ++i)
    {
        int level = planet.getTechLevel(techType, i);
        const CommonTech* tech = TechManager::instance().getTech(techType, i);
        if (tech->isValid())
        {
            Item* buildingItem = addSpinBoxItem(tableWidget, tech->name, row++, column, level);
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
    row++; // blank
}

void MainWindow::createPlanetLifeFormBuilding(QTableWidget* tableWidget, Planet &planet, int& row, int column)
{
    TechType techType = planet.getLifeFormBuilding();

    if (techType == TechType::None) { return;}

    addLabel(tableWidget, techTypeToString[static_cast<int>(techType)] + " : ", row++, column);

    int numberTechs = TechManager::instance().getNumberTechs(techType);
    for (int i = 0; i < numberTechs; ++i)
    {
        int level = planet.getTechLevel(techType, i);
        const CommonTech* tech = TechManager::instance().getTech(techType, i);
        if (tech->isValid())
        {
            Item* buildingItem = addSpinBoxItem(tableWidget, tech->name, row++, column, level);
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
    row++; // blank
}

void MainWindow::createPlanetLifeFormResearches(QTableWidget* tableWidget, Planet &planet, int& row, int column)
{
    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberLifeFormReseach; ++i)
    {
        Species speciesChoice = planet.getChoiceLifeFormResearch(i);
        int levelInit = planet.getLevelLifeFormResearch(speciesChoice, i);
        LifeFormSelectItem* item = addLifeFormSelectItem(tableWidget, row++, column, speciesChoice, levelInit);
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
            row++;
    }
}

void MainWindow::createPlanetDefenses(QTableWidget* tableWidget, FixUnitType unitType, Planet &planet, int& row, int column)
{
    Unit unit = TechManager::instance().getFixUnit(unitType);
    int initValue = planet.getDefense(unitType);
    Item* item = addSpinBoxItem(tableWidget, unit.name, row++, column, initValue, 0, 999999999);
    item->setOnValueChanged([this, &planet, unitType](int value) {
        planet.setDefense(unitType, value);
        emit techChanged();
        emit rentaChanged();
    });
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

        addLabel(tableWidget, levelUp._name + " : " + QString::number(levelUp._levelToUpdate), i+1, 0);
        addLabel(tableWidget, utils::timeToString(levelUp.timeToRecover), i+1, 2);
        addLabel(tableWidget, utils::timeToString(levelUp.timeToCompleteDay), i+1, 3);
    }

    for (int i = 0; i < PlayerManager::instance().getNumberPlanets(); ++i)
    {
        const Planet& planet = PlayerManager::instance().getPlanet(i);
        bool state = RentabilityManager::instance().getFilterPlanet(i);
        CheckBoxItem* planetFilterItem = addCheckBoxItem(tableWidget, planet.getName(), i + 1, 5, state);
        planetFilterItem->addOnValueChanged([this, i](bool value) {
            RentabilityManager::instance().setFilterPlanet(i, value);
            emit rentaChanged();
        });
    }

    for (int i = 0; i < static_cast<int>(RentabilityManager::TypeFilter::Count); ++i)
    {
        RentabilityManager::TypeFilter typeFilter = static_cast<RentabilityManager::TypeFilter>(i);
        bool state = RentabilityManager::instance().getFilterType(typeFilter);

        CheckBoxItem* typeFilterItem = addCheckBoxItem(tableWidget, RentabilityManager::typeFilterString.at(i), i + 1, 6, state);
        typeFilterItem->addOnValueChanged([this, typeFilter](bool value) {
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
    int currentRow = 0;
    addLabel(tableWidget, "Classes : ", currentRow++, column);

    int indexClass = static_cast<int>(PlayerManager::instance().getClass());
    ComboBoxItem* classBox = addComboBoxItem(tableWidget, "Classe", classToString, currentRow++, column, indexClass);
    classBox->setOnValueChanged([this](int index) {
        PlayerManager::instance().setClass(static_cast<Class>(index));
        emit techChanged();
    });

    int indexAllianceClass = static_cast<int>(PlayerManager::instance().getAllianceClass());
    ComboBoxItem* alliClassBox = addComboBoxItem(tableWidget, "Classe alli", allianceClassToString, currentRow++, column, indexAllianceClass);
    alliClassBox->setOnValueChanged([this](int index) {
        PlayerManager::instance().setAllianceClass(static_cast<AllianceClass>(index));
        emit techChanged();
    });

    // Officers
    currentRow++;
    addLabel(tableWidget, "Officiers : ", currentRow++, column);

    std::map<Officers, CheckBoxItem*> officerWidgets;
    for (int i = 0; i < static_cast<int>(Officers::Count); ++i)
    {
        Officers officer = static_cast<Officers>(i);
        bool initValue = PlayerManager::instance().getOfficerValue(officer);
        CheckBoxItem* officerCheckBox = addCheckBoxItem(tableWidget, officerToString[i], currentRow++, column, initValue);
        officerCheckBox->addOnValueChanged([this, officer](bool value) {
            PlayerManager::instance().setOfficerActivated(officer, value);
            emit techChanged();
        });
        officerWidgets[officer] = officerCheckBox;
    }

    auto hideOrShowOfficers = [this, officerWidgets](bool value) {
        for (int i = 0; i < static_cast<int>(Officers::Count); ++i)
        {
            Officers officer = static_cast<Officers>(i);
            if (officer != Officers::Concil)
            {
                if (value)
                {
                    officerWidgets.at(officer)->setValue(true);
                    PlayerManager::instance().setOfficerActivated(officer, true);
                }
                officerWidgets.at(officer)->setEnabled(!value);
            }
        }
    };

    CheckBoxItem* concilWidget = officerWidgets[Officers::Concil];
    if (concilWidget->getValue())
    {
        hideOrShowOfficers(true);
    }
    concilWidget->addOnValueChanged(hideOrShowOfficers);
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
    currentRow++;

    // Life form choice
    createPlanetLifeFormChoice(tableWidget, planet, currentRow, column);

    // Buildings
    createPlanetCommonBuilding(tableWidget, planet, currentRow, column);
    createPlanetLifeFormBuilding(tableWidget, planet, currentRow, column);

    // Life form research
    addLabel(tableWidget, "Life form research : ", currentRow++, column);
    createPlanetLifeFormResearches(tableWidget, planet, currentRow, column);

    // Units
    addLabel(tableWidget, "Units : ", currentRow++, column);
    for (int i = 0; i < static_cast<int>(FixUnitType::Count); ++i)
    {
        FixUnitType unitType = static_cast<FixUnitType>(i);
        createPlanetDefenses(tableWidget, unitType, planet, currentRow, column);
    }
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
    int row = 2;
    tableWidget->setCellWidget(row, 0, addButton);
    connect(addButton, &QPushButton::clicked, this, [tableWidget, row]() {
        for (int i = 1; i < static_cast<int>(DiscoveryManager::DiscoveryType::Count); ++i)
        {
            int indexRow = i + row;

            DiscoveryManager::DiscoveryType type = static_cast<DiscoveryManager::DiscoveryType>(i);
            Ressources<float> ressources;

            QSpinBox* number = qobject_cast<QSpinBox*>(tableWidget->cellWidget(indexRow, 2));
            QLineEdit* metalValue = qobject_cast<QLineEdit*>(tableWidget->cellWidget(indexRow, 3));
            QLineEdit* cristalValue = qobject_cast<QLineEdit*>(tableWidget->cellWidget(indexRow, 4));
            QLineEdit* deutValue = qobject_cast<QLineEdit*>(tableWidget->cellWidget(indexRow, 5));
            QLineEdit* amValue = qobject_cast<QLineEdit*>(tableWidget->cellWidget(indexRow, 6));

            if (number && metalValue && cristalValue && deutValue && amValue)
            {
                ressources.metal = metalValue->text().toFloat() * 1000000.f;
                ressources.cristal = cristalValue->text().toFloat() * 1000000.f;
                ressources.deut = deutValue->text().toFloat() * 1000000.f;
                ressources.antimatter = amValue->text().toFloat() * 1000.f;

                DiscoveryManager::instance().addDiscover(type, ressources, number->value());
            }
            else
            {
                qWarning() << "Invalid widgets for validate discoveries";
            }

            number->clear();
            metalValue->clear();
            cristalValue->clear();
            deutValue->clear();
            amValue->clear();
        }
    });
}

void MainWindow::buildPlanificationAstro(QTableWidget* tableWidget, int& column)
{
    Planet& planet = PlayerManager::instance().getPlanifAstro();
    int currentRow = 0;

    addLabel(tableWidget, "Planif astro :", currentRow++, column);

    // Position
    PositionItem* positionItem = addPositionItem(tableWidget, currentRow++, column, planet.getPosition());
    positionItem->setOnValueChanged([this, &planet](int g, int s, int p) {
        planet.setPosition(PlanetPosition(g, s, p));
        emit planifChanged();
    });

    // Temperature
    Item* temperatureItem = addSpinBoxItem(tableWidget, "Temp max : ", currentRow++, column, planet.getTemperatureMax(), -99, 99);
    temperatureItem->setOnValueChanged([this, &planet](int value){
        planet.setTemperatureMax(value);
        emit planifChanged();
    });
    currentRow++;

    // Life form choice
    createPlanetLifeFormChoice(tableWidget, planet, currentRow, column);

    // Common buildings
    createPlanetCommonBuilding(tableWidget, planet, currentRow, column);

    // Life form buildings
    createPlanetLifeFormBuilding(tableWidget, planet, currentRow, column);

    // Life form research
    addLabel(tableWidget, "Life form research : ", currentRow++, column);
    createPlanetLifeFormResearches(tableWidget, planet, currentRow, column);

    // Units
    addLabel(tableWidget, "Units : ", currentRow++, column);
    for (int i = 0; i < static_cast<int>(FixUnitType::Count); ++i)
    {
        FixUnitType unitType = static_cast<FixUnitType>(i);
        createPlanetDefenses(tableWidget, unitType, planet, currentRow, column);
    }

    column++;
}

void MainWindow::buildPlanificationFdv(QTableWidget* tableWidget, int& column)
{
    std::unordered_set<Species> availableSpecies = getAllAvailableSpecies();

    for (const Species& currentSpecies : availableSpecies) {
        int currentRow = 0;
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

        currentRow++;

        // Life form buildings
        addLabel(tableWidget, "Life form buildings : ", currentRow++, column);
        createPlanetLifeFormBuilding(tableWidget, planifPlanet, currentRow, column);

        // Life form research
        addLabel(tableWidget, "Life form research : ", currentRow++, column);
        createPlanetLifeFormResearches(tableWidget, planifPlanet, currentRow, column);

        column++;
    }
}

void MainWindow::buildPlanificationChangeSpecies(QTableWidget* tableWidget, int& column)
{
    int currentRow = 0;
    Species currentSpecies = PlayerManager::instance().getPlanifChgtSpecies().getSpecies();
    ComboBoxItem* speciesChoice = addComboBoxItem(tableWidget, "Chgt species : ", speciesToString, currentRow++, column, static_cast<int>(currentSpecies));
    speciesChoice->setOnValueChanged([this](int value) {
        PlayerManager::instance().setChgtSpecies(static_cast<Species>(value));
        emit planifChanged();
    });

    if (currentSpecies == Species::None) return;

    Planet& planifPlanet = PlayerManager::instance().getPlanifPlanet(currentSpecies);

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

    currentRow++;

    // Life form buildings
    addLabel(tableWidget, "Life form buildings : ", currentRow++, column);
    createPlanetLifeFormBuilding(tableWidget, planifPlanet, currentRow, column);

    // Life form research
    addLabel(tableWidget, "New life form research : ", currentRow++, column);
    createPlanetLifeFormResearches(tableWidget, planifPlanet, currentRow, column);

    column++;
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

void MainWindow::onPlanifChanged()
{
    _planificationTab->clear();

    int planifCol = 0;
    buildPlanificationAstro(_planificationTab, planifCol);
    buildPlanificationFdv(_planificationTab, planifCol);
    buildPlanificationChangeSpecies(_planificationTab, planifCol);
}
