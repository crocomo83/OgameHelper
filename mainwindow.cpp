#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "utils.h"
#include "techManager.h"
#include "generalManager.h"
#include "playerManager.h"
#include "rentabilityManager.h"
#include "discoverymanager.h"
#include "planetitem.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QMessageBox>

#include <chrono>

static constexpr int NUMBER_RENTA_MAX = 100;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _settings("config.ini", QSettings::IniFormat)
{
    ui->setupUi(this);

    Q_ASSERT(ui->overviewTab);
    Q_ASSERT(ui->renta);
    Q_ASSERT(ui->commonTab);
    Q_ASSERT(ui->planetsTab);
    Q_ASSERT(ui->discoveryTab);
    Q_ASSERT(ui->planificationTab);
    Q_ASSERT(ui->rentabiltyType);

    _overviewTable      = ui->overviewTab;
    _rentaTable         = ui->renta;
    _generalTable       = ui->commonTab;
    _planetTable        = ui->planetsTab;
    _discoveryTab       = ui->discoveryTab;
    _planificationTab   = ui->planificationTab;
    _fleetTab           = ui->fleetTab;
    _rentabilityType    = ui->rentabiltyType;
    _planetType         = ui->planetType;
    _unitProd           = ui->unitProd;

    _rentaTable->setRowCount(NUMBER_RENTA_MAX + 1);
    _rentaTable->setColumnWidth(0, 140);

    initTableScrollable(_overviewTable);
    _overviewTable->setColumnWidth(0, 150);

    initTable(_generalTable);
    initTableScrollable(_planificationTab);
    initTableScrollable(_planetTable);

    _fleetTab->setColumnWidth(0, 250);

    QPushButton* addPlanet = ui->addPlanet;
    connect(addPlanet, &QPushButton::clicked, this, [this]() {
        PlayerManager::instance().addPlanet("Colonie", {1, 1, 1}, 0, Species::None);
        RentabilityManager::instance().planetsAdded();
        emit planetsChanged();
    });

    QPushButton* duplicatePlanet = ui->duplicatePlanet;
    connect(duplicatePlanet, &QPushButton::clicked, this, [this]() {
        PlayerManager::instance().duplicatePlanet();
        RentabilityManager::instance().planetsAdded();
        emit planetsChanged();
    });

    QComboBox* choicePlanif = ui->choicePlanif;
    choicePlanif->addItems(PlayerManager::instance()._planificationStr);
    static PlayerManager::PlanificationType planifType = PlayerManager::PlanificationType::NewColony;
    connect(choicePlanif, &QComboBox::currentIndexChanged, this, [this](int index) {
        planifType = static_cast<PlayerManager::PlanificationType>(index);
    });

    QPushButton* addPlanif = ui->addPlanif;
    connect(addPlanif, &QPushButton::clicked, this, [this]() {
        PlayerManager::instance().addPlanif(planifType, Planet("New planification"));
        emit planifChanged();
        emit rentaChanged();
    });

    connect(this, &MainWindow::planetsChanged, this, &MainWindow::onPlanetsChanged);
    connect(this, &MainWindow::techChanged, this, &MainWindow::onTechChanged);
    connect(this, &MainWindow::rentaChanged, this, &MainWindow::onRentaChanged);
    connect(this, &MainWindow::planifChanged, this, &MainWindow::onPlanifChanged);
    connect(this, &MainWindow::fleetChanged, this, &MainWindow::onFleetChanged);

    // Menu
    connect(ui->actionNew, &QAction::triggered, this, [this]() {
        GeneralManager::instance().clear();
        PlayerManager::instance().clear();
        saveFilePath = std::nullopt;
        refresh();
        ui->actionSave->setEnabled(false);
    });

    connect(ui->actionLoad, &QAction::triggered, this, &MainWindow::loadDirectory);

    connect(ui->actionSave, &QAction::triggered, this, [this]() {
        if (saveFilePath)
        {
            GeneralManager::instance().saveGameData(*saveFilePath);
            PlayerManager::instance().saveGameData(*saveFilePath);
            DiscoveryManager::instance().save();
        }
    });

    connect(ui->actionSave_as, &QAction::triggered, this, &MainWindow::saveDirectoryAs);

    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);

    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);

    // Load save
    QString lastSavePath = _settings.value("saveFilePath").toString();
    if (!lastSavePath.isEmpty() && QFile::exists(lastSavePath))
    {
        loadDirectoryPath(lastSavePath);
    }
    else
    {
        ui->actionSave->setEnabled(false);
    }

    refreshLabelPlanet();
    refresh();

    connect(_rentabilityType, &QComboBox::currentIndexChanged, this, [this]() {
        RentabilityManager::instance().setRentabilityType(static_cast<RentabilityManager::RentabilityType>(_rentabilityType->currentIndex()));
        MainWindow::buildRentaOutputs();
    });

    connect(_planetType, &QComboBox::currentIndexChanged, this, [this]() {
        MainWindow::buildResumeOutputs();
    });

    connect(_unitProd, &QComboBox::currentIndexChanged, this, [this]() {
        MainWindow::buildResumeOutputs();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}


bool MainWindow::loadDirectory()
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QString directoryPath = QFileDialog::getExistingDirectory(
        this,
        "Load save directory",
        basePath,
        QFileDialog::ShowDirsOnly
        );

    // Vérifier que l'utilisateur n'a pas annulé
    if (!directoryPath.isEmpty())
    {
        return loadDirectoryPath(directoryPath);
    }
    else
    {
        return false;
    }
}

