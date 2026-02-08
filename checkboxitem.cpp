#include "checkboxitem.h"

#include <QHBoxLayout>

CheckBoxItem::CheckBoxItem(QString name, QWidget *parent)
    : QWidget(parent)
{
    _label = new QLabel(name, this);
    _checkBox = new QCheckBox(this);

    _label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _checkBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    _checkBox->setChecked(false);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(_label);
    layout->addWidget(_checkBox);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    setLayout(layout);

    _checkBox->installEventFilter(this);

    connect(_checkBox, QOverload<bool>::of(&QCheckBox::toggled), this, &CheckBoxItem::onCheckBoxChanged);
}

bool CheckBoxItem::value() const
{
    return _checkBox->isChecked();
}

void CheckBoxItem::setValue(bool value)
{
    _checkBox->setChecked(value);
}

void CheckBoxItem::onCheckBoxChanged(bool value)
{
    if (m_onValueChanged) {
        m_onValueChanged(value);
    }
}

void CheckBoxItem::setOnValueChanged(std::function<void(bool)> callback)
{
    m_onValueChanged = std::move(callback);
}
