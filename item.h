#pragma once

#include <functional>

#include <QWidget>
#include <QSpinBox>

namespace Ui {
class Item;
}

class Item : public QWidget
{
    Q_OBJECT

public:
    explicit Item(QString name, QWidget *parent = nullptr);
    ~Item();

    int value() const;
    void setValue(int v);
    void setMinValue(int min);
    void setMaxValue(int max);

    void setOnValueChanged(std::function<void(int)> callback);

private slots:
    void onSpinValueChanged(int value);

private:
    Ui::Item *ui;
    QSpinBox* _spinBox;
    std::function<void(int)> m_onValueChanged;
};
