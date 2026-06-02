#ifndef ADMINDIALOG_H
#define ADMINDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>

class AdminDialog : public QDialog {
    Q_OBJECT
public:
    explicit AdminDialog(QWidget* parent = nullptr);
    virtual ~AdminDialog();

private slots:
    void refreshUsers();
    void deleteUser();
    void changeRole();
    void onResponse(const QString& response);

private:
    QTableWidget* table;
    QPushButton* refreshBtn, * deleteBtn, * roleBtn;
    QLineEdit* roleEdit;
    void loadUsers();            // отправляет запрос на сервер
    void updateTable(const QStringList& usersData); // парсит ответ и заполняет таблицу
};

#endif