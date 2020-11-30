#include "combinationdialog.h"
#include "ui_combinationdialog.h"
#include "common/helper/string/stringhelper.h"


CombinationDialog::CombinationDialog(QWidget *parent) :
                                                        QDialog(parent),
                                                        ui(new Ui::CombinationDialog)
{
    ui->setupUi(this);
}

CombinationDialog::~CombinationDialog()
{
    delete ui;
}

void CombinationDialog::setUi(const Lottery::ShuffleR &m)
{
    QString e;

    for(auto&i:m.num){
        if(!e.isEmpty()) e+=",";
        e+=QString::number(i);
        //Lottery::_shuffled.insert(i);
    }

    ui->label_num->setText("számok: " +e);
    ui->label_com->setText(QString("kombinációk: %1 db").arg(m.comb.count()));

    QString txt;
    if(!m.comb.isEmpty()){
//        int o = m.comb.count()/30;
//        int o0=0;
        for(auto&i:m.comb){
            if(!txt.isEmpty()) txt+="\t";//com::helper::StringHelper::NewLine;
//            if(!o0%o)//nincs maradék
//            {
//                if(!txt.isEmpty()) txt+=com::helper::StringHelper::NewLine;
//            }
//            else
//            {
//                if(!txt.isEmpty()) txt+="\t";
//            }
//            o0++;
            txt+= i.NumbersToString();
        }
    }
    ui->textBrowser->setText(txt);
}
