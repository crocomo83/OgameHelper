#include "item.h"

#include <QEvent>

Item::Item(QString name, QWidget *parent)
    : QWidget(parent)
{
    _label = new QLabel(name, this);
    _spinBox = new QSpinBox(this);

    _label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _spinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(_label);
    layout->addWidget(_spinBox);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    setLayout(layout);

    _spinBox->setValue(0);
    _spinBox->installEventFilter(this);

    connect(_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Item::onSpinValueChanged);
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

bool Item::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Wheel)
    {
        auto *w = qobject_cast<QWidget*>(obj);
        if (w)
        {
            event->ignore();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}
