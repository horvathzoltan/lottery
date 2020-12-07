#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lottery.h"
#include "shufflingdialog.h"
#include "combinationdialog.h"
#include "common/helper/downloader/downloader.h"
#include "common/helper/string/stringhelper.h"
#include "common/helper/textfilehelper/textfilehelper.h"
#include <QtWidgets/QGraphicsView>

#include <QtCharts>
// TODO mennyiért lehet megjátszani a kombinációkat
/*
9 - 35775-(88*300) = 9375
8 - 18550-(34*300) = 8350
7 - 7950-(10*300) = 4950
6 - 3975-(4*300) = 2775
*/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
      , ui(new Ui::MainWindow)
{
    _isinited = false;
    ui->setupUi(this);

    uiSpinBoxSetMinMax(5, 30);
    uiSpinBoxSetValue(Lottery::_settings.K);
    ui->label_yearweek->setText(Lottery::_settings.yearweek());

    CreateTicket(); // a szelvény generálása
    ClearTicket();

    chart = new QChart(); // a grafikon generálása
    chart->setTitle("gyakoriság");
    chart->setAcceptHoverEvents(true);

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

    chart->addSeries(&_shuffled_series);

    _all_shuffled_series.setName("összes");
    _all_shuffled_series.setColor(Qt::green);
    _all_shuffled_series.setMarkerSize(7.0);
    _all_shuffled_series.append(QPointF(0, 0));
    _all_shuffled_series.append(QPointF(MAX, MAY));

    chart->addSeries(&_all_shuffled_series);

    connect(&_all_shuffled_series, &QLineSeries::hovered, this, &MainWindow::tooltip);



    RefreshByWeek();


    _isinited = true;
}

void MainWindow::ClearTicket(){
    static const int Q = 5;
    static const int R = 90/Q;

    QPalette pal_w = palette();
    QPalette pal_g = palette();
    pal_w.setColor(QPalette::Window, Qt::white);
    pal_g.setColor(QPalette::Background, Qt::lightGray);
    QPalette *p;

    for(int i=0;i<90;i++){
        p = ((i/R)%2)?&pal_g:&pal_w;
        frames[i]->setPalette(*p);
    }
}

void MainWindow::CreateTicket(){
    static const int s = 48;

    static auto fi = ui->frame->fontInfo();
    static QFont font(fi.family(), s/3, fi.weight()*2, fi.italic());
    static QFont font2(fi.family(), s/5, fi.weight()/2, fi.italic());


    QPalette pal_blue;
    pal_blue.setColor(QPalette::WindowText, Qt::blue);

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
        l2->setPalette(pal_blue);
        l2->colorCount();

        l2->setGeometry(2, 0, 48-(4*2), 16);
        l2->setAlignment(Qt::AlignmentFlag::AlignRight);

        frames.append(w);
        labels.append(l);
        labels2.append(l2);

        w->setAutoFillBackground(true);
    }
}

MainWindow::~MainWindow()
{
    //for(auto i:frames){delete i;}
    delete ui;
    //labels[0]->setText("a");
}

// https://code.qt.io/cgit/qt/qtcharts.git/tree/examples/charts/callout?h=5.15
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    m_coordX->setText(QString("X: %1").arg(chart->mapToValue(event->pos()).x()));
    m_coordY->setText(QString("Y: %1").arg(chart->mapToValue(event->pos()).y()));
    //QGraphicsView::mouseMoveEvent(event);

}

void MainWindow::keepCallout(){
    m_callouts.append(m_tooltip);
    m_tooltip = new Callout(chart);
}

void MainWindow::tooltip(QPointF point, bool state){
    if (!m_tooltip) m_tooltip = new Callout(chart);

    if (state) {
        auto txt = "kombináció:\n"+QString::number(point.x()+.5)+"\n"+QString::number(point.y(),'f',0);
        m_tooltip->setText(txt);
        m_tooltip->setAnchor(point);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();
    } else {
        m_tooltip->hide();
    }
}

