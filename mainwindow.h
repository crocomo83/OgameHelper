#pragma once

#include "commons.h"
#include "item.h"
#include "doubleSpinBoxItem.h"
#include "checkboxitem.h"
#include "comboboxitem.h"
#include "lineEditItem.h"
#include "positionitem.h"
#include "lifeFormSelectItem.h"

#include <QMainWindow>
#include <QVBoxLayout>
#include <QTableWidget>

#include <limits>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Planet;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initTable(QTableWidget* tableWidget);

    QLabel* addLabel(QTableWidget* tableWidget, QString str, int row, int column);
    Item* addSpinBoxItem(QTableWidget* tableWidget, QString str, int row, int column, int defaultValue = 0, int minValue = 0, int maxValue = 99);
    DoubleSpinBoxItem* addDoubleSpinBoxItem(QTableWidget* tableWidget, QString str, int row, int column, double defaultValue = 0.0, double minValue = 0.0, double maxValue = 99.9);
    CheckBoxItem* addCheckBoxItem(QTableWidget* tableWidget, QString str, int row, int column, bool defaultValue = false);
    ComboBoxItem* addComboBoxItem(QTableWidget* tableWidget, QString str, QStringList names, int row, int column, int defaultValue);
    PositionItem* addPositionItem(QTableWidget* tableWidget, int row, int column, const PlanetPosition &planetPosition);
    LifeFormSelectItem* addLifeFormSelectItem(QTableWidget* tableWidget, int row, int column, Species species, int level);
    std::unordered_set<Species> getAllAvailableSpecies();

    QLabel* createPlanetsLabel(std::vector<int> indexPlanets);
    void createPlanetLifeFormChoice(QTableWidget* tableWidget, Planet *planet, int& row, int column);
    void createPlanetCommonBuilding(QTableWidget* tableWidget, Planet *planet, int& row, int column);
    void createPlanetLifeFormBuilding(QTableWidget* tableWidget, Planet *planet, int& row, int column);
    void createPlanetLifeFormResearches(QTableWidget* tableWidget, Planet *planet, int& row, int column);
    void createPlanetDefenses(QTableWidget* tableWidget, FixUnitType unitType, Planet* planet, int& row, int column);

    void buildResumeOutputs(QTableWidget* tableWidget);
    void buildRentaOutputs(QTableWidget* tableWidget);
    void buildGeneralImputs(QTableWidget* tableWidget, int column);
    void buildResearchImputs(QTableWidget* tableWidget, int column);
    void buildSpecialisationImputs(QTableWidget* tableWidget, int column);
    void buildTradeImputs(QTableWidget* tableWidget, int column);
    void buildPlanetImputs(QTableWidget* tableWidget, int column);
    void buildDiscoveryImputs(QTableWidget* tableWidget);
    void buildPlanification(QTableWidget* tableWidget, int indexPlanif);

private slots:
    void onPlanetsChanged();
    void onTechChanged();
    void onRentaChanged();
    void onPlanifChanged();

signals:
    void planetsChanged();
    void techChanged();
    void rentaChanged();
    void planifChanged();

private:
    Ui::MainWindow *ui;
    QVector<Item*> _items;
    QTableWidget* _generalTable;
    QTableWidget* _rentaTable;
    QTableWidget* _planetTable;
    QTableWidget* _overviewTable;
    QTableWidget* _discoveryTab;
    QTableWidget* _planificationTab;
};
