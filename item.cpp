#include "item.h"
#include "ui_item.h"

Item::Item(QString name, QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::Item;
    ui->setupUi(this);
    ui->label->setText(name);

    _spinBox = ui->spinBox;
    _spinBox->setValue(0);

    connect(_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Item::onSpinValueChanged);
}

Item::~Item()
{
    delete ui;
}

int Item::value() const
{
    return _spinBox->value();
}

void Item::setValue(int value)
{
    _spinBox->setValue(value);
}

void Item::setMinValue(int min)
{
    _spinBox->setMinimum(min);

    int value = _spinBox->value();
    if (value < min)
    {
        _spinBox->setValue(min);
    }
}

void Item::setMaxValue(int max)
{
    _spinBox->setMaximum(max);

    int value = _spinBox->value();
    if (value > max)
    {
        _spinBox->setValue(max);
    }
}

void Item::onSpinValueChanged(int value)
{
    if (m_onValueChanged) {
        m_onValueChanged(value);
    }
}

void Item::setOnValueChanged(std::function<void(int)> callback)
{
    m_onValueChanged = std::move(callback);
}
