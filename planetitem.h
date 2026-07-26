#pragma once

#include <functional>

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

class PlanetItem : public QWidget
{
    Q_OBJECT

public:
    explicit PlanetItem(const QString &name, QWidget *parent = nullptr);
    void setOnValueChanged(std::function<void (const QString &)> callback);
    void setOnDelete(std::function<void()> callback);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onDelete();
    void onValueChanged(const QString &text);

private:
    QLineEdit* _nameLineEdit;
    QPushButton* _deleteButton;
    std::function<void(const QString &text)> m_onValueChanged;
    std::function<void()> m_onDelete;
};
