#pragma once

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProgressBar>
#include <QThread>
#include <QDialog>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

class GausThread : public QThread {
    Q_OBJECT

public:
    GausThread(QImage& image, double sigma) : image(image), sigma(sigma) {}

signals:
    void update(int value);
    void finish(QImage resultImage);

public:
    void run() override;

private:
    QImage& image;
    double sigma;
};


class GausWindow : public QDialog {
    Q_OBJECT
public:
    GausWindow(QImage& image, double sigma, QWidget* parent = nullptr) : QDialog(parent), image(image), sigma(sigma) {
        setupUi();
    }

    void start();
    

signals:
    void resultReady(QImage resultImage);

private slots:
    
    void stop();

    void update(int value);

    void finish(QImage resultImage);


private:
    void setupUi();

    QProgressBar* progressBar;
    QPushButton* stopButton;
    GausThread* thread;

    QImage& image;
    double sigma;
};