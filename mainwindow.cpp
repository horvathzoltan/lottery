#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lottery.h"
#include "shufflingdialog.h"
#include "common/helper/downloader/downloader.h"
#include "common/helper/string/stringhelper.h"
#include <QtCharts>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
      , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    static const int s = 48;

    static auto fi = ui->frame->fontInfo();
    static QFont font(fi.family(), s/3, fi.weight()*2, fi.italic());
    static QFont font2(fi.family(), s/5, fi.weight()/2, fi.italic());

    QPalette sample_palette;
    sample_palette.setColor(QPalette::Window, Qt::white);
    sample_palette.setColor(QPalette::WindowText, Qt::blue);

    static const int Q = 5;
    static const int R = 90/Q;

    for(int i=0;i<90;i++){
        int y = i/10;
        int x = i%10;

        QFrame *w = new QFrame(ui->frame);
        w->setGeometry(x*(s+8), y*(s+8), s, s);

        w->setFrameStyle(QFrame::Panel | QFrame::Plain);
        w->setLineWidth(1);

        QLabel *l = new QLabel(w);
        l->setFont(font);
        l->setText(QString::number(i+1));

        l->adjustSize();

        auto ax = (w->width()-l->width())/2;
        auto ay = (w->height()-l->height())/2;
        l->move(ax, ay);


        QLabel *l2 = new QLabel(w);
        l2->setFont(font2);
        l2->setPalette(sample_palette);
        //l2->setText("WWW");
        l2->colorCount();
        //l2->setGeometry()
        //l2->adjustSize();

        //ax = w->width()-l2->width()-4;

        //l2->move(ax, 0);
        l2->setGeometry(2, 0, 48-(4*2), 16);
        l2->setAlignment(Qt::AlignmentFlag::AlignRight);

        frames.append(w);
        labels.append(l);
        labels2.append(l2);

        w->setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Background, Qt::lightGray);

        if((i/R)%2) w->setPalette(pal);


    }

    //tab_2
    //QChart *
        chart = new QChart();
    chart->setTitle("gyakoriság");
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setParent(ui->tab_2);
    chartView->setGeometry(ui->tab_2->geometry());
    auto a = Lottery::Refresh();

    setUi(a); // beállítja a MAX, MAY értékeket is

    _shuffled_series.setName("sorsolt");
    _shuffled_series.setColor(Qt::blue);
    _shuffled_series.setMarkerSize(7.0);
    _shuffled_series.append(QPointF(0, 0));
    _shuffled_series.append(QPointF(MAX, MAY));
}

MainWindow::~MainWindow()
{
    //for(auto i:frames){delete i;}
    delete ui;
    //labels[0]->setText("a");
}

//https://bet.szerencsejatek.hu/cmsfiles/otos.csv
//
/*
 * Év;Hét;Húzásdátum;
 * 5 találat (db);5 találat (Ft);
 * 4 találat (db);4 találat (Ft);
 * 3 találat (db);3 találat (Ft);
 * 2 találat (db);2 találat (Ft);
 * Számok
 * */
//2020;46;2020.11.14.;0;0 Ft;41;1 533 055 Ft;3707;18 260 Ft;107291;1 645 Ft;8;13;30;61;68

// TODO ha a héten le lett töltve akkor nem kell megint leszedni
// ha az aktuálisan betöltött csv hete megfelel az aktuális hétnek, akkor az friss, nem kell tölteni
// legyen szürke a gomb
void MainWindow::on_pushButton_data_clicked()
{
    auto ffn = Lottery::_settings.download_ffn();
    bool isok = com::helper::Downloader::Wget(
        Lottery::_settings.url,
        ffn);
    if(!isok) return;
    Lottery::_data.clear();
    auto a = Lottery::Refresh();
    setUi(a);
}