bool MainWindow::saveDirectoryAs()
{
    QString basePath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation
        );

    QString directoryPath = QFileDialog::getExistingDirectory(
        this,
        "Save As",
        basePath,
        QFileDialog::ShowDirsOnly
        );

    if (directoryPath.isEmpty())
        return false;

    saveFilePath = directoryPath;

    if (GeneralManager::instance().saveGameData(*saveFilePath)
        && PlayerManager::instance().saveGameData(*saveFilePath))
    {
        _settings.setValue("saveFilePath", directoryPath);
        return true;
    }

    return false;
}

bool MainWindow::loadDirectoryPath(const QString& path)
{
    bool success = GeneralManager::instance().loadSave(path)
                   && PlayerManager::instance().loadSave(path);
    if (success)
    {
        saveFilePath = path;
        _settings.setValue("saveFilePath", path);

        refresh();
        ui->actionSave->setEnabled(true);
    }
    return success;
}

void MainWindow::about()
{
    QMessageBox::about(
        this,
        "À propos",
        "<h3>Ogame helper</h3>"
        "<p>Version 4.8</p>"
        "<p>Author : Star Lord</p>"
        "<p>The point of this app is to help ogame players to vizualise and optimise their strategy.\
            \n Work in progress, all feedbacks are welcome !</p>"
    );
}

void MainWindow::refresh()
{
    GeneralManager::instance().refresh();
    PlayerManager::instance().refresh();
    DiscoveryManager::instance().refresh();

    buildGeneralImputs(_generalTable, 0);
    buildResearchImputs(_generalTable, 1);
    buildSpecialisationImputs(_generalTable, 2);
    buildTradeImputs(_generalTable, 3);
    buildDiscoveryImputs(_discoveryTab);
    buildFleetImputs(_fleetTab);

    onPlanifChanged();
    onPlanetsChanged();
    onTechChanged();

    buildResumeOutputs();
    buildRentaOutputs();
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

void MainWindow::initTableScrollable(QTableWidget* tableWidget)
{
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableWidget->horizontalHeader()->setStretchLastSection(false);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableWidget->verticalHeader()->setStretchLastSection(false);
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
        const Planet* planet = PlayerManager::instance().getPlanet(i);
        Species species = planet->getSpecies();
        if (species != Species::None)
        {
            result.insert(species);
        }
    }
    return result;
}

void MainWindow::refreshLabelPlanet()
{
    _planetType->clear();
    _planetType->addItem("General");

    int numberPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < numberPlanet; ++i)
    {
        _planetType->addItem(PlayerManager::instance().getPlanetName(i));
    }
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
        const Planet* planet = PlayerManager::instance().getPlanet(indexPlanets.at(0));
        displayedName = planet->getName();
    }

    QLabel* labelPlanets = new QLabel(displayedName);
    if (indexPlanets.size() > 1)
    {
        QString listPlanets;
        for (int i = 0; i < indexPlanets.size(); ++i)
        {
            int indexPlanet = indexPlanets.at(i);
            const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);
            listPlanets += planet->getName();
            if (i != indexPlanets.size() - 1)
            {
                listPlanets += '\n';
            }
        }
        labelPlanets->setToolTip(listPlanets);
    }
    return labelPlanets;
}

void MainWindow::createPlanetLifeFormChoice(QTableWidget* tableWidget, Planet* planet, int& row, int column)
{
    addLabel(tableWidget, "Forme de vie", row++, column);
    int speciesIndex = static_cast<int>(planet->getSpecies());
    ComboBoxItem* speciesItem = addComboBoxItem(tableWidget, "Choix : ", speciesToString, row++, column, speciesIndex);
    speciesItem->setOnValueChanged([this, planet](int value) {
        planet->setSpecies(static_cast<Species>(value));
        emit techChanged();
        emit planetsChanged();
        emit planifChanged();
    });
    row++; //blank
}

