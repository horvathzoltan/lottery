#ifndef LOTTERY_H
#define LOTTERY_H

#include <QDate>
//#include <QList>
#include <QApplication>
#include <QDir>
#include <QSet>
#include <QString>
#include <QVarLengthArray>
#include <QVector>


class Lottery
{
public:
    struct Settings{
        private:
            const QString appname = "lottery_data";
            const QString download_dir = "download";
            const QString data_dir = "data";
            const QDir home = QDir(QDir::homePath());

            const QString home_fn = home.filePath(appname);

            const QString download_path = QDir(home_fn).filePath(download_dir);
            const QString data_path = QDir(QDir(home_fn).filePath(data_dir)).filePath(yearweek());
            QDir path(const QString& fn){
                auto p = QDir(fn);
                if(!p.exists()) p.mkpath(fn);
                return p;
            };


        public:
            int max = 30;
            int c_min = 10;
            int c_max = 1000;
            int K = 6;
            QString url = "https://bet.szerencsejatek.hu/cmsfiles/otos.csv";
            QString download_ffn(){ return path(download_path).filePath("otos.csv");};
            QString data_ffn(const QString &fn){ return path(data_path).filePath(fn);};
            QString yearweek(){
//                auto t = QDate::currentDate();
//                auto t_y = t.year();
//                auto t_w = t.weekNumber();

                auto t_y = year();
                auto t_w = week();
                return QString::number(t_y)+"-"+QString::number(t_w);
            }
            int year(){ return 2020;}
            int week(){ return 49;}
    };

    static Settings _settings;

    struct Hit{
        int count;
        int prize;
        QString currency;
        QString desc;

        static Hit FromCsv(const QStringList&, const QString &desc);
    };

    struct Data{ //lottery
        int year;
        int week;
        QDate datetime;
    private:
        Hit hits[5];
        int numbers[5];

    public:
        void setHit(int i, Hit h){
            int ix = i-1;
            if(ix<0 || ix>=5) return;
            hits[ix] = h;
        }

        int* Numbers() {return numbers;}

        QString NumbersToString() const {
            QString e;
            for(auto&i:numbers){
                if(!e.isEmpty())e+=",";
                e+=QString::number(i);
                }

            return e;
        }

        int prize_ix(const Data &d) const{
            int x = 0; //ennyi találat
            for(auto&i:d.numbers){
                for(int j=0;j<5;j++) if(numbers[j]==i) x++;
            }
            if(x<1 || x>5) return 0;
            return x-1;
        }

        int prizeCur(const Data &d, QString* curr) const{
            auto pix = prize_ix(d);
            auto h = hits[pix];
            auto p = h.prize;
            if(p<1) return 0;
            if(curr) *curr = h.currency;
            return p;
        }

        int number(int i) const{
            int ix = i-1;
            if(ix<0 || ix>=5) return 0;
            return numbers[ix];
        }

        void setNumber(int i, int n){
            int ix = i-1;
            if(ix<0 || ix>=5) return;
            numbers[ix] = n;
        }

        static bool AscByDate( const Data& l, const Data& r )
        {
            if(l.year == r.year) return (l.week<r.week);
            return l.year<r.year;
        }

        bool ParityTest(const QSet<int>& p){
            int n = 0;
            for(auto&i:numbers){
                if(!(i%2)) n++;
            }
            return p.contains(n);
        };

        bool PentilisTest(const QSet<int>& p){
            QSet<int> pen;
            static const int r = 90/5; //18
            for(auto&i:numbers){
                auto pn = i/r; // pentilis
                pen.insert(pn);
            }
            return p.contains(pen.count());

        }

        bool TestAll(){
            if(!ParityTest({2,3})) return false;
            if(!PentilisTest({3,4})) return false;
            return true;
        }

    };

//    struct DataAscByDate
//    {
//        bool operator()( const Data& l, const Data& r ) const {
//            if(l.year == r.year) return (l.week<r.week);
//            return l.year<r.year;
//        }
//    };

    static QVarLengthArray<Data> _data;

    //static QSet<int> _shuffled;
    Lottery();
    static bool FromFile(const QString& fp);
    static QStringList CsvSplit(const QString& s);

    struct RefreshR
    {
        bool isOk = false;
        QVarLengthArray<int> histogram;
        QVarLengthArray<int> histograms[5];
        QVarLengthArray<int*> last;
        int min_y;
        int max_y;
    };

    static RefreshR Refresh();

    struct Occurence{
        int num;
        int hist;

        bool operator < (const Occurence& r){ return num < r.num;};
    };

    struct ShuffleR
    {
        QVector<Occurence> num; // a húzás leggyakoribb számai 5-10
        QVector<Data> comb; // a leggyakoribb számokból képzett kombinációk
        bool isok;
    };

    static QVector<Data> Shuffle(int *, int max);

    static void Save(const QVector<Data>&);

    static QVarLengthArray<int> Histogram(
        const QVector<Data>::const_iterator begin,
        const QVector<Data>::const_iterator end, int x =0);
    static QVector<QVector<int>> SelectByCombination(const QVector<Occurence>& p, int k);
    static QVector<QVector<int>> Combination(int N, int K);
    static QVector<Data> Filter(QVector<QVector<int>>& p);


    static QVector<Occurence> SelectByOccurence(const QVector<Data> &d, int i);
    static ShuffleR Generate(int *p, int k, int max);
    static ShuffleR Generate2(const QVector<Lottery::Data>& d);
    struct RefreshByWeekR{
        int shuffnum;
        QVector<Occurence> num; // a húzás leggyakoribb számai 5-10
        QVector<Data> comb; // a leggyakoribb számokból képzett kombinációk
        bool isok;

        QString ToString() const {
            return QString("%2 - %1 db").arg(comb.count()).arg(shuffnum);
        }
    };
    static RefreshByWeekR RefreshByWeek();
    static QFileInfoList ExclusionByWeek();
    static QFileInfoList DataFileInfoListByWeek();
};

#endif // LOTTERY_H
