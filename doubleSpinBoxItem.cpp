#include "doubleSpinBoxItem.h"

#include <QHBoxLayout>

DoubleSpinBoxItem::DoubleSpinBoxItem(QString name, QWidget *parent)
    : QWidget(parent)
{
    _label = new QLabel(name, this);
    _doubleSpinBox = new QDoubleSpinBox(this);

    _label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _doubleSpinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(_label);
    layout->addWidget(_doubleSpinBox);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    setLayout(layout);

    _doubleSpinBox->setValue(0);

    connect(_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DoubleSpinBoxItem::onSpinValueChanged);
}

double DoubleSpinBoxItem::value() const
{
    return _doubleSpinBox->value();
}

void DoubleSpinBoxItem::setValue(double value)
{
    _doubleSpinBox->setValue(value);
    this->updateGeometry();
}

void DoubleSpinBoxItem::setMinValue(double min)
{
    _doubleSpinBox->setMinimum(min);

    double value = _doubleSpinBox->value();
    if (value < min)
    {
        setValue(min);
    }
}

void DoubleSpinBoxItem::setMaxValue(double max)
{
    _doubleSpinBox->setMaximum(max);

    double value = _doubleSpinBox->value();
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
