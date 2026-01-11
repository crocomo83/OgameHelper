#pragma once

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>

class LineEditItem : public QWidget
{
    Q_OBJECT

public:
    LineEditItem(const QString& labelText, QWidget* parent);

    QString getValue() const;
    void setValue(QString value);

private:
    QLabel* _label;
    QLineEdit* _lineEdit;
};
