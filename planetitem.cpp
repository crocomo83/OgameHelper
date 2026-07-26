#include "planetitem.h"

#include <QEvent>

PlanetItem::PlanetItem(const QString& name, QWidget *parent)
    : QWidget(parent)
{
    _nameLineEdit = new QLineEdit(name, this);
    _deleteButton = new QPushButton("Delete", this);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(_nameLineEdit);
    layout->addWidget(_deleteButton);

    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    setLayout(layout);

    connect(_nameLineEdit, QLineEdit::textChanged, this, &PlanetItem::onValueChanged);
    connect(_deleteButton, QPushButton::clicked, this, &PlanetItem::onDelete);
}

void PlanetItem::onDelete()
{
    if (m_onDelete) {
        m_onDelete();
    }
}

void PlanetItem::onValueChanged(const QString &text)
{
    if (m_onValueChanged) {
        m_onValueChanged(text);
    }
}

void PlanetItem::setOnValueChanged(std::function<void(const QString &text)> callback)
{
    m_onValueChanged = std::move(callback);
}

void PlanetItem::setOnDelete(std::function<void()> callback)
{
    m_onDelete = std::move(callback);
}

bool PlanetItem::eventFilter(QObject *obj, QEvent *event)
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
