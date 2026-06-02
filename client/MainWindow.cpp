#include "MainWindow.h"
#include "ClientCore.h"
#include <QHeaderView>

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
    historyTable->setHorizontalHeaderLabels({"Команда", "Ответ"});
    historyTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(historyTable);

    // Строка ввода команды
    commandEdit = new QLineEdit(this);
    commandEdit->setPlaceholderText("Введите команду (например, REGISTER alice pass)");
    layout->addWidget(commandEdit);

    // Кнопка отправки
    sendButton = new QPushButton("Отправить", this);
    layout->addWidget(sendButton);

    // Метка статуса
    statusLabel = new QLabel("Готов", this);
    layout->addWidget(statusLabel);

    // Соединения
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendCommand);
    connect(&ClientCore::getInstance(), &ClientCore::responseReceived, this, &MainWindow::onResponse);
    connect(&ClientCore::getInstance(), &ClientCore::errorOccurred, this, &MainWindow::onError);
}

MainWindow::~MainWindow() {}

void MainWindow::sendCommand() {
    QString cmd = commandEdit->text().trimmed();
    if (cmd.isEmpty()) return;
    addHistoryEntry(cmd, "..." );
    statusLabel->setText("Отправка...");
    ClientCore::getInstance().sendCommand(cmd);
    commandEdit->clear();
}

void MainWindow::onResponse(const QString& response) {
    statusLabel->setText("Готов");
    // Обновить последнюю строку в таблице (ответ)
    int row = historyTable->rowCount() - 1;
    if (row >= 0) {
        historyTable->setItem(row, 1, new QTableWidgetItem(response));
        historyTable->resizeRowToContents(row);
    } else {
        addHistoryEntry("", response);
    }
}

void MainWindow::onError(const QString& error) {
    statusLabel->setText("Ошибка: " + error);
    addHistoryEntry("", "Ошибка: " + error);
}

void MainWindow::addHistoryEntry(const QString& command, const QString& response) {
    int row = historyTable->rowCount();
    historyTable->insertRow(row);
    historyTable->setItem(row, 0, new QTableWidgetItem(command));
    historyTable->setItem(row, 1, new QTableWidgetItem(response));
    historyTable->resizeRowToContents(row);
    historyTable->scrollToBottom();
}
