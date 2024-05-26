#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>
#include <atomic>
#include "Client.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sendButton_clicked();
    void on_usernameLineEdit_textChanged(const QString &text);
    void checkForResponses();

private:
    Ui::MainWindow *ui;
    Client *client;
    std::thread responseThread;
    std::atomic<bool> running;
};

#endif // MAINWINDOW_H
