#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logwindow.h"
#include <QFileDialog>
#include <QFileSystemModel>
#include <QMessageBox>
#include <QtConcurrent>
#include <QStandardPaths>
#include <QThread>
#include <QTextStream>
#include <QDir>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , fileTreeModel(new QFileSystemModel(this))
{
    ui->setupUi(this);

    // Set up QLabel inside QScrollArea

    ui->imageLabel->setAlignment(Qt::AlignCenter);

    // Initialize image labels for each tab

    ui->siftImageLabel->setAlignment(Qt::AlignCenter);


    ui->surfImageLabel->setAlignment(Qt::AlignCenter);


    ui->orbImageLabel->setAlignment(Qt::AlignCenter);

    // Set up the file system model for the file tree view
    fileTreeModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    fileTreeModel->setNameFilters({"*.jpg", "*.png", "*.bmp"}); // Only show image files
    fileTreeModel->setNameFilterDisables(false);
    ui->fileTreeView->setModel(fileTreeModel);

    // Connect signals
    connect(ui->selectFolderButton, &QPushButton::clicked, this, &MainWindow::openFolderDialog);
    connect(ui->fileTreeView, &QTreeView::clicked, this, &MainWindow::loadSelectedImage);
    connect(ui->processButton, &QPushButton::clicked, this, &MainWindow::processImages);

    // Initialize the progress bar
    ui->progressBar->setVisible(false);

    LogWindow *logWindow = new LogWindow(this);
    connect(ui->actionOpen_Log_Window, &QAction::triggered, this, [logWindow]() {
        logWindow->loadLogData("out/processing_log.txt");
        logWindow->show();
    });


}

MainWindow::~MainWindow()
{
    delete ui;
}

// Slot to open a folder dialog and set the root path for the file tree
void MainWindow::openFolderDialog()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "Выберите папку с изображениями",
                                                           QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    if (!folderPath.isEmpty()) {
        fileTreeModel->setRootPath(folderPath);
        ui->fileTreeView->setRootIndex(fileTreeModel->index(folderPath));
    }
}

// Slot to load the selected image into the QLabel inside the QScrollArea using QImage
void MainWindow::loadSelectedImage(const QModelIndex &index)
{
    selectedImagePath = fileTreeModel->filePath(index);
    QImage image(selectedImagePath); // Load the image as QImage

    if (image.isNull()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение.");
        return;
    }

    // Display the original image
    ui->imageLabel->setPixmap(QPixmap::fromImage(image.scaled(ui->imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));

    // Check and display processed results if they exist
    displayProcessedImage("out/sift", ui->siftImageLabel);
    displayProcessedImage("out/surf", ui->surfImageLabel);
    displayProcessedImage("out/orb", ui->orbImageLabel);
}
void MainWindow::displayProcessedImage(const QString &folder, QLabel *label)
{
    QString baseFileName = QFileInfo(selectedImagePath).fileName();
    QString processedFilePath = folder + "/" + baseFileName;

    if (QFile::exists(processedFilePath)) {
        QImage processedImage(processedFilePath);
        label->setPixmap(QPixmap::fromImage(processedImage.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    }
}


// Slot to start processing images in multithreaded mode with SURF, SIFT, and ORB
void MainWindow::processImages()
{
    QString folderPath = fileTreeModel->rootPath();
    if (folderPath.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите папку для обработки");
        return;
    }

    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);

    // Set the maximum number of threads to 28
    QThreadPool::globalInstance()->setMaxThreadCount(28);

    QStringList imageFiles = QDir(folderPath).entryList({"*.jpg", "*.png", "*.bmp"}, QDir::Files);
    int totalImages = imageFiles.size();
    int completed = 0;

    // Create output directories for each type of processing
    QDir().mkpath("out/sift");
    QDir().mkpath("out/surf");
    QDir().mkpath("out/orb");

    QThreadPool::globalInstance()->start([=]() mutable {
        QFile logFile("out/processing_log.txt");
        if (!logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
            qWarning() << "Не удалось открыть файл лога!";
            return;
        }
        QTextStream logStream(&logFile);

        for (const QString &fileName : imageFiles) {
            QString filePath = folderPath + "/" + fileName;
            cv::Mat image = cv::imread(filePath.toStdString(), cv::IMREAD_GRAYSCALE);

            if (image.empty()) {
                logStream << "Ошибка загрузки файла: " << fileName << "\n";
                continue;
            }

            QElapsedTimer timer;

            // SIFT processing
            {
                timer.restart();
                cv::Ptr<cv::SIFT> siftDetector = cv::SIFT::create();
                std::vector<cv::KeyPoint> siftKeypoints;
                cv::Mat siftDescriptors;
                siftDetector->detectAndCompute(image, cv::noArray(), siftKeypoints, siftDescriptors);

                int elapsedSIFT = timer.nsecsElapsed();
                cv::Mat siftOutput;
                cv::drawKeypoints(image, siftKeypoints, siftOutput);
                cv::imwrite("out/sift/" + fileName.toStdString(), siftOutput);
                logStream << "SIFT - Processed " << fileName << " with " << siftKeypoints.size()
                          << " keypoints in " << elapsedSIFT << " ns.\n";
            }

            // SURF processing
            {
                timer.restart();
                cv::Ptr<cv::xfeatures2d::SURF> surfDetector = cv::xfeatures2d::SURF::create();
                std::vector<cv::KeyPoint> surfKeypoints;
                cv::Mat surfDescriptors;
                surfDetector->detectAndCompute(image, cv::noArray(), surfKeypoints, surfDescriptors);

                int elapsedSURF = timer.nsecsElapsed();
                cv::Mat surfOutput;
                cv::drawKeypoints(image, surfKeypoints, surfOutput);
                cv::imwrite("out/surf/" + fileName.toStdString(), surfOutput);
                logStream << "SURF - Processed " << fileName << " with " << surfKeypoints.size()
                          << " keypoints in " << elapsedSURF << " ns.\n";
            }

            // ORB processing
            {
                timer.restart();
                cv::Ptr<cv::ORB> orbDetector = cv::ORB::create();
                std::vector<cv::KeyPoint> orbKeypoints;
                cv::Mat orbDescriptors;
                orbDetector->detectAndCompute(image, cv::noArray(), orbKeypoints, orbDescriptors);

                int elapsedORB = timer.nsecsElapsed();
                cv::Mat orbOutput;
                cv::drawKeypoints(image, orbKeypoints, orbOutput);
                cv::imwrite("out/orb/" + fileName.toStdString(), orbOutput);
                logStream << "ORB - Processed " << fileName << " with " << orbKeypoints.size()
                          << " keypoints in " << elapsedORB << " ns.\n";
            }

            // Update progress bar in the UI thread
            completed++;
            int progress = (completed * 100) / totalImages;
            QMetaObject::invokeMethod(this, "updateProgressBar", Qt::QueuedConnection, Q_ARG(int, progress));
        }

        logFile.close();
        QMetaObject::invokeMethod(this, "processingComplete", Qt::QueuedConnection);
    });

}

// Slot to update the progress bar
void MainWindow::updateProgressBar(int value)
{
    ui->progressBar->setValue(value);
}

// Slot to indicate that processing is complete
void MainWindow::processingComplete()
{
    ui->progressBar->setVisible(false);
    QMessageBox::information(this, "Обработка завершена", "Обработка изображений завершена.");

    // Get the current selected index from fileTreeView
    QModelIndex currentIndex = ui->fileTreeView->currentIndex();

    // Call loadSelectedImage to refresh the display, showing processed results if available
    if (currentIndex.isValid()) {
        loadSelectedImage(currentIndex);
    }
}
