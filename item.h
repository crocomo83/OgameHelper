#pragma once

#include <QWidget>

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

private:
    Ui::Item *ui;
};
