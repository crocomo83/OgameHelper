#include "checkboxitem.h"
#include "ui_checkboxitem.h"

CheckBoxItem::CheckBoxItem(QString name, QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::CheckBoxItem;
    ui->setupUi(this);
    ui->label->setText(name);

    _checkBox = ui->checkBox;

    connect(_checkBox, QOverload<bool>::of(&QCheckBox::toggled), this, &CheckBoxItem::onCheckBoxChanged);
}

CheckBoxItem::~CheckBoxItem()
{
    delete ui;
}

bool CheckBoxItem::value() const
{
    return ui->checkBox->isChecked();
}

void CheckBoxItem::setValue(bool value)
{
    ui->checkBox->setChecked(value);
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