void MainWindow::createPlanetCommonBuilding(QTableWidget* tableWidget, Planet* planet, int& row, int column)
{
    TechType techType = TechType::CommonBuilding;
    addLabel(tableWidget, techTypeToString[static_cast<int>(techType)] + " : ", row++, column);

    int numberTechs = TechManager::instance().getNumberTechs(techType);
    for (int i = 0; i < numberTechs; ++i)
    {
        int level = planet->getTechLevel(techType, i);
        const CommonTech* tech = TechManager::instance().getTech(techType, i);
        if (tech->isValid())
        {
            Item* buildingItem = addSpinBoxItem(tableWidget, tech->name, row++, column, level);
            buildingItem->setOnValueChanged([this, techType, planet, i](int value) {
                planet->setTechLevel(techType, i, value);
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

void MainWindow::createPlanetLifeFormBuilding(QTableWidget* tableWidget, Planet* planet, int& row, int column)
{
    TechType techType = planet->getLifeFormBuilding();

    if (techType == TechType::None) { return;}

    addLabel(tableWidget, techTypeToString[static_cast<int>(techType)] + " : ", row++, column);

    int numberTechs = TechManager::instance().getNumberTechs(techType);
    for (int i = 0; i < numberTechs; ++i)
    {
        int level = planet->getTechLevel(techType, i);
        const CommonTech* tech = TechManager::instance().getTech(techType, i);
        if (tech->isValid())
        {
            Item* buildingItem = addSpinBoxItem(tableWidget, tech->name, row++, column, level);
            buildingItem->setOnValueChanged([this, techType, planet, i](int value) {
                planet->setTechLevel(techType, i, value);
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

void MainWindow::createPlanetLifeFormResearches(QTableWidget* tableWidget, Planet* planet, int& row, int column)
{
    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberLifeFormReseach; ++i)
    {
        Species speciesChoice = planet->getChoiceLifeFormResearch(i);
        int levelInit = planet->getLevelLifeFormResearch(speciesChoice, i);
        LifeFormSelectItem* item = addLifeFormSelectItem(tableWidget, row++, column, speciesChoice, levelInit);
        item->setOnIndexChanged([this, planet, i, item](int value) {
            Species speciesSelected = static_cast<Species>(value);
            planet->setChoiceLifeFormResearch(i, speciesSelected);
            item->setEnabled(speciesSelected != Species::None);
            item->setLevel(speciesSelected != Species::None ? planet->getLevelLifeFormResearch(speciesSelected, i) : 0);
            emit techChanged();
        });
        item->setOnLevelChanged([this, planet, i](int level) {
            Species speciesSelected = planet->getChoiceLifeFormResearch(i);
            planet->setLevelLifeFormResearch(speciesSelected, i, level);
            emit techChanged();
        });

        if ((i+1)%6 == 0)
            row++;
    }
}

void MainWindow::createFleetUnit(QTableWidget* tableWidget, MovingUnitType unitType, int& row, int column)
{
    Unit unit = TechManager::instance().getMovingUnit(unitType);
    int initValue = PlayerManager::instance().getShip(unitType);
    Item* item = addSpinBoxItem(tableWidget, unit.name, row++, column, initValue, 0, 999999999);
    item->setOnValueChanged([this, unitType](int value) {
        PlayerManager::instance().setShip(unitType, value);
        emit fleetChanged();
    });
}

void MainWindow::createPlanetDefenses(QTableWidget* tableWidget, FixUnitType unitType, Planet* planet, int& row, int column)
{
    Unit unit = TechManager::instance().getFixUnit(unitType);
    int initValue = planet->getDefense(unitType);
    Item* item = addSpinBoxItem(tableWidget, unit.name, row++, column, initValue, 0, 999999999);
    item->setOnValueChanged([this, planet, unitType](int value) {
        planet->setDefense(unitType, value);
        emit techChanged();
        emit rentaChanged();
    });
}

void MainWindow::buildResumeOutputs()
{
    _overviewTable->clear();

    float factor = 1;
    if (_unitProd->currentText() == "hours")
    {
        factor = 1.0f / 24.0f;
    }
    else if (_unitProd->currentText() == "week")
    {
        factor = 7.0f;
    }

    int currentIndex = _planetType->currentIndex();
    if (currentIndex <= 0)
    {
        buildResumeGeneralOutputs(factor);
    }
    else
    {
        buildResumePlanetOutputs(currentIndex - 1, factor);
    }
}

void MainWindow::buildResumeGeneralOutputs(float factor)
{
    int index = 0;

    addLabel(_overviewTable, "Productions", index, 0);
    addLabel(_overviewTable, "Métal", index, 1);
    addLabel(_overviewTable, "Cristal", index, 2);
    addLabel(_overviewTable, "Deuterium", index, 3);
    index++;

    auto addProdLabel = [this, &index, &factor](const QString& label, const Ressources<int>& prod)
    {
        addLabel(_overviewTable, label, index, 0);
        addLabel(_overviewTable, utils::ressourceToString((int)((float)prod.metal    * factor)), index, 1);
        addLabel(_overviewTable, utils::ressourceToString((int)((float)prod.cristal  * factor)), index, 2);
        addLabel(_overviewTable, utils::ressourceToString((int)((float)prod.deut     * factor)), index, 3);
        index++;
    };

    auto addPointLabel = [this, &index, &factor](const QString& label, const Ressources<int>& points)
    {
        addLabel(_overviewTable, label, index, 0);
        addLabel(_overviewTable, utils::ressourceToString(points.metal), index, 1);
        addLabel(_overviewTable, utils::ressourceToString(points.cristal), index, 2);
        addLabel(_overviewTable, utils::ressourceToString(points.deut), index, 3);

        int pointsTotal = points.metal + points.cristal + points.deut;
        addLabel(_overviewTable, utils::ressourceToString(pointsTotal), index, 4);
        index++;
    };

    Ressources<int> productionStat = PlayerManager::instance().getProduction(PlayerManager::ProductionStat::Total);
    addProdLabel("Mines : ", productionStat);

    Ressources<int> discoveryRessources = DiscoveryManager::instance().getSummary().globalMean;
    addProdLabel("Discovery : ", discoveryRessources);

    Ressources<int> totalRessources = productionStat + discoveryRessources;
    addProdLabel("Total : ", totalRessources);

    index++; //blank

    addLabel(_overviewTable, "Points", index, 0);
    addLabel(_overviewTable, "Métal", index, 1);
    addLabel(_overviewTable, "Cristal", index, 2);
    addLabel(_overviewTable, "Deuterium", index, 3);
    addLabel(_overviewTable, "Total", index, 4);
    index++;

    addPointLabel("Recherches : ", GeneralManager::instance().getInvestedRessources());
    addPointLabel("Fleet : ", PlayerManager::instance().getFleetRessources());

    int numberPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < numberPlanet; ++i)
    {
        const Planet* planet = PlayerManager::instance().getPlanet(i);
        addPointLabel(planet->getName() +  " : ", planet->getAllInvestedRessources());
    }

    addPointLabel("Total : ", PlayerManager::instance().getAllInvestedRessources());
}

void MainWindow::buildResumePlanetOutputs(int indexPlanet, float factor)
{
    int index = 0;

    addLabel(_overviewTable, "Productions", index, 0);
    addLabel(_overviewTable, "Métal",       index, 1);
    addLabel(_overviewTable, "Cristal",     index, 2);
    addLabel(_overviewTable, "Deuterium",   index, 3);
    addLabel(_overviewTable, "Energy",      index, 4);
    index++;

    const Planet* planet = PlayerManager::instance().getPlanet(indexPlanet);

    if (!planet) return;

    auto addProdLabel = [this, &index, &factor](const QString& label, const Ressources<int>& prod)
    {
        addLabel(_overviewTable, label, index, 0);
        addLabel(_overviewTable, utils::ressourceToString((int)((float)prod.metal    * factor)), index, 1);
        addLabel(_overviewTable, utils::ressourceToString((int)((float)prod.cristal  * factor)), index, 2);
        addLabel(_overviewTable, utils::ressourceToString((int)((float)prod.deut     * factor)), index, 3);
        addLabel(_overviewTable, utils::ressourceToString(prod.energy),            index, 4);
        index++;
    };

    const Ressources<int>& baseProd = planet->getProductionStat(Planet::ProductionStat::Base);
    addProdLabel("Base", baseProd);

    // Common buildings
    int numberCommonTech = planet->getNumberTech(TechType::CommonBuilding);
    for (int i = 0; i < numberCommonTech; ++i)
    {
        const CommonTech* tech = TechManager::instance().getTech(TechType::CommonBuilding, i);
        if (tech->isValid())
        {
            int level = planet->getTechLevel(TechType::CommonBuilding, i);
            const Ressources<int>& buildingProd = planet->getTechProduction(TechType::CommonBuilding, i);
            if (!buildingProd.empty())
            {
                addProdLabel(tech->name + " : " + QString::number(level), buildingProd);
            }
        }
    }

    // Life form buildings
    TechType typeBuilding = speciesToBuildingLifeForm.at(planet->getSpecies());
    int lifeFormBuildingNumber = typeBuilding == TechType::None ? 0 : TechManager::instance().getNumberTechs(typeBuilding);

    for (int i = 0; i < lifeFormBuildingNumber; i++)
    {
        const CommonTech* tech = TechManager::instance().getTech(typeBuilding, i);
        if (tech->isValid())
        {
            int level = planet->getTechLevel(typeBuilding, i);
            const Ressources<int>& buildingProd = planet->getTechProduction(typeBuilding, i);
            if (!buildingProd.empty())
            {
                addProdLabel(tech->name + " : " + QString::number(level), buildingProd);
            }
        }
    }

    // Satellites
    Ressources<int> prodSatellites;
    prodSatellites.energy = planet->getSatelliteProduction();
    addProdLabel("Satellites solaires : " + QString::number(planet->getDefense(FixUnitType::SolarSatellite)), prodSatellites);

    // Crawlers
    Ressources<int> prodCrawlers = planet->getProductionStat(Planet::ProductionStat::Crawlers);
    addProdLabel("Foreuses : " + QString::number(planet->getActiveCrawler()) + " / " + QString::number(planet->getMaxActiveCrawler()), prodCrawlers);

    // Plasma
    Ressources<int> prodPlasma = planet->getProductionStat(Planet::ProductionStat::Plasma);
    addProdLabel("Plasma : " + QString::number(GeneralManager::instance().getResearchLevel(ResearchType::Plasma)), prodPlasma);

    // Ingenior
    Ressources<int> prodIngenior = planet->getProductionStat(Planet::ProductionStat::Ingenior);
    addProdLabel("Ingenior : ", prodIngenior);

    // Geolog
    Ressources<int> prodGeolog = planet->getProductionStat(Planet::ProductionStat::Geolog);
    addProdLabel("Geolog : ", prodGeolog);

    // Council
    Ressources<int> prodCouncil = planet->getProductionStat(Planet::ProductionStat::Council);
    addProdLabel("Council : ", prodCouncil);

    // Class
    Ressources<int> prodClass = planet->getProductionStat(Planet::ProductionStat::Class);
    addProdLabel("Class : ", prodClass);

    // Alliance class
    Ressources<int> prodAllianceClass = planet->getProductionStat(Planet::ProductionStat::AllianceClass);
    addProdLabel("Alliance class : ", prodAllianceClass);

    // Total
    Ressources<int> prodTotal = planet->getProductionStat(Planet::ProductionStat::Total);
    int consumption = planet->getGlobalConsumption();
    //addProdLabel("Total : ", prodTotal);
    addLabel(_overviewTable, "Total : ", index, 0);
    addLabel(_overviewTable, utils::ressourceToString((int)((float)prodTotal.metal    * factor)), index, 1);
    addLabel(_overviewTable, utils::ressourceToString((int)((float)prodTotal.cristal  * factor)), index, 2);
    addLabel(_overviewTable, utils::ressourceToString((int)((float)prodTotal.deut     * factor)), index, 3);
    addLabel(_overviewTable, utils::ressourceToString(prodTotal.energy) + " / " + QString::number(consumption), index, 4);
    index++;
}

void MainWindow::buildRentaOutputs()
{
    QTableWidget* tableWidget = _rentaTable;
    tableWidget->clear();

    addLabel(tableWidget, "A augmenter", 0, 0);
    addLabel(tableWidget, "Planète", 0, 1);

    RentabilityManager::RentabilityType rentabilityType = RentabilityManager::instance().getRentabilityType();
    if (rentabilityType == RentabilityManager::RentabilityType::Ressources)
    {
        addLabel(tableWidget, "Temps de recouvrement", 0, 2);
    }
    else if (rentabilityType == RentabilityManager::RentabilityType::Energy)
    {
        addLabel(tableWidget, "Eq deut per energy", 0, 2);
    }
    else
    {
        addLabel(tableWidget, "Eq deut per percent", 0, 2);
    }

    addLabel(tableWidget, "Temps de construction", 0, 3);
    addLabel(tableWidget, "Planet filter", 0, 5);
    addLabel(tableWidget, "Type filter", 0, 6);

    int nb = RentabilityManager::instance().refresh();
    for (int i = 0; i < std::min(NUMBER_RENTA_MAX, nb); ++i)
    {
        const RentabilityManager::LevelUp* levelUp = RentabilityManager::instance().getLevelUp(i);

        QLabel* labelPlanets = createPlanetsLabel(levelUp->indexPlanets);
        tableWidget->setCellWidget(i+1, 1, labelPlanets);

        addLabel(tableWidget, levelUp->_name + " : " + QString::number(levelUp->_levelToUpdate), i+1, 0);

        QString textScore;
        if (rentabilityType == RentabilityManager::RentabilityType::Ressources)
        {
            textScore = utils::timeToString(levelUp->_score);
        }
        else
        {
            textScore = utils::ressourceToString(levelUp->_score);
        }
        addLabel(tableWidget, textScore, i+1, 2);
        addLabel(tableWidget, utils::timeToString(levelUp->timeToCompleteDay), i+1, 3);
    }

    for (int i = 0; i < PlayerManager::instance().getNumberPlanets(); ++i)
    {
        const Planet* planet = PlayerManager::instance().getPlanet(i);
        bool state = RentabilityManager::instance().getFilterPlanet(i);
        CheckBoxItem* planetFilterItem = addCheckBoxItem(tableWidget, planet->getName(), i + 1, 5, state);
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

    int speed = GeneralManager::instance().getUniverseSpecific(UniverseSpecifics::EconnomicSpeed);
    Item* universeSpeed = addSpinBoxItem(tableWidget, "Vitesse univers", 1, column, 1, speed);
    universeSpeed->setOnValueChanged([](int value) {
        GeneralManager::instance().setUniverseSpecific(UniverseSpecifics::EconnomicSpeed, value);
    });

    int researchSpeedValue = GeneralManager::instance().getUniverseSpecific(UniverseSpecifics::ResearchBoost);
    Item* researchSpeed = addSpinBoxItem(tableWidget, "Vitesse recherches", 2, column, 1, researchSpeedValue);
    researchSpeed->setOnValueChanged([](int value) {
        GeneralManager::instance().setUniverseSpecific(UniverseSpecifics::ResearchBoost, value);
    });

    addLabel(tableWidget, "Niveaux races : ", 4, column);
    for (int i = 1; i < static_cast<int>(Species::Count); ++i) {
        int levelRace = GeneralManager::instance().getLevelSpecies(static_cast<Species>(i));
        Item* speciesLevel = addSpinBoxItem(tableWidget, "Niveau " + speciesToString[i], i+5, column, levelRace, 0, 100);
        speciesLevel->setOnValueChanged([this, i](int value) {
            GeneralManager::instance().setSpecies(static_cast<Species>(i), value);
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

        int level = GeneralManager::instance().getResearchLevel(type);
        Item* researchItem = addSpinBoxItem(tableWidget, research->name, i+1, column, level);
        researchItem->setOnValueChanged([this, column, i, type](int value) {
            GeneralManager::instance().setResearchLevel(type, value);
            emit techChanged();
        });
    }
}

void MainWindow::buildSpecialisationImputs(QTableWidget* tableWidget, int column)
{
    int currentRow = 0;
    addLabel(tableWidget, "Classes : ", currentRow++, column);

    int indexClass = static_cast<int>(GeneralManager::instance().getClass());
    ComboBoxItem* classBox = addComboBoxItem(tableWidget, "Classe", classToString, currentRow++, column, indexClass);
    classBox->setOnValueChanged([this](int index) {
        GeneralManager::instance().setClass(static_cast<Class>(index));
        emit techChanged();
    });

    int indexAllianceClass = static_cast<int>(GeneralManager::instance().getAllianceClass());
    ComboBoxItem* alliClassBox = addComboBoxItem(tableWidget, "Classe alli", allianceClassToString, currentRow++, column, indexAllianceClass);
    alliClassBox->setOnValueChanged([this](int index) {
        GeneralManager::instance().setAllianceClass(static_cast<AllianceClass>(index));
        emit techChanged();
    });

    // Officers
    currentRow++;
    addLabel(tableWidget, "Officiers : ", currentRow++, column);

    std::map<Officers, CheckBoxItem*> officerWidgets;
    for (int i = 0; i < static_cast<int>(Officers::Count); ++i)
    {
        Officers officer = static_cast<Officers>(i);
        bool initValue = GeneralManager::instance().getOfficerValue(officer);
        CheckBoxItem* officerCheckBox = addCheckBoxItem(tableWidget, officerToString[i], currentRow++, column, initValue);
        officerCheckBox->addOnValueChanged([this, officer](bool value) {
            GeneralManager::instance().setOfficerActivated(officer, value);
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
                    GeneralManager::instance().setOfficerActivated(officer, true);
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

    double metalRateValue = GeneralManager::instance().getConversionRate().metal;
    DoubleSpinBoxItem* metalRate = addDoubleSpinBoxItem(tableWidget, "Métal", 1, column, metalRateValue, 1.0);
    metalRate->setOnValueChanged([this](double value) {
        GeneralManager::instance().setConversionRateAt(RessourceType::Metal, value);
        emit techChanged();
    });

    double cristalRateValue = GeneralManager::instance().getConversionRate().cristal;
    DoubleSpinBoxItem* cristalRate = addDoubleSpinBoxItem(tableWidget, "Cristal", 2, column, cristalRateValue, 1.0);
    cristalRate->setOnValueChanged([this](double value) {
        GeneralManager::instance().setConversionRateAt(RessourceType::Cristal, value);
        emit techChanged();
    });

    double deutRateValue = GeneralManager::instance().getConversionRate().deut;
    DoubleSpinBoxItem* deutRate = addDoubleSpinBoxItem(tableWidget, "Deut", 3, column, deutRateValue, 1.0);
    deutRate->setOnValueChanged([this](double value) {
        GeneralManager::instance().setConversionRateAt(RessourceType::Deut, value);
        emit techChanged();
    });

    int rate = GeneralManager::instance().getScrapRate();
    Item* scrapRate = addSpinBoxItem(tableWidget, "Taux ferraille (%)", 5, column, rate, 35, 100);
    scrapRate->setOnValueChanged([this](int value) {
        GeneralManager::instance().setScrapRate(value);
        emit techChanged();
    });
}

void MainWindow::buildPlanetImputs(QTableWidget* tableWidget, int column)
{
    int currentRow = 0;
    Planet* planet = PlayerManager::instance().getPlanet(column);

    if (!planet) return;

    // Name + delete
    PlanetItem* planetItem = new PlanetItem(planet->getName(), tableWidget);
    planetItem->setOnValueChanged([this, &planet](const QString &text){
        planet->setName(text);
        emit rentaChanged();
    });
    planetItem->setOnDelete([this, column](){
        PlayerManager::instance().removePlanet(column);
        RentabilityManager::instance().onPlanetRemoved(column);
        emit planetsChanged();
    });
    tableWidget->setCellWidget(currentRow++, column, planetItem);

    // Position
    PositionItem* positionItem = addPositionItem(tableWidget, currentRow++, column, planet->getPosition());
    positionItem->setOnValueChanged([this, planet](int g, int s, int p) {
        PlanetPosition newPos(g, s, p);
        planet->setPosition(newPos);
        emit techChanged();
    });

    // Temperature
    Item* temperatureItem = addSpinBoxItem(tableWidget, "Temp max : ", currentRow++, column, planet->getTemperatureMax(), -999, 999);
    temperatureItem->setOnValueChanged([this, planet](int value){
        planet->setTemperatureMax(value);
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
    float expePerDay = PlayerManager::instance().getDiscoveryPerDay();
    DoubleSpinBoxItem* expeItem = addDoubleSpinBoxItem(tableWidget, "Expé/j : ", 0, 0, expePerDay, 0, 999.9);
    expeItem->setOnValueChanged([this](double value) {
        PlayerManager::instance().setDiscoveryPerDay((float)value);
    });

    float bonusTemp = PlayerManager::instance().getTempBonusRessources();
    Item* tempBonus = addSpinBoxItem(tableWidget, "Bonus R : ", 0, 1, bonusTemp, 0, 99);
    tempBonus->setOnValueChanged([this](int value) {
        float bonusRessources = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::ExpeditionRessourcesIncrease);
        float bonusFleat = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::ExpeditionShipIncrease);
        float bonusAM = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::Antimatter);
        float bonusDiscover = PlayerManager::instance().getLifeFormBonus(BonusLifeForm::ExploratorClass);
        PlayerManager::instance().setTempBonusRessources(value);
        DiscoveryManager::instance().loadBonusFactor(bonusRessources, bonusFleat, bonusAM, bonusDiscover);
    });

    float deutConsumption = PlayerManager::instance().getDeutConsumption();
    Item* deutConso = addSpinBoxItem(tableWidget, "Deut cons : ", 0, 2, deutConsumption, 0, 99999);
    deutConso->setOnValueChanged([this](int value) {
        PlayerManager::instance().setDeutConsumption(value);
    });

    float positionDiscovery = PlayerManager::instance().getPositionDiscovery();
    Item* posDiscoveryItem = addSpinBoxItem(tableWidget, "Pos discovery : ", 0, 3, positionDiscovery, 1, 15);
    posDiscoveryItem->setOnValueChanged([this](int value) {
        PlayerManager::instance().setPositionDiscovery(value);
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
    int indexRow = 2;
    addLabel(tableWidget, "Ajout : ",           indexRow, index++);
    addLabel(tableWidget, "Type : ",            indexRow, index++);
    addLabel(tableWidget, "Number : ",          indexRow, index++);
    addLabel(tableWidget, "Metal (M) : ",       indexRow, index++);
    addLabel(tableWidget, "Cristal (M) : ",     indexRow, index++);
    addLabel(tableWidget, "Deut (M) : ",        indexRow, index++);
    addLabel(tableWidget, "AM (milliers) : ",   indexRow, index++);

    for (int i = 1; i < static_cast<int>(DiscoveryManager::DiscoveryType::Count); ++i)
    {
        DiscoveryManager::DiscoveryType type = static_cast<DiscoveryManager::DiscoveryType>(i);
        QString strType = DiscoveryManager::instance().getTypeStrList(type);
        addLabel(tableWidget, strType, i + indexRow, 1);

        QSpinBox* numberDiscovery = new QSpinBox();
        numberDiscovery->setMaximum(999);
        tableWidget->setCellWidget(i + indexRow, 2, numberDiscovery);

        DiscoveryManager::Discovery discovery = DiscoveryManager::instance().getDiscovery(type);

        QDoubleValidator* validator = new QDoubleValidator(0.0, 1000.0, 3, this);
        validator->setNotation(QDoubleValidator::StandardNotation);
        validator->setLocale(QLocale::C);

        QLineEdit* metalValue = new QLineEdit();
        metalValue->setValidator(validator);
        metalValue->setEnabled(discovery.hasRessource(RessourceType::Metal));
        tableWidget->setCellWidget(i + indexRow, 3, metalValue);

        QLineEdit* cristalValue = new QLineEdit();
        cristalValue->setValidator(validator);
        cristalValue->setEnabled(discovery.hasRessource(RessourceType::Cristal));
        tableWidget->setCellWidget(i + indexRow, 4, cristalValue);

        QLineEdit* deutValue = new QLineEdit();
        deutValue->setValidator(validator);
        deutValue->setEnabled(discovery.hasRessource(RessourceType::Deut));
        tableWidget->setCellWidget(i + indexRow, 5, deutValue);

        QLineEdit* amValue = new QLineEdit();
        amValue->setValidator(validator);
        amValue->setEnabled(discovery.hasRessource(RessourceType::Antimatter));
        tableWidget->setCellWidget(i + indexRow, 6, amValue);
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

void MainWindow::buildPlanification(QTableWidget* tableWidget, int indexPlanif)
{
    PlayerManager::Planification* planif = PlayerManager::instance().getPlanif(indexPlanif);
    Planet& planet = planif->planet;
    int currentRow = 0;

    // Type
    addLabel(tableWidget, PlayerManager::instance()._planificationStr.at(static_cast<int>(planif->type)), currentRow++, indexPlanif);

    // Name + delete
    PlanetItem* planetItem = new PlanetItem(planet.getName(), tableWidget);
    planetItem->setOnValueChanged([this, &planet](const QString &text){
        planet.setName(text);
    });
    planetItem->setOnDelete([this, indexPlanif](){
        PlayerManager::instance().removePlanif(indexPlanif);
        emit planifChanged();
    });
    tableWidget->setCellWidget(currentRow++, indexPlanif, planetItem);

    // Position
    PositionItem* positionItem = addPositionItem(tableWidget, currentRow++, indexPlanif, planet.getPosition());
    positionItem->setOnValueChanged([this, &planet](int g, int s, int p) {
        planet.setPosition(PlanetPosition(g, s, p));
        emit planifChanged();
        emit rentaChanged();
    });

    // Temperature
    Item* temperatureItem = addSpinBoxItem(tableWidget, "Temp max : ", currentRow++, indexPlanif, planet.getTemperatureMax(), -99, 99);
    temperatureItem->setOnValueChanged([this, &planet](int value){
        planet.setTemperatureMax(value);
        emit planifChanged();
        emit rentaChanged();
    });
    currentRow++;

    // Life form choice
    createPlanetLifeFormChoice(tableWidget, &planet, currentRow, indexPlanif);

    // Common buildings
    createPlanetCommonBuilding(tableWidget, &planet, currentRow, indexPlanif);

    // Life form buildings
    createPlanetLifeFormBuilding(tableWidget, &planet, currentRow, indexPlanif);

    // Life form research
    addLabel(tableWidget, "Life form research : ", currentRow++, indexPlanif);
    createPlanetLifeFormResearches(tableWidget, &planet, currentRow, indexPlanif);

    // Units
    addLabel(tableWidget, "Units : ", currentRow++, indexPlanif);
    for (int i = 0; i < static_cast<int>(FixUnitType::Count); ++i)
    {
        FixUnitType unitType = static_cast<FixUnitType>(i);
        createPlanetDefenses(tableWidget, unitType, &planet, currentRow, indexPlanif);
    }
}

void MainWindow::buildFleetImputs(QTableWidget* tableWidget)
{
    tableWidget->clear();

    int currentRow = 0;
    addLabel(tableWidget, "Fleet : ", 0, 0);
    addLabel(tableWidget, "Ferraille : ", 0, 1);
    addLabel(tableWidget, "Métal : ", 0, 2);
    addLabel(tableWidget, "Cristal : ", 0, 3);
    addLabel(tableWidget, "Deut : ", 0, 4);

    onFleetChanged();

    currentRow++;
    for (int i = 0; i < static_cast<int>(MovingUnitType::Count); ++i)
    {
        MovingUnitType unitType = static_cast<MovingUnitType>(i);
        Unit unit = TechManager::instance().getMovingUnit(unitType);
        int initValue = PlayerManager::instance().getShip(unitType);
        Item* item = addSpinBoxItem(tableWidget, unit.name, currentRow, 0, initValue, 0, 999999999);
        item->setOnValueChanged([this, unitType](int value) {
            PlayerManager::instance().setShip(unitType, value);
            emit fleetChanged();
        });

        QSpinBox* scrapShip = new QSpinBox(tableWidget);
        scrapShip->setMaximum(999999999);
        scrapShip->setValue(PlayerManager::instance().getScrapShip(unitType));
        connect(scrapShip, &QSpinBox::valueChanged, this, [this, unitType](int value)
        {
            PlayerManager::instance().setScrapShip(unitType, value);
            PlayerManager::instance().computeScrapValue();
            emit fleetChanged();
        });
        tableWidget->setCellWidget(currentRow, 1, scrapShip);

        currentRow++;
    }
}

void MainWindow::onPlanetsChanged()
{
    _planetTable->clear();
    PlayerManager::instance().refresh();

    int numberPlanet = PlayerManager::instance().getNumberPlanets();
    _planetTable->setColumnCount(numberPlanet);

    for (int i = 0; i < numberPlanet; ++i)
    {
        buildPlanetImputs(_planetTable, i);
    }

    for (int col = 0; col < _planetTable->columnCount(); ++col) {
        _planetTable->setColumnWidth(col, 160); // width in pixel
    }

    emit rentaChanged();
}

void MainWindow::onTechChanged()
{
    PlayerManager::instance().refresh();
    DiscoveryManager::instance().refresh();

    buildResumeOutputs();
    buildRentaOutputs();
    buildDiscoveryImputs(_discoveryTab);
}

void MainWindow::onRentaChanged()
{
    buildRentaOutputs();
}

void MainWindow::onPlanifChanged()
{
    _planificationTab->clear();

    int numberPlanet = PlayerManager::instance().getNumberPlanifs();
    _planificationTab->setColumnCount(numberPlanet);
    for (int i = 0; i < numberPlanet; ++i)
    {
        buildPlanification(_planificationTab, i);
    }

    for (int col = 0; col < _planificationTab->columnCount(); ++col) {
        _planificationTab->setColumnWidth(col, 160); // width in pixel
    }

    emit rentaChanged();
}

void MainWindow::onFleetChanged()
{
    Ressources<float> scrapValue = PlayerManager::instance().getScrapValue();
    addLabel(_fleetTab, utils::ressourceToString(scrapValue.metal), 1, 2);
    addLabel(_fleetTab, utils::ressourceToString(scrapValue.cristal), 1, 3);
    addLabel(_fleetTab, utils::ressourceToString(scrapValue.deut), 1, 4);
}
