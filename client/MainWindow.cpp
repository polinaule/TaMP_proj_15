#include "MainWindow.h"
#include "ClientCore.h"
#include <QHeaderView>
#include <QRegExp>
#include "AdminDialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Crypto Client");
    resize(800, 400);

    // Центральный виджет
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *layout = new QVBoxLayout(central);

    // Таблица истории
    historyTable = new QTableWidget(0, 2, this);
    historyTable->setHorizontalHeaderLabels({"Command", "Response"});
    historyTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(historyTable);

    // Строка ввода команды
    commandEdit = new QLineEdit(this);
    commandEdit->setPlaceholderText("Enter a command (for example, REGISTER alice pass)");
    layout->addWidget(commandEdit);

    // Кнопка отправки
    sendButton = new QPushButton("Send", this);
    layout->addWidget(sendButton);

    // Метка статуса
    statusLabel = new QLabel("Ready", this);
    layout->addWidget(statusLabel);

    // Соединения
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendCommand);
    connect(&ClientCore::getInstance(), &ClientCore::responseReceived, this, &MainWindow::onResponse);
    connect(&ClientCore::getInstance(), &ClientCore::errorOccurred, this, &MainWindow::onError);

    // Кнопка администрирование
    adminButton = new QPushButton("Administration", this);
    layout->addWidget(adminButton);
    adminButton->hide();
    connect(adminButton, &QPushButton::clicked, this, &MainWindow::openAdminDialog);
}

MainWindow::~MainWindow() {}

void MainWindow::sendCommand() {
    QString cmd = commandEdit->text().trimmed();
    if (cmd.isEmpty()) return;
    addHistoryEntry(cmd, "..." );
    statusLabel->setText("Sending...");
    ClientCore::getInstance().sendCommand(cmd);
    commandEdit->clear();
}

void MainWindow::onResponse(const QString& response) {
    statusLabel->setText("Ready");
    // Обновить последнюю строку в таблице (ответ)
    int row = historyTable->rowCount() - 1;
    if (row >= 0) {
        historyTable->setItem(row, 1, new QTableWidgetItem(response));
        historyTable->resizeRowToContents(row);
    }
    else {
        addHistoryEntry("", response);
    }
    // Отдельно разбираем логин и пароль
    static QString lastCommand;
    if (historyTable->rowCount() > 0) {
        lastCommand = historyTable->item(historyTable->rowCount() - 1, 0)->text();
    }
    if (lastCommand.startsWith("LOGIN") && response.startsWith("OK")) {
        // Извлечь роль
        int pos = response.indexOf("role=");
        if (pos != -1) {
            QString role = response.mid(pos + 5).split(' ').first().trimmed();
            ClientCore::getInstance().setCurrentRole(role);
            // Сохраняем логин и пароль из команды
            QStringList parts = lastCommand.split(' ');
            if (parts.size() >= 3) {
                ClientCore::getInstance().setCurrentLogin(parts[1]);
                ClientCore::getInstance().setCurrentPassword(parts[2]);   // сохраняем пароль
            }
            statusLabel->setText("Logged in as " + role);
            if (role == "admin") {
                addAdminButton();
            }
        }
    }
}

void MainWindow::onError(const QString& error) {
    statusLabel->setText("Error: " + error);
    addHistoryEntry("", "Error: " + error);
}

void MainWindow::addHistoryEntry(const QString& command, const QString& response) {
    int row = historyTable->rowCount();
    historyTable->insertRow(row);
    historyTable->setItem(row, 0, new QTableWidgetItem(command));
    historyTable->setItem(row, 1, new QTableWidgetItem(response));
    historyTable->resizeRowToContents(row);
    historyTable->scrollToBottom();
}

void MainWindow::addAdminButton() {
    if (adminButton) adminButton->show();
}

void MainWindow::openAdminDialog() {
    if (!ClientCore::getInstance().isAdmin()) {
        statusLabel->setText("Access denied: insufficient rights");
        return;
    }
    AdminDialog* dialog = new AdminDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}