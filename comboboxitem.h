#pragma once

#include <QLabel>
#include <QWidget>
#include <QComboBox>

class ComboBoxItem : public QWidget
{
    Q_OBJECT

public:
    explicit ComboBoxItem(QString str, QStringList names, QWidget *parent = nullptr);

    void setOnValueChanged(std::function<void(int)> callback);
    int getCurrentIndex() const;
    void setValue(int index);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onComboBoxChanged(int index);

private:
    QLabel* _label;
    QComboBox* _comboBox;
    std::function<void(int)> m_onValueChanged;
};
