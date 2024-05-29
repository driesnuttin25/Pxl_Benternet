#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QPixmap>

/**********************************
*  Main Window Class
**********************************/

// Constructor: Initializes UI, sets up the client, and starts the response thread
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , client(new Client("tcp://benternet.pxl-ea-ict.be:24041", "tcp://benternet.pxl-ea-ict.be:24042"))
    , running(true)
{
    ui->setupUi(this);
    ui->comboBox->addItem("Spelling Checker");
    ui->comboBox->addItem("Random Sentence Generator");

    // Load and set the logo image
    QPixmap logo(":/new/prefix1/logo.png");
    ui->logoLabel->setPixmap(logo);
    ui->logoLabel->setScaledContents(true);

    // Connect the send button's clicked signal to the on_sendButton_clicked slot
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::on_sendButton_clicked);
    connect(ui->usernameLineEdit, &QLineEdit::textChanged, this, &MainWindow::on_usernameLineEdit_textChanged);

    // Start the response checking thread
    responseThread = std::thread(&MainWindow::checkForResponses, this);
    heartbeatThread = std::thread(&MainWindow::checkForHeartbeat, this);
}

// Destructor: Cleans up resources
MainWindow::~MainWindow() {
    running = false;
    if (responseThread.joinable()) {
        responseThread.join();
    }
    if (heartbeatThread.joinable()) {
        heartbeatThread.join();
    }
    delete ui;
    delete client;
}

// Slot to handle username input change: Enables or disables the message input field
void MainWindow::on_usernameLineEdit_textChanged(const QString &text) {
    ui->lineEdit->setEnabled(!text.isEmpty());
}

// Slot to handle send button click: Validates input and sends a request to the server
void MainWindow::on_sendButton_clicked() {
    qDebug() << "Send button clicked";  // Debugging message

    QString username = ui->usernameLineEdit->text();
    QString input = ui->lineEdit->text();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter your username.");
        return;
    }

    if (input.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a word or sentence.");
        return;
    }

    QString serviceType = ui->comboBox->currentText();
    std::string request;

    if (input == "-help") {
        if (serviceType == "Spelling Checker") {
            request = "spellingschecker<" + username.toStdString() + "<-help>";
        } else if (serviceType == "Random Sentence Generator") {
            request = "randomsentence<" + username.toStdString() + "<-help>";
        }
    } else {
        if (serviceType == "Spelling Checker") {
            request = "spellingschecker<" + username.toStdString() + "<" + input.toStdString() + ">";
        } else if (serviceType == "Random Sentence Generator") {
            bool ok;
            int wordCount = input.toInt(&ok);
            if (!ok || wordCount <= 0) {
                QMessageBox::warning(this, "Input Error", "Please enter a valid number of words.");
                return;
            }
            request = "randomsentence<" + username.toStdString() + "<" + std::to_string(wordCount) + ">";
        }
    }

    client->sendRequest(request);
}

// Method to check for responses from the server in a separate thread
void MainWindow::checkForResponses() {
    while (running) {
        if (client->isResponseAvailable()) {
            std::string response = client->receiveResponse();
            if (!response.empty()) {
                // This is just to process the output and not have the <response<username<....> infront of it
                std::string output = response;
                size_t startPos = response.find("response<");
                if (startPos != std::string::npos) {
                    startPos = response.find('<', startPos + 9); // Skip "response<"
                    if (startPos != std::string::npos) {
                        startPos = response.find('<', startPos + 1); // Skip the service type
                        if (startPos != std::string::npos) {
                            output = response.substr(startPos + 1);
                        }
                    }
                }
                if (!output.empty() && output.back() == '>') {
                    output.pop_back();
                }

                // Display the cleaned response in the text edit
                QString qResponse = QString::fromStdString(output);
                QMetaObject::invokeMethod(ui->textEdit, "setText", Qt::QueuedConnection, Q_ARG(QString, qResponse));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Polling interval
    }
}

// Method to check for heartbeat messages in a separate thread
void MainWindow::checkForHeartbeat() {
    bool connected = false;
    while (running) {
        if (client->isHeartbeatReceived()) {
            if (!connected) {
                connected = true;
                displayStatus(connected);
            }
        } else {
            if (connected) {
                connected = false;
                displayStatus(connected);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Check interval
    }
}

// Method to display connection status
void MainWindow::displayStatus(bool connected) {
    if (connected) {
        ui->statusLabel->setText("Connected to service");
    } else {
        ui->statusLabel->setText("Disconnected from service");
    }
}
