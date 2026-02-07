#include "comboboxitem.h"

#include <QHBoxLayout>

ComboBoxItem::ComboBoxItem(QString str, QStringList names, QWidget *parent)
    : QWidget(parent)
{
    _label = new QLabel(str, this);
    _comboBox = new QComboBox(this);

    _label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _comboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    _comboBox->addItems(names);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(_label);
    layout->addWidget(_comboBox);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    setLayout(layout);

    _comboBox->setCurrentIndex(0);

    connect(_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ComboBoxItem::onComboBoxChanged);
}

void ComboBoxItem::setValue(int index)
{
    _comboBox->setCurrentIndex(index);
}

int ComboBoxItem::getCurrentIndex() const
{
    return _comboBox->currentIndex();
}

void ComboBoxItem::onComboBoxChanged(int index)
{
    if (m_onValueChanged) {
        m_onValueChanged(index);
    }
}

void ComboBoxItem::setOnValueChanged(std::function<void(int)> callback)
{
    m_onValueChanged = std::move(callback);
}
