#include "positionitem.h"

PositionItem::PositionItem(QWidget *parent)
    : QWidget(parent)
{
    _galaxy = new QSpinBox(this);
    _solarSystem = new QSpinBox(this);
    _position = new QSpinBox(this);

    _galaxy->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    _solarSystem->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    _position->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(_galaxy);
    layout->addWidget(_solarSystem);
    layout->addWidget(_position);

    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    setLayout(layout);

    _galaxy->setValue(1);
    _galaxy->setMinimum(1);
    _galaxy->setMaximum(5);

    _solarSystem->setValue(1);
    _solarSystem->setMinimum(1);
    _solarSystem->setMaximum(499);

    _position->setValue(1);
    _position->setMinimum(1);
    _position->setMaximum(15);

    connect(_galaxy, QOverload<int>::of(&QSpinBox::valueChanged), this, &PositionItem::onValueChanged);
    connect(_solarSystem, QOverload<int>::of(&QSpinBox::valueChanged), this, &PositionItem::onValueChanged);
    connect(_position, QOverload<int>::of(&QSpinBox::valueChanged), this, &PositionItem::onValueChanged);
}

void PositionItem::onValueChanged(int /*unused*/)
{
    if (m_onValueChanged) {
        m_onValueChanged(
            _galaxy->value(),
            _solarSystem->value(),
            _position->value()
            );
    }
}

void PositionItem::setValue(int g, int s, int p)
{
    _galaxy->setValue(g);
    _solarSystem->setValue(s);
    _position->setValue(p);
}

void PositionItem::setOnValueChanged(std::function<void(int, int, int)> callback)
{
    m_onValueChanged = std::move(callback);
}
