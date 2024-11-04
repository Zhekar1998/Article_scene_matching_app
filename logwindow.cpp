#include "logwindow.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QMessageBox>

LogWindow::LogWindow(QWidget *parent) : QMainWindow(parent), tableWidget(new QTableWidget(this))
{
    setWindowTitle("Log Data Analysis");
    setMinimumSize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tableWidget);
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    tableWidget->setColumnCount(4);
    tableWidget->setHorizontalHeaderLabels({"File Name", "Method", "Keypoints", "Processing Time (ns)"});
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void LogWindow::loadLogData(const QString &filePath)
{
    parseLogFile(filePath);
    calculateStatistics();
}

void LogWindow::parseLogFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Unable to open log file.");
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(" ");

        if (parts.size() < 7) continue;

        ProcessedData data;
        data.method = parts[0]; // SIFT, SURF, ORB
        data.fileName = parts[2];
        data.keypoints = parts[4].toInt();
        data.processingTime = parts[6].toInt();
        logData.append(data);

        // Add row to table
        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);
        tableWidget->setItem(row, 0, new QTableWidgetItem(data.fileName));
        tableWidget->setItem(row, 1, new QTableWidgetItem(data.method));
        tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(data.keypoints)));
        tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(data.processingTime)));
    }
}

void LogWindow::calculateStatistics()
{
    // Calculate average keypoints, find files with zero or low keypoints
    int siftTotal = 0, surfTotal = 0, orbTotal = 0;
    int siftCount = 0, surfCount = 0, orbCount = 0;
    int zeroOrLowCount = 0;

    for (const auto &data : logData) {
        if (data.keypoints <= 5) { // Threshold for "low keypoints"
            zeroOrLowCount++;
        }

        if (data.method == "SIFT") {
            siftTotal += data.keypoints;
            siftCount++;
        } else if (data.method == "SURF") {
            surfTotal += data.keypoints;
            surfCount++;
        } else if (data.method == "ORB") {
            orbTotal += data.keypoints;
            orbCount++;
        }
    }

    double avgSIFT = siftCount ? siftTotal / static_cast<double>(siftCount) : 0;
    double avgSURF = surfCount ? surfTotal / static_cast<double>(surfCount) : 0;
    double avgORB = orbCount ? orbTotal / static_cast<double>(orbCount) : 0;

    QMessageBox::information(this, "Statistics",
                             QString("Average Keypoints:\nSIFT: %1\nSURF: %2\nORB: %3\n\nFiles with Low Keypoints: %4")
                                 .arg(avgSIFT).arg(avgSURF).arg(avgORB).arg(zeroOrLowCount));
}
