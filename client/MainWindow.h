#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void sendCommand();
    void onResponse(const QString& response);
    void onError(const QString& error);

private:
    QTableWidget *historyTable;
    QLabel *statusLabel;
    QLineEdit *commandEdit;
    QPushButton *sendButton;
    void addHistoryEntry(const QString& command, const QString& response);
};

#endif
