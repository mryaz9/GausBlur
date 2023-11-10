#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include <QThread>
#include <QObject>
#include <QPushButton>
#include "ui_MainWindow.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString clear_image;
    

public slots:
    void open();
    void save();
    void exit();
    void showAbout();

    int start();

    void saveSettings();
    void loadSettings();

    void result(QImage resultImage);

private:
    Ui::MainWindow ui;
    bool blur_image = false;

    QImage resultImage;
    QString image_path;
    QString image_path_save;
};
