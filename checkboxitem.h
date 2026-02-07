#pragma once

#include <QLabel>
#include <QWidget>
#include <QCheckBox>

class CheckBoxItem : public QWidget
{
    Q_OBJECT

public:
    explicit CheckBoxItem(QString name, QWidget *parent = nullptr);

    bool value() const;
    void setValue(bool b);

    void setOnValueChanged(std::function<void(bool)> callback);

private slots:
    void onCheckBoxChanged(bool value);

private:
    QLabel* _label;
    QCheckBox* _checkBox;
    std::function<void(bool)> m_onValueChanged;
};
