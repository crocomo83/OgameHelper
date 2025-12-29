#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QTableWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->research->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    _researchHandler = new ResearchHandler();
    int numberOfResearch = _researchHandler->getNumberResearch();

    for (int i = 0; i < numberOfResearch; ++i) {
        const ResearchHandler::Research& research = _researchHandler->getResearch(static_cast<ResearchType>(i));

        qDebug() << "item set : " << research.name;

        Item *item = new Item(research.name);
        ui->research->setCellWidget(i, 0, item);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
