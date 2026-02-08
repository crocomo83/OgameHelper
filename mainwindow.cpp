#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "techManager.h"
#include "playerManager.h"
#include "rentabilityManager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Q_ASSERT(ui->overviewTab);
    Q_ASSERT(ui->commonTab);
    Q_ASSERT(ui->planetsTab);

    _overviewTable = ui->overviewTab;
    _rentaTable = ui->renta;
    _generalTable = ui->commonTab;
    _planetTable = ui->planetsTab;

    initTable(_overviewTable);
    initTable(_generalTable);

    _planetTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    _planetTable->horizontalHeader()->setStretchLastSection(false);
    _planetTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    _planetTable->verticalHeader()->setStretchLastSection(false);
    _planetTable->setFrameShape(QFrame::NoFrame);
    _planetTable->setSelectionMode(QAbstractItemView::NoSelection);
    _planetTable->setFocusPolicy(Qt::NoFocus);

    buildResumeOutputs(_overviewTable);
    buildRentaOutputs(_rentaTable);
    buildGeneralImputs(_generalTable, 0);
    buildResearchImputs(_generalTable, 1);
    buildSpecialisationImputs(_generalTable, 2);
    buildTradeImputs(_generalTable, 3);

    onPlanetsChanged();
    onTechChanged();

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

void MainWindow::addLabel(QTableWidget* tableWidget, QString str, int row, int column)
{
    QLabel* label = new QLabel(str);
    tableWidget->setCellWidget(row, column, label);
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
    doubleSpinBox->setValue(defaultValue);
    doubleSpinBox->setMinValue(minValue);
    doubleSpinBox->setMaxValue(maxValue);
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
    comboBoxItem->setValue(defaultValue);
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

void MainWindow::buildResumeOutputs(QTableWidget* tableWidget)
{
    tableWidget->clear();
    int index = 0;

    addLabel(tableWidget, "Productions", index, 0);
    addLabel(tableWidget, "Métal", index, 1);
    addLabel(tableWidget, "Cristal", index, 2);
    addLabel(tableWidget, "Deuterium", index, 3);
    index++;

    for (int i = 0; i < static_cast<int>(PlayerManager::ProductionStat::Count); i++)
    {
        PlayerManager::ProductionStat stat = static_cast<PlayerManager::ProductionStat>(i);
        Ressources base = PlayerManager::instance().getProduction(stat);
        addLabel(tableWidget, PlayerManager::instance().getProductionStr(stat), index, 0);
        addLabel(tableWidget, QString::number((int)base.metal), index, 1);
        addLabel(tableWidget, QString::number((int)base.cristal), index, 2);
        addLabel(tableWidget, QString::number((int)base.deut), index, 3);
        index++;
    }
}

void MainWindow::buildRentaOutputs(QTableWidget* tableWidget)
{
    addLabel(tableWidget, "A augmenter", 0, 0);

    addLabel(tableWidget, "Planète", 0, 1);
    addLabel(tableWidget, "Temps de recouvrement", 0, 2);

    int nb = RentabilityManager::instance().refresh();
    for (int i = 0; i < std::min(10, nb); ++i)
    {
        const RentabilityManager::LevelUp& levelUp = RentabilityManager::instance().getLevelUp(i);

        QString planetName = "";
        if (levelUp.indexPlanet >= 0)
        {
            const Planet* planet = PlayerManager::instance().getPlanet(levelUp.indexPlanet);
            planetName = planet->getName();
        }

        const QString& name = RentabilityManager::instance().levelUpToString(levelUp.type);
        addLabel(tableWidget, name + " : " + QString::number(levelUp.levelToUpdate), i+1, 0);
        addLabel(tableWidget, planetName, i+1, 1);
        addLabel(tableWidget, levelUp.timeToRecoverStr, i+1, 2);
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
    Planet* planet = PlayerManager::instance().getPlanet(column);

    // Name
    QLineEdit* lineEdit = new QLineEdit(planet->getName(), tableWidget);
    connect(lineEdit, &QLineEdit::textChanged, this,
            [planet](const QString &text) {
                planet->setName(text);
    });
    tableWidget->setCellWidget(currentRow++, column, lineEdit);

    // Position
    PositionItem* positionItem = addPositionItem(tableWidget, currentRow++, column, planet->getPosition());
    positionItem->setOnValueChanged([this, planet](int g, int s, int p) {
        planet->setPosition(PlanetPosition(g, s, p));
        emit techChanged();
    });

    // Life form
    currentRow++;
    addLabel(tableWidget, "Forme de vie", currentRow++, column);
    int speciesIndex = static_cast<int>(planet->getSpecies());
    ComboBoxItem* speciesItem = addComboBoxItem(tableWidget, "Choix : ", speciesToString, currentRow++, column, speciesIndex);
    speciesItem->setOnValueChanged([this, planet](int value) {
        planet->setSpecies(static_cast<Species>(value));
        emit techChanged();
        emit planetsChanged();
    });

    currentRow++; //blank

    // Buildings
    std::vector<TechType> techTypes = planet->getAvailableBuildings();
    for (TechType techType : techTypes)
    {
        if (techType == TechType::None) { continue;}

        addLabel(tableWidget, techTypeToString[static_cast<int>(techType)] + " : ", currentRow++, column);

        int numberTechs = TechManager::instance().getNumberTechs(techType);
        for (int i = 0; i < numberTechs; ++i)
        {
            int level = planet->getTechLevel(techType, i);
            const CommonTech* tech = TechManager::instance().getTech(techType, i);
            if (tech->isValid())
            {
                Item* buildingItem = addSpinBoxItem(tableWidget, tech->name, currentRow++, column, level);
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
        currentRow++; // blank
    }

    // Life form research
    int numberLifeFormReseach = TechManager::instance().getNumberTechs(TechType::HumanResearch);
    for (int i = 0; i < numberLifeFormReseach; ++i)
    {
        Species speciesChoice = planet->getChoiceLifeFormResearch(i);
        int levelInit = planet->getLevelLifeFormResearch(speciesChoice, i);
        LifeFormSelectItem* item = addLifeFormSelectItem(tableWidget, currentRow++, column, speciesChoice, levelInit);
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
    }

    QString label = "Foreuses (max=" + QString::number(planet->getMaxActiveCrawler()) + ")";
    Item* crawlerItem = addSpinBoxItem(tableWidget, label, currentRow++, column, planet->getCrawlerNumber(), 0, 9999);
    crawlerItem->setOnValueChanged([this, planet](int value) {
        planet->setCrawlerNumber(value);
        emit techChanged();
    });
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
        Planet* planet = PlayerManager::instance().getPlanet(i);
        planet->computeLifeFormBuildingBonus();
        planet->computeBonusPos();
        planet->computeProduction();
    }

    PlayerManager::instance().computeLifeFormResearch();
    PlayerManager::instance().computeProduction();

    buildResumeOutputs(_overviewTable);
    buildRentaOutputs(_rentaTable);
}
