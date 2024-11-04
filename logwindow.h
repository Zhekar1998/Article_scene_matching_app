#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QMainWindow>
#include <QTableWidget>

class LogWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent = nullptr);
    void loadLogData(const QString &filePath);

private:
    QTableWidget *tableWidget;
    void parseLogFile(const QString &filePath);
    void calculateStatistics();

    // Data structures to store parsed information
    struct ProcessedData {
        QString fileName;
        QString method;
        int keypoints;
        int processingTime;
    };
    QVector<ProcessedData> logData;
};

#endif // LOGWINDOW_H
