#pragma once

#include <QLabel>
#include <QWidget>
#include <QDoubleSpinBox>

class DoubleSpinBoxItem : public QWidget
{
    Q_OBJECT

public:
    explicit DoubleSpinBoxItem(QString name, QWidget *parent = nullptr);

    double value() const;
    void setValue(double v);
    void setMinValue(double min);
    void setMaxValue(double max);

    void setOnValueChanged(std::function<void(double)> callback);

private slots:
    void onSpinValueChanged(double value);

private:
    QLabel* _label;
    QDoubleSpinBox* _doubleSpinBox;
    std::function<void(double)> m_onValueChanged;
};
