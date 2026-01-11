#include "comboboxitem.h"
#include "ui_comboboxitem.h"

ComboBoxItem::ComboBoxItem(QString str, QStringList names, QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::ComboBoxItem;
    ui->setupUi(this);
    ui->label->setText(str);
    ui->comboBox->addItems(names);

    _comboBox = ui->comboBox;

    connect(_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ComboBoxItem::onComboBoxChanged);
}

ComboBoxItem::~ComboBoxItem()
{
    delete ui;
}

void ComboBoxItem::setValue(int index)
{
    ui->comboBox->setCurrentIndex(index);
}

int ComboBoxItem::getCurrentIndex() const
{
    return ui->comboBox->currentIndex();
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
