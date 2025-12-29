#include "item.h"
#include "ui_item.h"

Item::Item(QString name, QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::Item;
    ui->setupUi(this);
    ui->label->setText(name);
}

Item::~Item()
{
    delete ui;
}

int Item::value() const
{
    return ui->spinBox->value();
}

void Item::setValue(int value)
{
    ui->spinBox->setValue(value);
}
