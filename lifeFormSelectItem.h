#pragma once

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>

class LifeFormSelectItem : public QWidget
{
    Q_OBJECT

public:
    explicit LifeFormSelectItem(QStringList names, QWidget *parent = nullptr);

    int getCurrentIndex() const;
    int getCurrentLevel() const;
    void setIndex(int index);
    void setLevel(int level);
    void setOnIndexChanged(std::function<void(int)> callback);
    void setOnLevelChanged(std::function<void(int)> callback);
    void setEnabled(bool enabled);

private slots:
    void onComboBoxChanged(int index);
    void onSpinValueChanged(int value);

private:
    QComboBox* _comboBox;
    QSpinBox* _spinBox;
    std::function<void(int)> m_onIndexChanged;
    std::function<void(int)> m_onLevelChanged;
};
