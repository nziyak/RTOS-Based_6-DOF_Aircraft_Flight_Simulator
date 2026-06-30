#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    void updateDashboard(double time, double altitude,  double velocity, double thrust, double qx, double qy, double qz, double qw);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
