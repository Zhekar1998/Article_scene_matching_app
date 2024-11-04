#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QFileSystemModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFolderDialog();
    void loadSelectedImage(const QModelIndex &index);
    void processImages();
    void updateProgressBar(int value);
    void processingComplete();

private:
    Ui::MainWindow *ui;
    QFileSystemModel *fileTreeModel;
    QString selectedImagePath;
    void displayProcessedImage(const QString &folder, QLabel *label);

};

#endif // MAINWINDOW_H
