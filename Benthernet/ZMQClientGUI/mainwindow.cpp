#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , client(new Client("tcp://benternet.pxl-ea-ict.be:24041", "tcp://benternet.pxl-ea-ict.be:24042"))
{
    ui->setupUi(this);
    ui->comboBox->addItem("Spelling Checker");
    ui->comboBox->addItem("Random Sentence Generator");

    // Connect the send button's clicked signal to the on_sendButton_clicked slot
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::on_sendButton_clicked);
    connect(ui->usernameLineEdit, &QLineEdit::textChanged, this, &MainWindow::on_usernameLineEdit_textChanged);
}

MainWindow::~MainWindow() {
    delete ui;
    delete client;
}

void MainWindow::on_usernameLineEdit_textChanged(const QString &text) {
    // Enable or disable the message input field based on the username input
    ui->lineEdit->setEnabled(!text.isEmpty());
}

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
    std::string response = client->receiveResponse();

    // Clear previous output
    ui->textEdit->clear();

    // Process the response to strip the prefix and display only the actual output
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
    // Display the cleaned response
    ui->textEdit->setText(QString::fromStdString(output));
}
