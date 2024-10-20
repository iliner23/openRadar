#include "takeremed.h"
#include "ui_takeremed.h"

takeRemed::takeRemed(std::shared_ptr<QStringList> name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::takeRemed)
{
    ui->setupUi(this);

    _name = name;

    for(auto i = 0; i != 10; ++i)
        ui->comboBox->addItem(QString::number(i));

    ui->comboBox->setCurrentIndex(1);
}
void takeRemed::setDefault(){
    ui->checkBox_3->setChecked(true);
    ui->checkBox_4->setChecked(true);
    ui->checkBox_5->setChecked(true);
    ui->checkBox_6->setChecked(true);

    ui->lineEdit->clear();
    ui->checkBox_2->setChecked(false);

    ui->checkBox_7->setChecked(false);
    ui->checkBox_8->setChecked(false);

    ui->checkBox_9->setChecked(true);
    ui->checkBox_10->setChecked(false);
    ui->checkBox_11->setChecked(false);
    ui->checkBox_12->setChecked(false);
    ui->checkBox_13->setChecked(false);
    ui->checkBox_14->setChecked(false);
    ui->checkBox_15->setChecked(false);
    ui->checkBox_16->setChecked(false);
    ui->checkBox_17->setChecked(false);
    ui->checkBox_18->setChecked(false);

    setClipboardsName();

    ui->comboBox->setCurrentIndex(1);
}
void takeRemed::showEvent(QShowEvent *event){
    setDefault();
    event->ignore();
}
void takeRemed::setClipboardsName(){
    ui->lineEdit_2->setText(_name->at(0));
    ui->lineEdit_3->setText(_name->at(1));
    ui->lineEdit_4->setText(_name->at(2));
    ui->lineEdit_5->setText(_name->at(3));
    ui->lineEdit_6->setText(_name->at(4));
    ui->lineEdit_7->setText(_name->at(5));
    ui->lineEdit_8->setText(_name->at(6));
    ui->lineEdit_9->setText(_name->at(7));
    ui->lineEdit_10->setText(_name->at(8));
    ui->lineEdit_11->setText(_name->at(9));
}
void takeRemed::accept(){
    QDialog::accept();

    (*_name)[0] = ui->lineEdit_2->text();
    (*_name)[1] = ui->lineEdit_3->text();
    (*_name)[2] = ui->lineEdit_4->text();
    (*_name)[3] = ui->lineEdit_5->text();
    (*_name)[4] = ui->lineEdit_6->text();
    (*_name)[5] = ui->lineEdit_7->text();
    (*_name)[6] = ui->lineEdit_8->text();
    (*_name)[7] = ui->lineEdit_9->text();
    (*_name)[8] = ui->lineEdit_10->text();
    (*_name)[9] = ui->lineEdit_11->text();

    //emit changeClipboardsName();
    //setClipboardsName();

    func::remedClipboardInfo info;
    info.intensity = ui->comboBox->currentIndex();
    info.elim = ui->checkBox_8->isChecked();
    info.cas = ui->checkBox_7->isChecked();
    info.group = ui->lineEdit->text();

    info.measure[0] = ui->checkBox_3->isChecked();
    info.measure[1] = ui->checkBox_4->isChecked();
    info.measure[2] = ui->checkBox_5->isChecked();
    info.measure[3] = ui->checkBox_6->isChecked();

    info.codec = _codec;
    info.path = _repertory;
    info.key = _key;
    info.remFilter = _filter;

    std::array<QCheckBox*, 10> array = {
        ui->checkBox_9, ui->checkBox_10, ui->checkBox_11, ui->checkBox_12,
        ui->checkBox_13, ui->checkBox_14, ui->checkBox_15, ui->checkBox_16,
        ui->checkBox_17, ui->checkBox_18 };

    for(auto it = 0; it != array.size(); ++it){
        if(array.at(it)->isChecked())
            emit addedClipboardsRemed(info, it);
    }
}
void takeRemed::setPos(QDir repertory, QByteArray key, quint16 filter, QTextCodec * codec){
    _repertory = repertory;
    _key = key;
    _codec = codec;
    _filter = filter;

    openCtree rep(_repertory.filePath("symptom").toStdString());
    rep.at(_key.toStdString(), false);

    ui->label->setText(func::renderingLabel(rep, false, _codec));
}
takeRemed::~takeRemed(){
    delete ui;
}
