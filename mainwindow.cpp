#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "techManager.h"
#include "playerManager.h"

#include <QLabel>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Q_ASSERT(ui->commonTab);
    Q_ASSERT(ui->planetsTab);

    _generalTable = ui->commonTab;
    _planetTable = ui->planetsTab;

    initTable(_generalTable);
    initTable(_planetTable);

    buildGeneralImputs(_generalTable, 0);
    buildResearchImputs(_generalTable, 1);
    buildSpecialisationImputs(_generalTable, 2);
    buildTradeImputs(_generalTable, 3);

    int numberPlanet = PlayerManager::instance().getNumberPlanets();
    for (int i = 0; i < numberPlanet; ++i)
    {
        buildPlanetImputs(_planetTable, i);
    }

    QPushButton* saveButton = ui->saveButton;
    connect(saveButton, &QPushButton::clicked, this, []() {
        PlayerManager::instance().saveGameData();
    });
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
    item->setValue(defaultValue);
    item->setMinValue(minValue);
    item->setMaxValue(maxValue);
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
        int levelRace = PlayerManager::instance().getSpecies(static_cast<Species>(i-1));
        Item* speciesLevel = addSpinBoxItem(tableWidget, "Niveau " + speciesToString[i], i+5, column, levelRace, 0, 100);
        speciesLevel->setOnValueChanged([i](int value) {
            PlayerManager::instance().setSpecies(static_cast<Species>(i), value);
        });
    }
}

void MainWindow::buildResearchImputs(QTableWidget* tableWidget, int column)
{
    addLabel(tableWidget, "Recherches : ", 0, column);

    int numberOfResearch = TechManager::instance().getNumberTechs(TechType::CommonResearch);

    for (int i = 0; i < numberOfResearch; ++i) {
        const CommonTech& research = TechManager::instance().getTech(TechType::CommonResearch, i);

        ResearchType type = static_cast<ResearchType>(i);

        int level = PlayerManager::instance().getResearchLevel(type);
        Item* researchItem = addSpinBoxItem(tableWidget, research.name, i+1, column, level);
        researchItem->setOnValueChanged([column, i, type](int value) {
            PlayerManager::instance().setResearchLevel(type, value);
        });
    }
}

void MainWindow::buildSpecialisationImputs(QTableWidget* tableWidget, int column)
{
    addLabel(tableWidget, "Classes : ", 0, column);

    int indexClass = static_cast<int>(PlayerManager::instance().getClass());
    ComboBoxItem* classBox = addComboBoxItem(tableWidget, "Classe", classToString, 1, column, indexClass);
    classBox->setOnValueChanged([](int index) {
        PlayerManager::instance().setClass(static_cast<Class>(index));
    });

    int indexAllianceClass = static_cast<int>(PlayerManager::instance().getAllianceClass());
    ComboBoxItem* alliClassBox = addComboBoxItem(tableWidget, "Classe alli", allianceClassToString, 2, column, indexAllianceClass);
    alliClassBox->setOnValueChanged([](int index) {
        PlayerManager::instance().setAllianceClass(static_cast<AllianceClass>(index));
    });

    addLabel(tableWidget, "Officiers : ", 4, column);

    for (int i = 0; i < static_cast<int>(Officers::Count); ++i)
    {
        Officers officer = static_cast<Officers>(i);
        bool initValue = PlayerManager::instance().getOfficerValue(officer);
        CheckBoxItem* officerCheckBox = addCheckBoxItem(tableWidget, officerToString[i], i+5, column, initValue);
        officerCheckBox->setOnValueChanged([officer](bool value) {
            PlayerManager::instance().setOfficerActivated(officer, value);
        });
    }
}

void MainWindow::buildTradeImputs(QTableWidget* tableWidget, int column)
{
    addLabel(tableWidget, "Taux de conversion : ", 0, column);

    double metalRateValue = PlayerManager::instance().getConversionRate(ConversionRate::Metal);
    DoubleSpinBoxItem* metalRate = addDoubleSpinBoxItem(tableWidget, "Métal", 1, column, metalRateValue, 1.0);
    metalRate->setOnValueChanged([](double value) {
        PlayerManager::instance().setConversionRate(ConversionRate::Metal, value);
    });

    double cristalRateValue = PlayerManager::instance().getConversionRate(ConversionRate::Cristal);
    DoubleSpinBoxItem* cristalRate = addDoubleSpinBoxItem(tableWidget, "Cristal", 2, column, cristalRateValue, 1.0);
    cristalRate->setOnValueChanged([](double value) {
        PlayerManager::instance().setConversionRate(ConversionRate::Cristal, value);
    });

    double deutRateValue = PlayerManager::instance().getConversionRate(ConversionRate::Deut);
    DoubleSpinBoxItem* deutRate = addDoubleSpinBoxItem(tableWidget, "Deut", 3, column, deutRateValue, 1.0);
    deutRate->setOnValueChanged([](double value) {
        PlayerManager::instance().setConversionRate(ConversionRate::Deut, value);
    });

    int rate = PlayerManager::instance().getScrapRate();
    Item* scrapRate = addSpinBoxItem(tableWidget, "Taux ferraille (%)", 5, column, rate, 35, 100);
    scrapRate->setOnValueChanged([](int value) {
        PlayerManager::instance().setScrapRate(value);
    });
}

void MainWindow::buildPlanetImputs(QTableWidget* tableWidget, int column)
{
    const QString& planetName = PlayerManager::instance().getPlanetName(column);
    addLabel(tableWidget, planetName, 0, column);

    addLabel(tableWidget, "Buildings : ", 1, column);

    int indexBuilding = 2;
    int numberBuilding = TechManager::instance().getNumberTechs(TechType::CommonBuilding);
    for (int i = 0; i < numberBuilding; ++i)
    {
        int level = PlayerManager::instance().getTechLevel(column, TechType::CommonBuilding, i);
        const CommonTech& tech = TechManager::instance().getTech(TechType::CommonBuilding, i);
        if (tech.isValid())
        {
            Item* buildingItem = addSpinBoxItem(tableWidget, tech.name, indexBuilding++, column, level);
            buildingItem->setOnValueChanged([column, i](int value) {
                PlayerManager::instance().setTechLevel(column, TechType::CommonBuilding, i, value);
            });
        }
        else
        {
            qWarning() << "tech not found : " << i;
        }
    }
}
