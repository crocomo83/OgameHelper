#include "lineEditItem.h"

LineEditItem::LineEditItem(const QString& labelText, QWidget* parent)
    : QWidget(parent)
{
    _label = new QLabel(labelText, this);
    _lineEdit = new QLineEdit(this);

    _label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    _lineEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(_label);
    layout->addWidget(_lineEdit);

    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    setLayout(layout);
}

QString LineEditItem::getValue() const
{
    return _lineEdit->text();
}

void LineEditItem::setValue(QString value)
{
    _lineEdit->setText(value);
    this->updateGeometry();
}
