#pragma once

#include <QWidget>
#include <QComboBox>

namespace Ui {
class ComboBoxItem;
}

class ComboBoxItem : public QWidget
{
    Q_OBJECT

public:
    explicit ComboBoxItem(QString str, QStringList names, QWidget *parent = nullptr);
    ~ComboBoxItem();

    void setOnValueChanged(std::function<void(int)> callback);
    int getCurrentIndex() const;
    void setValue(int index);

private slots:
    void onComboBoxChanged(int index);

private:
    Ui::ComboBoxItem *ui;
    QComboBox* _comboBox;
    std::function<void(int)> m_onValueChanged;
};
