#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "lottery.h"

#include <QFrame>
#include <QLabel>
#include <QMainWindow>
#include <QtCharts>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setUi(Lottery::RefreshR m);
    void setUi(Lottery::ShuffleR m);
private:
    QList<QFrame*> frames;
    QList<QLabel*> labels;
    QList<QLabel*> labels2;
    QChart *chart;
    QChartView *chartView;
    int MAX=0,MAY=0;
private slots:
    void on_pushButton_data_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