void MainWindow::tooltip2(bool status, int index, QBarSet *barset){
    if (!m_tooltip) m_tooltip = new Callout(chart);

    if (status) {
        auto a = barset->at(index);
        auto point = QPointF(index, a);

        auto txt = "összes:\n"+QString::number(index+1)+"\n"+QString::number(a,'f',0);

        m_tooltip->setText(txt);
        m_tooltip->setAnchor(point);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();
    } else {
        m_tooltip->hide();
    }
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
void MainWindow::on_pushButton_download_clicked()
{
    auto ffn = Lottery::_settings.download_ffn();
    bool isok = com::helper::Downloader::Wget(
        Lottery::_settings.url,
        ffn);
    if(!isok) return;
    Lottery::_data.clear();
    auto a = Lottery::Refresh();
    setUi(a);
    auto b = Lottery::RefreshByWeek();
    setUi(b);
}

void MainWindow::setUi(const Lottery::RefreshR& m){
    if(!m.isOk) return;
    QString txt = QString::number(Lottery::_data.size()) + com::helper::StringHelper::NewLine+ \
                Lottery::_data.last().datetime.toString() + com::helper::StringHelper::NewLine+ \
                Lottery::_data.last().NumbersToString();


    this->ui->label_data->setText(txt);

    //this->ui->label_date->setText(d.datetime.toString());

    MAX = 90;
    MAY = m.max_y;

    //chart->series().clear();
    //chart->axes().detach();

    if(!chart->series().isEmpty()) for(auto&s:chart->series()) chart->removeSeries(s);
    for(auto& a:chart->axes()) chart->removeAxis(a);

    QBarSeries* barseries = new QBarSeries();
    //barseries->setBarWidth(1);
    QBarSet* set0 = new QBarSet("összes");
    barseries->append(set0);

    QScatterSeries *scatterseries = new QScatterSeries();
    scatterseries->setName("utolsó");
    scatterseries->setColor(Qt::red);
    scatterseries->setMarkerSize(7.0);

    scatterseries->append(QPointF(0, 0));
    scatterseries->append(QPointF(MAX, MAY));

    //set0->append(0);

    ClearTicket();

    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::red);

    bool frames_isok = !frames.isEmpty();
    // az utolsót kirakni
    for(int* j:m.last){
       for(int i=0;i<5;i++){
           int n = j[i];
            qreal y = m.histogram[n-1];
            //qreal y = 0;
            qreal x = n-.5;
            scatterseries->append(QPointF(x, y));

            if(frames_isok) frames[n-1]->setPalette(pal);
        }
    }


    QLineSeries *lineseries[5];
    for(int n=0;n<5;n++)
    {
        auto l = new QLineSeries();
        lineseries[n] = l;
        l->setName(QString::number(n+1));
        lineseries[0]->append(QPointF(0, 0));

        //auto s = new QBarSet(QString::number(n+1));
    }

    /*lineseries[0]->append(QPointF(0, 0));
    lineseries[1]->append(QPointF(0, 0));
    lineseries[2]->append(QPointF(0, 0));
    lineseries[3]->append(QPointF(0, 0));
    lineseries[4]->append(QPointF(0, 0));*/


    QStringList categories;

    categories.append(QString::number(0)); //1-90

    bool labels2_isok = !labels2.isEmpty();
    for(int i=0;i<MAX;i++){
        categories.append(QString::number(i+1)); //1-90

        auto x = m.histogram[i];
        auto a = QString::number(x);

        if(labels2_isok) labels2[i]->setText(a); // labelek a táblában

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

    connect(barseries, &QBarSeries::hovered, this, &MainWindow::tooltip2);

}

//TODO könyvtárban gyűlnek a számok ok
//TODO a kombinációknak is gyűlniük kellene - lehessen kiszürkíteni nem tetszőt
//TODO az összes heti számok alapján is kellene tudni számolni kombinációt
//TODO az aktuális kombinációt mutatni kellene zöld pöttyökkel illetve a szelvény nézeten zöld kerettel
//TODO egy kombináció hány számja egyezik meg az előzővel - legfeljebb mennyi egyezhet meg? 1.
//TODO az kombinációkban szereplő összes számokat -nem csak az utolsó sorsolásét- mutatni kellene

// generate
void MainWindow::on_pushButton_clicked()
{
    ShufflingDialog d;
    d.exec();
    auto a = d.result();
    if(a.isok) setUi(a);

//    QString data_ffn = Lottery::_settings.data_ffn("");
//    QDir dir(data_ffn);
//    auto fl = dir.entryInfoList(QDir::Filter::Files);
//    if(fl.isEmpty()) return;
//    QVector<Lottery::Data> fd;
//    for(auto&i:fl){
//        if(!i.fileName().endsWith(".csv")) continue;
//        auto fn = i.absoluteFilePath();
//        auto txt = com::helper::TextFileHelper::load(fn);
//        auto lines = com::helper::StringHelper::toStringList(txt);
//        for(auto&line:lines){
//            auto l = line.split(",");
//            if(l.count()<5) continue;
//            Lottery::Data d0;
//            bool isok;
//            for(int k=0;k<5;k++) d0.setNumber(k+1, l[k].toInt(&isok));
//            fd.append(d0);
//        }
//    }
//    if(fd.isEmpty()) return;
//    auto r = Lottery::Generate2(fd, 7);
    auto r = Lottery::RefreshByWeek();
    setUi(r);

}

void MainWindow::setUi(const Lottery::ShuffleR& m)
{
    CombinationDialog d;
    d.setUi(m);
    d.exec();

    _shuffled_series.clear();
    //chart->removeSeries(&_shuffled_series);
    //auto r = shuffnum/Lottery::_data.size();

    for(auto&i:m.num){//Lottery::_shuffled
        qreal x = i.num-.5;
        _shuffled_series.append(QPointF(x, 1));
    }

    //chart->addSeries(&_shuffled_series);
}

void MainWindow::setUi(const Lottery::RefreshByWeekR& m){
    ui->listWidget->clear();
    QString txt = m.ToString();    

    qreal may=0;
    for(auto&i:m.num){if(i.hist>may) may=i.hist;}
    qreal r2 = may?MAY/may:0;

//    int r;
//    if(Lottery::_data.isEmpty()) r=1;
//    else r = (m.shuffnum/Lottery::_data.size())+5;
    auto last = Lottery::_data.last();
    ui->label_comb->setText(txt);

    auto t_y = Lottery::_settings.year();
    auto t_w = Lottery::_settings.week();
    bool isok = last.year == t_y && last.week == t_w;

    int sum_prize=0;
    int sum_n=0;
    QString sum_curr;
    for(auto&i:m.comb){
        auto txt = i.NumbersToString();

        if(isok){
            QString curr;
            auto v = last.prizeCur(i, &curr);
            if(v>0){
                txt += "-"+ QString::number(v) + ' ' + curr;
                sum_prize += v;
                if(sum_curr.isEmpty()) sum_curr = curr;
                sum_n++;
            }

        }

        if(last.year == t_y && last.week == t_w)

        ui->listWidget->addItem(txt);
    }


    _all_shuffled_series.clear();
    _all_shuffled_series.append(QPointF(0, 0));
    _all_shuffled_series.append(QPointF(MAX, MAY));
    chart->removeSeries(&_all_shuffled_series);
    QString ctxt;
    int o = 0;

    for(auto&i:m.num){ // felsorolja a kombináció számait és felrakja a zöld pettyeket
        qreal x = i.num-.5;
        _all_shuffled_series.append(QPointF(x, r2?i.hist*r2:6));
        if(!ctxt.isEmpty()) ctxt+=",";
        if(!(o%7)) if(!ctxt.isEmpty()) ctxt+=com::helper::StringHelper::NewLine;

        ctxt += QString::number(i.num);

        o++;

    }

    if(sum_prize>0){
        ctxt += '\n'+ QString::number(sum_prize) + ' ' + sum_curr;
        ctxt += '\n'+ QString::number(sum_n)+'/'+QString::number(m.comb.length())+' '+"db";
        ctxt += "\narány:"+ QString::number(sum_n/(qreal)m.comb.length());
    }


    ui->label_combnum->setText(ctxt);
    chart->addSeries(&_all_shuffled_series);
}

//delete
void MainWindow::on_pushButton_2_clicked()
{
    chart->removeSeries(&_shuffled_series);
    _shuffled_series.clear();
    _shuffled_series.append(QPointF(0, 0));
    _shuffled_series.append(QPointF(MAX, MAY));
    //Lottery::_shuffled.clear();

}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    if(!_isinited) return;
    static bool lock = false;
    if(lock) return;
    lock=true;
    Lottery::_settings.K = arg1;
    RefreshByWeek();
    lock=false;
}

void MainWindow::RefreshByWeek(){
    auto r = Lottery::RefreshByWeek(); // ez betölti
    if(r.isok)
        setUi(r);
}

void MainWindow::on_pushButton_cminux_clicked()
{
    int v = ui->labelc->property("value").toInt();
    int min = ui->labelc->property("min").toUInt();
    if(v<=min) return; // elértük
    uiSpinBoxSetValue(--v);
    on_spinBox_valueChanged(v);
}

void MainWindow::on_pushButton_cplus_clicked()
{
    int v = ui->labelc->property("value").toInt();
    int max = ui->labelc->property("max").toUInt();
    if(v>=max) return; // elértük
    uiSpinBoxSetValue(++v);
    on_spinBox_valueChanged(v);
}

void MainWindow::uiSpinBoxSetValue(int i){
    ui->labelc->setProperty("value", i);
    ui->labelc->setText(QString::number(i));
    //Lottery::_settings.K;
}

void MainWindow::uiSpinBoxSetMinMax(int min, int max){
    ui->labelc->setProperty("min", min);
    ui->labelc->setProperty("max", max);
}

//
