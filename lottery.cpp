#include "lottery.h"
#include "common/helper/textfilehelper/textfilehelper.h"
#include "common/helper/string/stringhelper.h"
#include "common/helper/downloader/downloader.h"
#include <QThread>
#include <random>

Lottery::Lottery()
{

}

Lottery::Settings Lottery::_settings;
QVarLengthArray<Lottery::Data> Lottery::_data;
QSet<int> Lottery::_shuffled;

bool Lottery::FromFile(const QString& txt){
    //auto txt = com::helper::TextFileHelper::load(fp);
    auto lines = com::helper::StringHelper::toStringList(txt);

    auto size_orioginal = _data.size();

    static const int year_ix = 0;
    static const int week_ix = 1;
    static const int datetime_ix = 2;
    static const int hit5_ix = 3;
    static const int hit4_ix = 5;
    static const int hit3_ix = 7;
    static const int hit2_ix = 9;
    static const int hit_len = 2;
    static const int numbers_ix = 11;

    for(auto l: lines){
        auto a = CsvSplit(l);
        if(a.length()<16) continue;
        Data d;
        bool isok;

        d.year = a[year_ix].toInt(&isok);
        d.week = a[week_ix].toInt(&isok);
        d.datetime = QDate::fromString(a[datetime_ix], "yyyy.MM.dd.");

        d.setHit(5, Hit::FromCsv(lines.mid(hit5_ix, hit_len), "5"));
        d.setHit(4, Hit::FromCsv(lines.mid(hit4_ix, hit_len), "4"));
        d.setHit(3, Hit::FromCsv(lines.mid(hit3_ix, hit_len), "3"));
        d.setHit(2, Hit::FromCsv(lines.mid(hit2_ix, hit_len), "2"));

        d.setNumber(1, a[numbers_ix].toInt(&isok));
        d.setNumber(2, a[numbers_ix+1].toInt(&isok));
        d.setNumber(3, a[numbers_ix+2].toInt(&isok));
        d.setNumber(4, a[numbers_ix+3].toInt(&isok));
        d.setNumber(5, a[numbers_ix+4].toInt(&isok));

        _data.append(d);
    }
    return size_orioginal<_data.size();
}

QStringList Lottery::CsvSplit(const QString &s)
{
    return s.trimmed().split(';');
}

Lottery::Hit Lottery::Hit::FromCsv(const QStringList& lines, const QString &desc)
{
    bool isok;
    return {lines[0].toInt(&isok), lines[1], desc};//.toInt(&isok)
}

Lottery::RefreshR Lottery::Refresh(){
    ////https://bet.szerencsejatek.hu/cmsfiles/otos.csv
    //com::helper::Downloader d;
    static Lottery::RefreshR nullobj{false, {0}, {{0},{0},{0},{0},{0}}, {{}}, 0, 0};
//    bool isok = com::helper::Downloader::Wget(
//        "https://bet.szerencsejatek.hu/cmsfiles/otos.csv",
//        Lottery::_settings.path);
//    if(!isok) return nullobj;
    auto txt = com::helper::TextFileHelper::load(Lottery::_settings.path);
    bool isok = Lottery::FromFile(txt);
    if(!isok) return nullobj;

    Lottery::RefreshR r;
    r.isOk = true;
    std::sort(_data.begin(), _data.end(), Data::AscByDate);

    auto i_min = _data.begin();
    auto i_max = _data.end();

    r.histogram = Lottery::Histogram(i_min, i_max);
    for(int n=0;n<5;n++) r.histograms[n] = Lottery::Histogram(i_min, i_max, n+1);

    auto last = _data.last().Numbers();
    r.last.append(last);

    r.min_y = *std::min_element(r.histogram.begin(), r.histogram.end());
    r.max_y = *std::max_element(r.histogram.begin(), r.histogram.end());

    return r;
}


Lottery::ShuffleR Lottery::Shuffle(int* ptr){
    static Lottery::ShuffleR nullobj{{0,0,0,0,0},false};

//    auto txt = com::helper::TextFileHelper::load(Lottery::_settings.path);
//    bool isok = Lottery::FromFile(txt);
//    if(!isok) return nullobj;

    Lottery::ShuffleR e;

    auto i_min = _data.begin();
    auto i_max = _data.end();

    auto histogram = Lottery::Histogram(i_min, i_max);

    QVector<int> num2;
    for(int i=0;i<90;i++)
    {
        int n = histogram[i]; // i a szám, n a daramszáma
        for(int k=0;k<n;k++)
        {
            num2.append(i+1);
        }
    }

    //std::random_shuffle(num.begin(), num.end());
    //static const int MAX = 1000;
    QVarLengthArray<Data, 1000> d;
    int t=0;
    do {
        if(ptr)*ptr=t/10;
        auto num = num2;
        Data d0;
        for(int j=1;j<=5;j++)
        {
            std::random_shuffle(num.begin(), num.end());
            int max = num.count();
            auto ix = rand() % max;
            int n = num[ix];
            d0.setNumber(j, n);
            num.removeAll(n);
        }
        if(!d0.ParityTest({2,3})) continue;
        if(!d0.PentilisTest({3,4})) continue;

        if ( QThread::currentThread()->isInterruptionRequested() )
        {
            return nullobj;
        }
        d.append(d0);
        t++;
    } while(t<d.capacity());

    auto h2 = Lottery::Histogram(d.begin(), d.end());

    auto h3 = h2;

    for(int i=0;i<5;i++)
    {
        int ix = -1;
        int max=-1;
        for(int j=0;j<90;j++){
            int n = h3[j];
            if(n>max)
            {
                max=n;
                ix=j;
            }
        }
        e.num[i] = ix+1;
        h3[ix] = 0;
    }

    int n = sizeof(e.num) / sizeof(e.num[0]);

    std::sort(e.num, e.num+n);
    return e;
}

QVarLengthArray<int> Lottery::Histogram(
    QVarLengthArray<Data>::iterator begin,
    QVarLengthArray<Data>::iterator end, int m)
{
    QVarLengthArray<int> r(90);for(auto& i:r) i=0;//0-89
    if(begin==end) return r;
    if(m<0||m>5) return r;

    //int t = 0;
    for(auto& i = begin;i!=end;i++){
        if(!m){
            for(int n =1;n<=5;n++)
            {
                int x = i->number(n);// X: 1-90
                if(!x) continue; // 0 az hiba
                r[x-1]++;
            }
        }
        else{
            int x = i->number(m);// X: 1-90
            r[x-1]++;
        }
    }

    return r;
}


