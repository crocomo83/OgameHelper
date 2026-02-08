#pragma once

#include <functional>

#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QHBoxLayout>

class Item : public QWidget
{
    Q_OBJECT

public:
    explicit Item(QString name, QWidget *parent = nullptr);

    int value() const;
    void setValue(int v);
    void setMinValue(int min);
    void setMaxValue(int max);

    void setOnValueChanged(std::function<void(int)> callback);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onSpinValueChanged(int value);

private:
    QLabel* _label;
    QSpinBox* _spinBox;
    std::function<void(int)> m_onValueChanged;
};
