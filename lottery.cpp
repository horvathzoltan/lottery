#include "lottery.h"
#include "common/helper/textfilehelper/textfilehelper.h"
#include "common/helper/string/stringhelper.h"
#include "common/helper/downloader/downloader.h"
#include "common/helper/textfilehelper/textfilehelper.h"
#include <QDir>
#include <QThread>
#include <random>

Lottery::Lottery()
{

}

Lottery::Settings Lottery::_settings;
QVarLengthArray<Lottery::Data> Lottery::_data;
//QSet<int> Lottery::_shuffled;

QFileInfoList Lottery::DataFileInfoListByWeek(){
    QString data_ffn = Lottery::_settings.data_ffn("");
    QDir dir(data_ffn);
    auto fl = dir.entryInfoList(QDir::Filter::Files);
    return fl;
}

QFileInfoList Lottery::ExclusionByWeek(){
    auto fil = DataFileInfoListByWeek();
    QString ex_fn = Lottery::_settings.data_ffn("excl.csv");

    auto ex_txt = com::helper::TextFileHelper::load(ex_fn);
    auto ex_lines = com::helper::StringHelper::toStringList(ex_txt);

    QFileInfoList e;

    for(auto&i:fil) if(!ex_lines.contains(i.fileName())) e.append(i);

    return e;
}

Lottery::RefreshByWeekR Lottery::RefreshByWeek(){
    Lottery::RefreshByWeekR nullelem{0, {},{},false};
//    QString data_ffn = Lottery::_settings.data_ffn("");
//    QDir dir(data_ffn);
//    auto fl = dir.entryInfoList(QDir::Filter::Files);
    auto fl = DataFileInfoListByWeek();
    if(fl.isEmpty()) return nullelem;
    QVector<Lottery::Data> fd;
    for(auto&i:fl){
        if(!i.fileName().endsWith(".csv")) continue;
        auto fn = i.absoluteFilePath();
        auto txt = com::helper::TextFileHelper::load(fn);
        auto lines = com::helper::StringHelper::toStringList(txt);
        for(auto&line:lines){
            auto l = line.split(",");
            if(l.count()<5) continue;
            Lottery::Data d0;
            bool isok;
            for(int k=0;k<5;k++) d0.setNumber(k+1, l[k].toInt(&isok));
            fd.append(d0);
        }
    }
    if(fd.isEmpty()) return nullelem;
    auto r = Lottery::Generate2(fd);
    return {fd.count(), r.num, r.comb, r.isok};
}

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
    auto ffn = Lottery::_settings.download_ffn();
    auto txt = com::helper::TextFileHelper::load(ffn);
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

QVector<QVector<int>> Lottery::SelectByCombination(const QVector<Occurence>&p, int k){
    QVector<QVector<int>> e;
    auto n = p.count();
    if(n<1||k<1||n<k) return e;

    auto c = Combination(n, k);

    for(auto&i:c){
        QVector<int> f;
        for(auto&j:i){
            f.append(p[j].num);
        }

        e.append(f);
    }

    return e;
}

QVector<Lottery::Data> Lottery::Filter(QVector<QVector<int>>& p){
    QVector<Data> e;
    if(p.isEmpty()) return e;

    for(auto&i:p){
        Data d0;
        if(i.count()<5) return e;
        for(int j=0;j<5;j++)
        {
            int n = i[j];
            d0.setNumber(j+1, n);
        }
        if(!d0.TestAll()) continue;
        e.append(d0);
    }
    return e;
}

//http://rosettacode.org/wiki/Combinations#C.2B.2B
QVector<QVector<int>> Lottery::Combination(int N, int K)
{
    std::string bitmask(K, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's
    QVector<QVector<int>> e;

    // print integers and permute bitmask
    do {
        QVector<int> f;
        for (int i = 0; i < N; ++i) // [0..N-1] integers
        {
            if (bitmask[i]) f.append(i);//std::cout << " " << i;
        }
        e.append(f);
        //std::cout << std::endl;
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
    return e;
}

Lottery::ShuffleR Lottery::Generate(int *p, int k, int max){

    auto shuffled = Lottery::Shuffle(p, max); //sorsolás - 1000 db
    auto r = Generate2(shuffled);

    return r;
}

Lottery::ShuffleR Lottery::Generate2(const QVector<Lottery::Data>& d){
    Lottery::ShuffleR r;
    int K = Lottery::_settings.K;
     r.num = Lottery::SelectByOccurence(d, K); // vesszük k db leggyakoribbat

    if(r.num.count()>5){
        auto a = Lottery::SelectByCombination(r.num, 5); // permutáljuk
        r.comb = Lottery::Filter(a);
        r.isok = true;
    }
    else if(r.num.count()==5){
        Lottery::Data d0;
        for(int j=1;j<=5;j++) d0.setNumber(j, r.num[j-1].num);
        if(d0.TestAll()) r.comb.append(d0);
        r.isok = true;
    }

    return r;
}


QVector<Lottery::Data> Lottery::Shuffle(int* ptr, int max){
    QVector<Data> d;

    if(max<Lottery::_settings.c_min) max=Lottery::_settings.c_min; else if(max>Lottery::_settings.c_max) max = Lottery::_settings.c_max;

    auto histogram = Lottery::Histogram(_data.begin(), _data.end());

    QVector<int> num2;
    for(int i=0;i<90;i++)
    {
        int n = histogram[i]; // i a szám, n a daramszáma
        for(int k=0;k<n;k++)
        {
            num2.append(i+1);
        }
    }

    int t=0;
    do {
        if(ptr)*ptr=t/(max/100);
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
        if(!d0.TestAll()) continue;

        if(QThread::currentThread()->isInterruptionRequested()) return d;

        d.append(d0);
        t++;
    } while(t<max);

    Lottery::Save(d);

    return d;
}

QVector<Lottery::Occurence> Lottery::SelectByOccurence(const QVector<Data>& d, int db){
    QVector<Occurence> e;
    if(db<5 || db>Lottery::_settings.max) return e;
    if(d.isEmpty()) return e;

    auto h2 = Lottery::Histogram(d.begin(), d.end());

//    auto h3 = h2;

    // kiválasztjuk a 10 legjobbat
    for(int i=0;i<db;i++)
    {
        int ix = -1;
        int max=-1;
        for(int j=0;j<90;j++){
            int n = h2[j];
            if(n>max)
            {
                max=n;
                ix=j;
            }
        }
        e.append({ix+1, h2[ix]});
        h2[ix] = 0;
    }

    std::sort(e.begin(), e.end());
    return e;
}


void Lottery::Save(const QVector<Lottery::Data> &d)
{
    QString fn = QDateTime::currentDateTimeUtc().toString("yyyyMMdd_hh:mm:ss")+".csv";
    QString ffn =Lottery::_settings.data_ffn(fn);
    QString txt;

    for(auto&i:d)
    {
        if(!txt.isEmpty()) txt+= com::helper::StringHelper::NewLine;
        txt.append(i.NumbersToString());
    }

    com::helper::TextFileHelper::save(txt, ffn);
}

QVarLengthArray<int> Lottery::Histogram(
    const QVector<Data>::const_iterator begin,
    const QVector<Data>::const_iterator end, int m)
{
    QVarLengthArray<int> r(90);for(auto& i:r) i=0;//0-89
    if(begin==end) return r;
    if(m<0||m>5) return r;

    //int t = 0;
    for(auto i = begin;i!=end;i++){
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


