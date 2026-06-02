#include "AdminDialog.h"
#include "ClientCore.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>

AdminDialog::AdminDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Administration");
    resize(600, 400);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    table = new QTableWidget(0, 3, this);
    table->setHorizontalHeaderLabels({ "ID", "Login", "Role" });
    table->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(table);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    refreshBtn = new QPushButton("Update", this);
    deleteBtn = new QPushButton("Delete selected", this);
    roleBtn = new QPushButton("Change role", this);
    roleEdit = new QLineEdit(this);
    roleEdit->setPlaceholderText("user или admin");
    btnLayout->addWidget(refreshBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(roleEdit);
    btnLayout->addWidget(roleBtn);
    mainLayout->addLayout(btnLayout);

    connect(refreshBtn, &QPushButton::clicked, this, &AdminDialog::refreshUsers);
    connect(deleteBtn, &QPushButton::clicked, this, &AdminDialog::deleteUser);
    connect(roleBtn, &QPushButton::clicked, this, &AdminDialog::changeRole);
    connect(&ClientCore::getInstance(), &ClientCore::responseReceived,
        this, &AdminDialog::onResponse);

    // «агружаем список пользователей при открытии
    refreshUsers();
}

AdminDialog::~AdminDialog() {}

void AdminDialog::loadUsers() {
    QString login = ClientCore::getInstance().getCurrentLogin();
    QString password = ClientCore::getInstance().getCurrentPassword();
    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "You are not logged in as an administrator");
        return;
    }
    QString cmd = QString("ADMIN_GET_USERS %1 %2").arg(login).arg(password);
    ClientCore::getInstance().sendCommand(cmd);
}

void AdminDialog::refreshUsers() {
    loadUsers();
}

void AdminDialog::onResponse(const QString& response) {
    if (response.startsWith("OK Users:")) {
        // –азбираем ответ: перва€ строка "OK Users:", далее строки "id login role"
        QStringList lines = response.split('\n');
        QStringList usersData;
        for (int i = 1; i < lines.size(); ++i) {
            QString line = lines[i].trimmed();
            if (!line.isEmpty())
                usersData.append(line);
        }
        updateTable(usersData);
    }
    else if (response.startsWith("OK User ") || response.startsWith("OK Role of ")) {
        // ѕосле успешного удалени€ или смены роли обновл€ем таблицу
        refreshUsers();
    }
    else if (response.startsWith("ERROR")) {
        QMessageBox::warning(this, "Error", response);
    }
}

void AdminDialog::updateTable(const QStringList& usersData) {
    table->setRowCount(0);
    for (const QString& line : usersData) {
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 3) continue;
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(parts[0]));
        table->setItem(row, 1, new QTableWidgetItem(parts[1]));
        table->setItem(row, 2, new QTableWidgetItem(parts[2]));
    }
    table->resizeColumnsToContents();
}

void AdminDialog::deleteUser() {
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Deletion", "Select a user in the table");
        return;
    }
    QString login = table->item(row, 1)->text();
    QString adminLogin = ClientCore::getInstance().getCurrentLogin();
    QString adminPass = ClientCore::getInstance().getCurrentPassword();
    if (login == adminLogin) {
        QMessageBox::warning(this, "Error", "You can't delete yourself");
        return;
    }
    QString cmd = QString("ADMIN_DELETE_USER %1 %2 %3").arg(adminLogin).arg(adminPass).arg(login);
    ClientCore::getInstance().sendCommand(cmd);
}

void AdminDialog::changeRole() {
    int row = table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Role change", "Select a user in the table");
        return;
    }
    QString login = table->item(row, 1)->text();
    QString newRole = roleEdit->text().trimmed();
    if (newRole != "user" && newRole != "admin") {
        QMessageBox::warning(this, "Error", "The role must be 'user' or 'admin'");
        return;
    }
    QString adminLogin = ClientCore::getInstance().getCurrentLogin();
    QString adminPass = ClientCore::getInstance().getCurrentPassword();
    QString cmd = QString("ADMIN_SET_ROLE %1 %2 %3 %4")
        .arg(adminLogin).arg(adminPass).arg(login).arg(newRole);
    ClientCore::getInstance().sendCommand(cmd);
    roleEdit->clear();
}