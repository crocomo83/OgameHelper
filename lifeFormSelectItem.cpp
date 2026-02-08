#include "lifeFormSelectItem.h"

#include <QHBoxLayout>
#include <QEvent>

LifeFormSelectItem::LifeFormSelectItem(QStringList names, QWidget *parent)
    : QWidget(parent)
{
    _comboBox = new QComboBox(this);
    _comboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    _comboBox->addItems(names);
    _comboBox->setCurrentIndex(0);
    _comboBox->installEventFilter(this);

    _spinBox = new QSpinBox(this);
    _spinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    _spinBox->installEventFilter(this);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(_comboBox);
    layout->addWidget(_spinBox);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    setLayout(layout);

    connect(_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LifeFormSelectItem::onComboBoxChanged);
    connect(_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &LifeFormSelectItem::onSpinValueChanged);
}

int LifeFormSelectItem::getCurrentIndex() const
{
    return _comboBox->currentIndex();
}

int LifeFormSelectItem::getCurrentLevel() const
{
    return _spinBox->value();
}

void LifeFormSelectItem::setIndex(int index)
{
    _comboBox->setCurrentIndex(index);
}

void LifeFormSelectItem::setLevel(int level)
{
    _spinBox->setValue(level);
}

void LifeFormSelectItem::onComboBoxChanged(int index)
{
    if (m_onIndexChanged) {
        m_onIndexChanged(index);
    }
}

void LifeFormSelectItem::onSpinValueChanged(int value)
{
    if (m_onLevelChanged) {
        m_onLevelChanged(value);
    }
}

void LifeFormSelectItem::setOnIndexChanged(std::function<void(int)> callback)
{
    m_onIndexChanged = std::move(callback);
}

void LifeFormSelectItem::setOnLevelChanged(std::function<void(int)> callback)
{
    m_onLevelChanged = std::move(callback);
}

void LifeFormSelectItem::setEnabled(bool enabled)
{
    _spinBox->setEnabled(enabled);
}

bool LifeFormSelectItem::eventFilter(QObject *obj, QEvent *event)
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
