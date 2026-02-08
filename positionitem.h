#pragma once

#include <functional>

#include <QWidget>
#include <QSpinBox>
#include <QHBoxLayout>

class PositionItem : public QWidget
{
    Q_OBJECT

public:
    explicit PositionItem(QWidget *parent = nullptr);

    void setValue(int g, int s, int p);
    void setOnValueChanged(std::function<void(int, int, int)> callback);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onValueChanged(int value);

private:
    QSpinBox* _galaxy;
    QSpinBox* _solarSystem;
    QSpinBox* _position;
    std::function<void(int, int, int)> m_onValueChanged;
};
