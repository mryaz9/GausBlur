#include "GausBlur.h"

void GausThread::run()
{
    double radius = 3 * sigma; // пиксели на расстоянии более 3σ оказывают на него достаточно малое влияние

    QImage resultImage = QImage(image);

    int width = resultImage.width();
    int height = resultImage.height();

    int totalPixels = width * height;
    int processedPixels = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double red = 0, green = 0, blue = 0, alpha = 0;

            for (int j = -radius; j <= radius; ++j) {
                for (int i = -radius; i <= radius; ++i) {
                    int new_y = max(0, min(height - 1, y + j));
                    int new_x = max(0, min(width - 1, x + i));

                    double weight = 1 / (2 * M_PI * sigma * sigma) * exp(-(i * i + j * j) / (2 * sigma * sigma)); //  произведение двух функций Гаусса, по одной для каждого измерения
                    QRgb pixel = image.pixel(new_x, new_y);

                    red += qRed(pixel) * weight;
                    green += qGreen(pixel) * weight;
                    blue += qBlue(pixel) * weight;
                    alpha += qAlpha(pixel) * weight;
                }
            }

            resultImage.setPixel(x, y, qRgba(static_cast<int>(red), static_cast<int>(green), static_cast<int>(blue), static_cast<int>(alpha)));

            processedPixels++;
            int progress = qRound(static_cast<double>(processedPixels) / totalPixels * 100);
            emit update(progress);
        }
    }

    emit finish(resultImage);
}

void GausWindow::start()
{
    thread = new GausThread(image, sigma);
    connect(thread, &GausThread::update, this, &GausWindow::update);
    connect(thread, &GausThread::finish, this, &GausWindow::finish);
    thread->start();
}


void GausWindow::stop() {
    if (thread->isRunning()) {
        thread->terminate();
        thread->wait();
        delete thread;
        thread = nullptr;
    }
    accept();
}

void GausWindow::update(int value) {
    progressBar->setValue(value);
}

void GausWindow::finish(QImage resultImage) {
    accept();
    emit resultReady(resultImage);
}


void GausWindow::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);

    progressBar = new QProgressBar(this);
    layout->addWidget(progressBar);

    stopButton = new QPushButton("Stop", this);
    connect(stopButton, &QPushButton::clicked, this, &GausWindow::stop);
    layout->addWidget(stopButton);

    thread = nullptr;
}
