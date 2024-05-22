#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , client(new Client("tcp://benternet.pxl-ea-ict.be:24041", "tcp://benternet.pxl-ea-ict.be:24042"))
{
    ui->setupUi(this);
    ui->comboBox->addItem("Spelling Checker");
    ui->comboBox->addItem("Random Sentence Generator");
}

MainWindow::~MainWindow() {
    delete ui;
    delete client;
}

void MainWindow::on_sendButton_clicked() {
    QString input = ui->lineEdit->text();
    if (input.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a word or sentence.");
        return;
    }

    QString serviceType = ui->comboBox->currentText();
    std::string request;

    if (serviceType == "Spelling Checker") {
        request = "spellingschecker<" + ui->lineEdit->text().toStdString() + ">";
    } else if (serviceType == "Random Sentence Generator") {
        bool ok;
        int wordCount = input.toInt(&ok);
        if (!ok || wordCount <= 0) {
            QMessageBox::warning(this, "Input Error", "Please enter a valid number of words.");
            return;
        }
        request = "randomsentence<" + std::to_string(wordCount) + ">";
    }

    client->sendRequest(request);
    std::string response = client->receiveResponse();
    ui->textEdit->append(QString::fromStdString(response));
}
