#include "GausBlur.h"
#include <time.h> 
#include <QtConcurrent/QtConcurrent>

void GausThread::run()
{
    clock_t t;
    t = clock();

    double radius = 3 * sigma; // пиксели на расстоянии более 3σ оказывают на него достаточно малое влияние

    double first = 1 / (2 * M_PI * sigma * sigma);
    double denominator = (2 * sigma * sigma);

    QImage resultImage = QImage(image);

    int width = resultImage.width();
    int height = resultImage.height();

    int totalPixels = width * height;
    int processedPixels = 0;

    QVector<double> weights;
    for (int i = -radius; i <= radius; ++i) {
        for (int j = -radius; j <= radius; ++j) {
            weights.append(first * exp(-(i * i + j * j) / denominator));
        }
    }

    auto horizontal = [&](int rowStart, int rowEnd) {
        for (int y = rowStart; y < rowEnd; ++y) {
            for (int x = 0; x < width; ++x) {
                double red = 0, green = 0, blue = 0, alpha = 0;
                double totalWeight = 0;

                for (int i = -radius; i <= radius; ++i) {
                    int newX = max(0, min(width - 1, x + i));
                    QRgb pixel = image.pixel(newX, y);

                    double weight = weights[i + radius];
                    totalWeight += weight;

                    red += qRed(pixel) * weight;
                    green += qGreen(pixel) * weight;
                    blue += qBlue(pixel) * weight;
                    alpha += qAlpha(pixel) * weight;
                }

                resultImage.setPixel(x, y, qRgba(
                    static_cast<int>(red / totalWeight),
                    static_cast<int>(green / totalWeight),
                    static_cast<int>(blue / totalWeight),
                    static_cast<int>(alpha / totalWeight)
                ));
            }
            processedPixels+= y;
            int percent = static_cast<int>((static_cast<float>(processedPixels) / totalPixels) * 100);
            update(percent);
        }
        };

    auto vertical = [&](int columStart, int columEnd) {
        for (int x = columStart; x < columEnd; ++x) {
            for (int y = 0; y < height; ++y) {
                double red = 0, green = 0, blue = 0, alpha = 0;
                double totalWeight = 0;

                for (int j = -radius; j <= radius; ++j) {
                    int newY = max(0, min(height - 1, y + j));
                    QRgb pixel = resultImage.pixel(x, newY);

                    double weight = weights[j + radius];
                    totalWeight += weight;

                    red += qRed(pixel) * weight;
                    green += qGreen(pixel) * weight;
                    blue += qBlue(pixel) * weight;
                    alpha += qAlpha(pixel) * weight;
                }

                resultImage.setPixel(x, y, qRgba(
                    static_cast<int>(red / totalWeight),
                    static_cast<int>(green / totalWeight),
                    static_cast<int>(blue / totalWeight),
                    static_cast<int>(alpha / totalWeight)
                ));
            }
            processedPixels+= x;
            int percent = static_cast<int>((static_cast<float>(processedPixels) / totalPixels) * 100);
            update(percent);
        }
        };

    int Threads = QThread::idealThreadCount();
    
    QVector<QFuture<void>> futures_horizontal;
    int rowsThread = height / Threads;

    for (int i = 0; i < Threads; ++i) {
        int rowStart = i * rowsThread;
        int rowEnd = rowStart + rowsThread;

        QFuture<void> future = QtConcurrent::run(horizontal, rowStart, rowEnd);
        futures_horizontal.append(future);
    }

    for (QFuture<void>& future : futures_horizontal) {
        future.waitForFinished();
    }
    
    QVector<QFuture<void>> futures_vertical;
    int columThread = width / Threads;

    for (int i = 0; i < Threads; ++i) {
        int columStart = i * columThread;
        int columEnd = columStart + columThread;

        QFuture<void> future = QtConcurrent::run(vertical, columStart, columEnd);
        futures_vertical.append(future);
    }

    for (QFuture<void>& future : futures_vertical) {
        future.waitForFinished();
    }

    t = clock() - t;
    qDebug() << (float)t / CLOCKS_PER_SEC;

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
