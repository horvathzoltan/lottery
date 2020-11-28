#ifndef LOTTERY_H
#define LOTTERY_H

#include <QDate>
//#include <QList>
#include <QSet>
#include <QVarLengthArray>


class Lottery
{
public:
    struct Settings{
        QString path = "/home/zoli/Letöltések/otos.csv";
    };

    static Settings _settings;

    struct Hit{
        int count;
        QString prize;
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

        int number(int i){
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

    };

//    struct DataAscByDate
//    {
//        bool operator()( const Data& l, const Data& r ) const {
//            if(l.year == r.year) return (l.week<r.week);
//            return l.year<r.year;
//        }
//    };

    static QVarLengthArray<Data> _data;

    static QSet<int> _shuffled;
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

    struct ShuffleR
    {
        int num[5];
        bool isok;
    };

    static ShuffleR Shuffle(int *);

    static QVarLengthArray<int> Histogram(
        QVarLengthArray<Data>::iterator begin,
        QVarLengthArray<Data>::iterator end, int x =0);
};

#endif // LOTTERY_H