void MainWindow::setUi(Lottery::RefreshR m){
    if(!m.isOk) return;
    QString txt = QString::number(Lottery::_data.size()) + com::helper::StringHelper::NewLine+ \
                Lottery::_data.last().datetime.toString() + com::helper::StringHelper::NewLine+ \
                Lottery::_data.last().NumbersToString();


    this->ui->label_data->setText(txt);

    //this->ui->label_date->setText(d.datetime.toString());

    MAX = 90;
    MAY = m.max_y;

//    chart->series().clear();
    //chart->axes().detach();

    if(chart->series().count()>0) chart->removeAllSeries();
    for(auto& a:chart->axes()) chart->removeAxis(a);

    QBarSeries *barseries = new QBarSeries();
    //barseries->setBarWidth(1);
    QBarSet *set0 = new QBarSet("összes");
    barseries->append(set0);

    QScatterSeries *scatterseries = new QScatterSeries();
    scatterseries->setName("utolsó");
    scatterseries->setColor(Qt::red);
    scatterseries->setMarkerSize(7.0);

    scatterseries->append(QPointF(0, 0));
    scatterseries->append(QPointF(MAX, MAY));

    //set0->append(0);
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::red);

    for(int* j:m.last){
       for(int i=0;i<5;i++){
           int n = j[i];
            qreal y = m.histogram[n-1];
            //qreal y = 0;
            qreal x = n-.5;
            scatterseries->append(QPointF(x, y));

            frames[n-1]->setPalette(pal);
        }
    }


    QLineSeries *lineseries[5];
    for(int n=0;n<5;n++)
    {
        auto l = new QLineSeries();
        lineseries[n] = l;
        l->setName(QString::number(n+1));
        lineseries[0]->append(QPointF(0, 0));

        auto s = new QBarSet(QString::number(n+1));
    }

    /*lineseries[0]->append(QPointF(0, 0));
    lineseries[1]->append(QPointF(0, 0));
    lineseries[2]->append(QPointF(0, 0));
    lineseries[3]->append(QPointF(0, 0));
    lineseries[4]->append(QPointF(0, 0));*/


    QStringList categories;

    categories.append(QString::number(0)); //1-90

    for(int i=0;i<MAX;i++){
        categories.append(QString::number(i+1)); //1-90

        auto x = m.histogram[i];
        auto a = QString::number(x);

        labels2[i]->setText(a); // labelek a táblában

//        int R = 90/5;
//        int o = (i/R);

        set0->append(x); //bars

        for(int n=0;n<5;n++) // lines
        {
            qreal x = (qreal)(i)+.5;
            qreal y = m.histograms[n][i];
            lineseries[n]->append(QPointF(x, y));
        }
    }

    categories.append(QString::number(MAX+1));
    //set0->append(MAY);

    lineseries[0]->append(QPointF(MAX, MAY));
    lineseries[1]->append(QPointF(MAX, MAY));
    lineseries[2]->append(QPointF(MAX, MAY));
    lineseries[3]->append(QPointF(MAX, MAY));
    lineseries[4]->append(QPointF(MAX, MAY));

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    //QValueAxis *axisX = new QValueAxis();

    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);

    lineseries[0]->attachAxis(axisX);
    lineseries[1]->attachAxis(axisX);
    lineseries[2]->attachAxis(axisX);
    lineseries[3]->attachAxis(axisX);
    lineseries[4]->attachAxis(axisX);
    scatterseries->attachAxis(axisX);
    barseries->attachAxis(axisX);

    axisX->setRange(QString("1"), QString::number(MAX));
    //axisX->setRange(1, MAX);

    //axisX->setTickCount(18);
    //axisX->setMinorTickCount(1);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTickCount(10);
    axisY->setMax(MAY);
    axisY->setRange(0, MAY);
    //chartView->chart()->setAxisY(axisY, scatterseries);

    lineseries[0]->attachAxis(axisY);
    lineseries[1]->attachAxis(axisY);
    lineseries[2]->attachAxis(axisY);
    lineseries[3]->attachAxis(axisY);
    lineseries[4]->attachAxis(axisY);
    scatterseries->attachAxis(axisY);
    barseries->attachAxis(axisY);

    QLinearGradient plotAreaGradient;
    plotAreaGradient.setStart(QPointF(0, 0));
    plotAreaGradient.setFinalStop(QPointF(1, 0));

    plotAreaGradient.setColorAt(0.0, QRgb(0xefebe7));
    plotAreaGradient.setColorAt(.9999/5, QRgb(0xefebe7));
    plotAreaGradient.setColorAt(1.0/5, QRgb(0xc0c0c0));
    plotAreaGradient.setColorAt(1.9999/5, QRgb(0xc0c0c0));
    plotAreaGradient.setColorAt(2.0/5, QRgb(0xefebe7));
    plotAreaGradient.setColorAt(2.9999/5, QRgb(0xefebe7));
    plotAreaGradient.setColorAt(3.0/5, QRgb(0xc0c0c0));
    plotAreaGradient.setColorAt(3.9999/5, QRgb(0xc0c0c0));
    plotAreaGradient.setColorAt(4.0/5, QRgb(0xefebe7));

    plotAreaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    chart->setPlotAreaBackgroundBrush(plotAreaGradient);
    chart->setPlotAreaBackgroundVisible(true);



    chart->addAxis(axisY, Qt::AlignLeft);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignTop);

   //chart->createDefaultAxes();


    chart->addSeries(barseries);
    for(int n=0;n<5;n++)
    {
        chart->addSeries(lineseries[n]);
    }
    chart->addSeries(scatterseries);
}

//TODO könyvtárban gyűlnek a számok ok
//TODO a kombinációknak is gyűlniük kellene - lehessen kiszürkíteni nem tetszőt
//TODO az összes heti számok alapján is kellene tudni számolni kombinációt
//TODO az aktuális kombinációt mutatni kellene zöld pöttyökkel illetve a szelvény nézeten zöld kerettel
//TODO egy kombináció hány számja egyezik meg az előzővel - legfeljebb mennyi egyezhet meg? 1.
//TODO az kombinációkban szereplő összes számokat -nem csak az utolsó sorsolásét- mutatni kellene
void MainWindow::on_pushButton_clicked()
{
    ShufflingDialog d;
    d.exec();
    auto a = d.result();
    if(a.isok) setUi(a);
}

void MainWindow::setUi(Lottery::ShuffleR m)
{
    QMessageBox msgBox;
    QString e;

    for(auto&i:m.num){
        if(!e.isEmpty()) e+=",";
        e+=QString::number(i);
        Lottery::_shuffled.insert(i);        
    }
    QString txt = "számok: " +e;
    if(!m.comb.isEmpty()){
        if(!txt.isEmpty()) txt+=com::helper::StringHelper::NewLine;
        txt+=QString("kombinációk: %1 db").arg(m.comb.count());
        for(auto&i:m.comb){
            if(!txt.isEmpty()) txt+=com::helper::StringHelper::NewLine;
            txt+= i.NumbersToString();
        }
    }
    msgBox.setText(txt);
    msgBox.exec();

    //static int const MAX = 90;
    //static int const MAY = m.max_y;

    chart->removeSeries(&_shuffled_series);

    for(auto&i:Lottery::_shuffled){
        qreal x = i-.5;
        _shuffled_series.append(QPointF(x, 5));
    }

    chart->addSeries(&_shuffled_series);
}


void MainWindow::on_pushButton_2_clicked()
{
    chart->removeSeries(&_shuffled_series);
    _shuffled_series.clear();
    _shuffled_series.append(QPointF(0, 0));
    _shuffled_series.append(QPointF(MAX, MAY));
    Lottery::_shuffled.clear();

}
