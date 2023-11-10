#include "MainWindow.h"
#include "GausBlur.h"
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QImage>
#include <QDebug>
#include <QThread>
#include <QSettings>


MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
    ui.setupUi(this);
    loadSettings();

    showMaximized();

    connect(ui.radius, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::saveSettings);

    connect(ui.open, &QAction::triggered, this, &MainWindow::open);
    connect(ui.save, &QAction::triggered, this, &MainWindow::save);
    connect(ui.exit, &QAction::triggered, this, &MainWindow::exit);
    connect(ui.action, &QAction::triggered, this, &MainWindow::showAbout);

    connect(ui.done, &QPushButton::clicked, this, &MainWindow::start);

}


MainWindow::~MainWindow()
{
}


void MainWindow::saveSettings() {
    QSettings settings("Gaus", "App");
    settings.setValue("sigmaValue", ui.radius->value());
}


void MainWindow::loadSettings() {
    QSettings settings("Gaus", "App");
    double savedSigma = settings.value("sigmaValue", 0.00).toDouble();

    ui.radius->setValue(savedSigma);
}


void MainWindow::open()
{
    image_path = QFileDialog::getOpenFileName(this, "Select image", "", "Images (*.bmp *.jpg *.png )");
    if (!image_path.isEmpty()) {
        QPixmap Pixmap(image_path);

        ui.clear_image->setPixmap(Pixmap);
        ui.gaus_image->clear();
        blur_image = false;
    }
}

void MainWindow::save()
{
    if (blur_image) {
        image_path_save = QFileDialog::getSaveFileName(this, "Save image", "", "Images (*.bmp *.jpg *.png )");

        QString format = QFileInfo(image_path_save).suffix();

        if (!image_path_save.isEmpty()) {
            QImageWriter writer(image_path_save, format.toUtf8());

            if (resultImage.save(image_path_save, format.toUtf8())) {
                qDebug() << "Image saved successfully!";
            }
            else {
                // Обработка ошибок записи
                qDebug() << "Error writing image";
            }
        }
    }
    else {
        QMessageBox infoWindow;
        infoWindow.setText("Processed image not found");
        infoWindow.exec();
    }
}


void MainWindow::exit()
{
    QApplication::quit();
}

void MainWindow::showAbout()
{
    QMessageBox infoWindow;
    infoWindow.setText("Gaussian blur\nThe program uses Gaussian blur for image processing\nVersion: 0.5\nDmitriev Nikolay");
    infoWindow.exec();
}

int MainWindow::start()
{
    if (blur_image && sigma == ui.radius->value()) {
        QMessageBox infoWindow;
        infoWindow.setText("Image already blur");
        infoWindow.exec();
        return 1;
    }

    QImage image(image_path);
    if (image.isNull()) {
        QMessageBox infoWindow;
        infoWindow.setText("Image not found");
        infoWindow.exec();
        return 1;
    }

    sigma = ui.radius->value();

    GausWindow* Window = new GausWindow(image, sigma, this);
    Window->start();
    connect(Window, &GausWindow::resultReady, this, &MainWindow::result);
    Window->exec();

    return 0;
}


void MainWindow::result(QImage result) {
    resultImage = result;
    ui.gaus_image->setPixmap(QPixmap::fromImage(result));
    blur_image = true;
}
