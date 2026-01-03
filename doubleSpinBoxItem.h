#pragma once

#include <QWidget>
#include <QDoubleSpinBox>

namespace Ui {
class DoubleSpinBoxItem;
}

class DoubleSpinBoxItem : public QWidget
{
    Q_OBJECT

public:
    explicit DoubleSpinBoxItem(QString name, QWidget *parent = nullptr);
    ~DoubleSpinBoxItem();

    double value() const;
    void setValue(double v);
    void setMinValue(double min);
    void setMaxValue(double max);

    void setOnValueChanged(std::function<void(double)> callback);

private slots:
    void onSpinValueChanged(double value);

private:
    Ui::DoubleSpinBoxItem *ui;
    QDoubleSpinBox* _doubleSpinBox;
    std::function<void(double)> m_onValueChanged;
};
