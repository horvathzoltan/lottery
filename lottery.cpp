#include "lottery.h"
#include "common/helper/textfilehelper/textfilehelper.h"
#include "common/helper/string/stringhelper.h"

Lottery::Lottery()
{

}

Lottery::Settings Lottery::_settings;
QVarLengthArray<Lottery::Data> Lottery::_data;

bool Lottery::FromFile(const QString& fp){
    auto txt = com::helper::TextFileHelper::load(fp);
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
    auto isok = Lottery::FromFile(Lottery::_settings.path);
    if(!isok) return {false, {0}, {{0},{0},{0},{0},{0}}, {{}}, 0, 0};

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


