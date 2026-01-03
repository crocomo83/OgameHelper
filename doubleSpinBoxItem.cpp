#include "doubleSpinBoxItem.h"
#include "ui_doubleSpinBoxItem.h"

DoubleSpinBoxItem::DoubleSpinBoxItem(QString name, QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::DoubleSpinBoxItem;
    ui->setupUi(this);
    ui->label->setText(name);
    this->updateGeometry();

    _doubleSpinBox = ui->doubleSpinBox;
    _doubleSpinBox->setValue(0.0);

    connect(_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DoubleSpinBoxItem::onSpinValueChanged);
}

DoubleSpinBoxItem::~DoubleSpinBoxItem()
{
    delete ui;
}

double DoubleSpinBoxItem::value() const
{
    return ui->doubleSpinBox->value();
}

void DoubleSpinBoxItem::setValue(double value)
{
    ui->doubleSpinBox->setValue(value);
    this->updateGeometry();
}

void DoubleSpinBoxItem::setMinValue(double min)
{
    ui->doubleSpinBox->setMinimum(min);

    double value = ui->doubleSpinBox->value();
    if (value < min)
    {
        setValue(min);
    }
}

void DoubleSpinBoxItem::setMaxValue(double max)
{
    ui->doubleSpinBox->setMaximum(max);

    double value = ui->doubleSpinBox->value();
    if (value > max)
    {
        setValue(max);
    }
}

void DoubleSpinBoxItem::onSpinValueChanged(double value)
{
    if (m_onValueChanged) {
        m_onValueChanged(value);
    }
}

void DoubleSpinBoxItem::setOnValueChanged(std::function<void(double)> callback)
{
    m_onValueChanged = std::move(callback);
}
