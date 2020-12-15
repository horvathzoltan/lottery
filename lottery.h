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
            QDate _date = QDate::currentDate();

            const QString appname = "lottery_data";
            const QString download_dir = "download";
            const QString data_dir = "data";
            const QDir home = QDir(QDir::homePath());

            const QString home_fn = home.filePath(appname);

            const QString download_path = QDir(home_fn).filePath(download_dir);
            const QString data_root = QDir(home_fn).filePath(data_dir);
            QString data_path() { return QDir(data_root).filePath(yearweek());}
            QDir path(const QString& fn){
                auto p = QDir(fn);
                if(!p.exists()) p.mkpath(fn);
                return p;
            };


        public:
            int filter = 2;
            int max = 30;
            int shuff_max = 100;
            int c_min = 10;
            int c_max = 1000000;
            int K = 6;
            int ticket_price = 300;
            QString ticket_curr = "Ft";
            QString url2= "http://www.lottoszamok.net/otoslotto/";
            QString url = "https://bet.szerencsejatek.hu/cmsfiles/otos.csv";
            QString download_ffn(){ return path(download_path).filePath("otos.csv");};
            QString settings_ffn(){ return path(home_fn).filePath("settings.ini");};
            QString data_ffn(const QString &fn){ return path(data_path()).filePath(fn);};

            void setDate(QDate d){_date = d;};
            void datemm(){_date=_date.addDays(-7);};
            void datepp(){_date=_date.addDays(7);};

//            int year(){return _date.year();}
//            int week(){return _date.weekNumber();}
            QString yearweek(int *y = nullptr, int *w = nullptr){
                auto t = _date;
                auto t_y = t.year();
                auto t_w = t.weekNumber();

                if(y) *y = t_y;
                if(w) *w = t_w;
                return QString::number(t_y)+"-"+QString::number(t_w);
            }

            //TODO Toini Fromini - inihelper
            QString ToIni(){

                return "mamaliga";
            }

    };

    static Settings _settings;    

    struct Hit{
        int count;
        int prize;
        QString currency;
        QString desc;

        static Hit FromCsv(const QStringList&, const QString &desc);
    };

    struct Numbers{
        private:
            int numbers[5];
        public:
            qreal weight = 1;

        QString ToString() const {
            QString e;
            for(auto&i:numbers){
                if(!e.isEmpty())e+=",";
                e+=QString::number(i);
            }

            return e;
        }

        QString ToString(const Numbers& n2) const {
            QString e;
            for(auto&i:numbers){
                if(!e.isEmpty())e+=",";
                e+=QString::number(i);
                if(n2.contains(i)) e+='*';
            }

            return e;
        }

        bool contains(int i) const {auto a = std::find(numbers, numbers+5,i); return a!=numbers+5;}

        void WeightByParity(const QVector<qreal>& w)
        {
            weight *= w[NumbersEven()];
        }

        void WeightByPentilis(const QVector<qreal>& w)
        {
            weight *= w[NumbersPentilis()];
        }

        bool operator== (const Numbers& r){
            int o=0;
            for(auto&i:numbers){
                for(auto&j:r.numbers) if(i==j) {o++;break;}
            }
            return o==5;
        };

        void sort(){std::sort(numbers, numbers+5);}

        int NumbersEven()const{int p=0;for(auto&i:numbers)if(!(i%2))p++;return p;}

        int NumbersPentilis()const{
            QSet<int> pen;
            static const int r = 90/5;//18
            for(auto&i:numbers) pen.insert(i/r); // pentilis

            return pen.count(); // a külömbözők száma
        }

        // ennyies a találat
        int HitNum(const Numbers& d) const{
            int x = 0; //ennyi találat
            for(auto&i:numbers){
                for(auto&j:d.numbers) if(j==i) x++;
                //for(int j=0;j<5;j++) if(d.numbers[j]==i) x++;
            }
            if(x<1 || x>5) return 0;
            return x-1;
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
    };



    struct Data{ //lottery
        int year;
        int week;
        QDate datetime;
        Numbers num;
    private:
        Hit hits[5];

    public:
        void setHit(int i, Hit h){
            int ix = i-1;
            if(ix<0 || ix>=5) return;
            hits[ix] = h;
        }

        // a this díjazása alapján a d kombináció mennyi találatt illetve pénz
        int prizeCur(const Data& d, QString* curr, int* pixe = nullptr) const{
            auto pix = num.HitNum(d.num);
            if(pixe) *pixe = pix;
            auto h = hits[pix];
            auto p = h.prize;
            if(p<1) return 0;
            if(curr) *curr = h.currency;
            return p;
        }


        static bool AscByDate( const Data& l, const Data& r )
        {
            if(l.year == r.year) return (l.week<r.week);
            return l.year<r.year;
        }

    };
    static Data _next;

    static QVector<Data> _data;

    Lottery();
    static bool FromFile(const QString& fp, int y, int w);
    static QStringList CsvSplit(const QString& s);

    struct RefreshR
    {
        bool isOk = false;
        QVector<qreal> histogram;
        QVector<qreal> histograms[5];
        //QVector<Numbers> last;
        int min_y;
        int max_y;
    };

    static RefreshR Refresh(int year, int week);

    struct Occurence{
        int num;
        qreal hist;

        bool operator < (const Occurence& r){ return num < r.num;};
    };

    struct ShuffleR
    {
        QVector<Occurence> num; // a húzás leggyakoribb számai 5-10
        QVector<Data> comb; // a leggyakoribb számokból képzett kombinációk
        //QVector<qreal> weight;
        bool isok;
    };

    static QVector<Data> Shuffle(int *, int max);

    static void Save(const QVector<Data>&);

    static QVector<qreal> Histogram(const QVector<Data>& d, int x);

    static QVector<QVector<int>> SelectByCombination(const QVector<Occurence>& p, int k);
    static QVector<QVector<int>> Combination(int N, int K);
    static QVector<Data> ToData(QVector<QVector<int>>& p);


    static QVector<Occurence> SelectByOccurence(QVector<Data> &d, int i);
    static ShuffleR Generate(int *p, int k, int max);
    static ShuffleR Generate2(QVector<Lottery::Data>& d);
    struct BestHit{
        int ix;
        Numbers numbers;
    };

    struct RefreshByWeekR{
        int shuffnum;
        QVector<Occurence> num; // a húzás leggyakoribb számai 5-10
        QVector<Data> comb; // a leggyakoribb számokból képzett kombinációk
        bool isok;
        QVector<QVector<struct Lottery::BestHit>> besthits;
        QVector<Numbers> mweights;
        int mweight;

        QString ToString() const {
            return QString::number(shuffnum);//QString("%2 - %1 db").arg(comb.count()).arg(shuffnum);
        }
    };
    static RefreshByWeekR RefreshByWeek();
    static QFileInfoList ExclusionByWeek();
    static QFileInfoList DataFileInfoListByWeek();


    static QVector<QVector<BestHit>> FindBestHit(const QVector<Lottery::Data> &fd,const Numbers& numbers);
    //static QVector<qreal> SumWeight(const QVector<QVector<qreal>>&w);

    static void Weight(QVector<Data>* d);
    static void WeightClear(QVector<Data> *d);
    static void WeightByParity(QVector<Data>* d);
    static void WeightByPentilis(QVector<Data>* d);

    static QVector<qreal> WeightsByParity();
    static QVector<qreal> WeightsByPentilis();
    static QVector<Lottery::Numbers> FindByMaxWeight(const QVector<Lottery::Data> &fd, int *maxweight);
};

#endif // LOTTERY_H
