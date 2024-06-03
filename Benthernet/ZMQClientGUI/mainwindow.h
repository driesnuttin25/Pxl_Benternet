#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>
#include <atomic>
#include "Client.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**********************************
*  Main Window Class Declaration
**********************************/
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // Constructor and destructor
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // New register button
    void on_registerButton_clicked();

    // Slot to handle send button click
    void on_sendButton_clicked();

    // Slot to handle username input change
    void on_usernameLineEdit_textChanged(const QString &text);

    // Method to check for responses from the server
    void checkForResponses();

private:
    Ui::MainWindow *ui;
    Client *client;           // Pointer to the client
    std::thread responseThread; // Thread for checking responses
    std::atomic<bool> running;  // Flag to control the response thread
};

#endif // MAINWINDOW_H
