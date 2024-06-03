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
}

// Destructor: Cleans up resources
MainWindow::~MainWindow() {
    running = false;
    if (responseThread.joinable()) {
        responseThread.join();
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
        QString limit = ui->limit->text(); // Get the limit from the QLineEdit
        if (serviceType == "Spelling Checker") {
            // Ok so I added a limit error here because it was interfering with the help as it would need the user to enter a limit for the -help request
            if (limit.isEmpty()) {
                QMessageBox::warning(this, "Input Error", "Please enter the limit.");
                return;
            }
            request = "spellingschecker<" + username.toStdString() + "<" + limit.toStdString() + "<" + input.toStdString() + ">";
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
                // Process the output to handle the correct response format
                std::string output = response;
                size_t startPos = response.find("response<");
                if (startPos != std::string::npos) {
                    startPos = response.find('<', startPos + 9); // Skip "response<"
                    if (startPos != std::string::npos) {
                        size_t serviceTypePos = response.find('<', startPos + 1);
                        if (serviceTypePos != std::string::npos) {
                            std::string serviceType = response.substr(startPos + 1, serviceTypePos - startPos - 1);
                            if (serviceType == "correctspelling") {
                                // Handle help response
                                if (response.find("Spell Checker Service:") != std::string::npos) {
                                    output = response.substr(serviceTypePos + 1);
                                    if (!output.empty() && output.back() == '>') {
                                        output.pop_back();
                                    }
                                } else {
                                    // Handle regular correct spelling response
                                    size_t userNamePos = response.find('<', serviceTypePos + 1);
                                    size_t limitPos = response.find('<', userNamePos + 1);
                                    if (userNamePos != std::string::npos && limitPos != std::string::npos) {
                                        output = response.substr(limitPos + 1);
                                        if (!output.empty() && output.back() == '>') {
                                            output.pop_back();
                                        }
                                    }
                                }
                            } else {
                                // Handle other service types (e.g., randomsentence)
                                output = response.substr(serviceTypePos + 1);
                                if (!output.empty() && output.back() == '>') {
                                    output.pop_back();
                                }
                            }
                        }
                    }
                }

                // Display the cleaned response in the text edit
                QString qResponse = QString::fromStdString(output);
                QMetaObject::invokeMethod(ui->textEdit, "setText", Qt::QueuedConnection, Q_ARG(QString, qResponse));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Polling interval
    }
}
