#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "lottery.h"

#include <QFrame>
#include <QLabel>
#include <QMainWindow>
#include <QtCharts>
#include "callout.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setUi(const Lottery::RefreshR& m);
    void setUi(const Lottery::ShuffleR& m);
    void setUi(const Lottery::RefreshByWeekR& r);
    void uiSpinBoxSetValue(int i);
    void uiSpinBoxSetMinMax(int i);
    void uiSpinBoxSetMinMax(int min, int max);
    void CreateTicket();
    void ClearTicket();
private:
    bool _isinited=false;
    QList<QFrame*> frames;
    QList<QLabel*> labels;
    QList<QLabel*> labels2;
    QChart *chart;
    QChartView *chartView;
    int MAX=0,MAY=0;
    QScatterSeries _shuffled_series;
    QScatterSeries _all_shuffled_series;

    QGraphicsSimpleTextItem *m_coordX = nullptr;
    QGraphicsSimpleTextItem *m_coordY = nullptr;
    //QChart *m_chart;
    Callout *m_tooltip = nullptr;
    QList<Callout *> m_callouts;

private slots:
    void on_pushButton_download_clicked();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

    void on_spinBox_valueChanged(int arg1);

    void on_pushButton_cminux_clicked();

    void on_pushButton_cplus_clicked();

public slots:
    void keepCallout();
    void tooltip(QPointF point, bool state);
    void tooltip2(bool status, int index, QBarSet *barset);

protected:
    void mouseMoveEvent(QMouseEvent *event);

private:
    Ui::MainWindow *ui;
    void RefreshByWeek();
};
#endif // MAINWINDOW_H
