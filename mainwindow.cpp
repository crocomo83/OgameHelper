#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QVBoxLayout *layout = new QVBoxLayout;
    QStringList itemNames = {"Ressource A", "Ressource B", "Ressource C"};

    for (const QString &name : itemNames) {
        Item *item = new Item(name);
        layout->addWidget(item);
        _items.append(item); // pour accéder plus tard
    }

    ui->common->setLayout(layout);
}

MainWindow::~MainWindow()
{
    delete ui;
}
