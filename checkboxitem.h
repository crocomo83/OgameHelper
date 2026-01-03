#pragma once

#include <QWidget>
#include <QCheckBox>

namespace Ui {
class CheckBoxItem;
}

class CheckBoxItem : public QWidget
{
    Q_OBJECT

public:
    explicit CheckBoxItem(QString name, QWidget *parent = nullptr);
    ~CheckBoxItem();

    bool value() const;
    void setValue(bool b);

    void setOnValueChanged(std::function<void(bool)> callback);

private slots:
    void onCheckBoxChanged(bool value);

private:
    Ui::CheckBoxItem *ui;
    QCheckBox* _checkBox;
    std::function<void(bool)> m_onValueChanged;
};
