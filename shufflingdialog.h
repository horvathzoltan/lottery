#ifndef SHUFFLINGDIALOG_H
#define SHUFFLINGDIALOG_H

#include <QDialog>
#include <QThread>
#include "lottery.h"

namespace Ui {
class ShufflingDialog;
}

class ShufflingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShufflingDialog(QWidget *parent = nullptr);
    ~ShufflingDialog();
    const Lottery::ShuffleR& result(){return _result;}

    void closeEvent(QCloseEvent *) override;
private slots:
        void on_timeout();
        void handleResults();
private:
    Ui::ShufflingDialog *ui;
    QTimer *t;
    int p;
    Lottery::ShuffleR _result;
    class WorkerThread* _workerThread;
};

class WorkerThread : public QThread
{
    Q_OBJECT

public:
    int *p;
    int k=7;
    Lottery::ShuffleR r;
    void run() override
    {
        r= Lottery::Shuffle(p, k);

        if(r.num.count()>5){
            auto a = Lottery::Select(r.num, 5);
            r.comb = Lottery::Filter(a);
        }
        else{
            Lottery::Data d0;
            for(int j=1;j<=5;j++) d0.setNumber(j, r.num[j]);
            if(d0.TestAll()) r.comb.append(d0);
        }

        emit resultReady();
    }
signals:
    void resultReady();
};

#endif // SHUFFLINGDIALOG_H
