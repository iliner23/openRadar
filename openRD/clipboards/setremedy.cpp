#include "setremedy.h"
#include "ui_setremedy.h"

setRemedy::setRemedy(std::shared_ptr<std::array<QVector<rci>, 10>> clipRemed, std::pair<qsizetype, qsizetype> pos, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setRemedy)
{
    ui->setupUi(this);

    for(auto i = 0; i != 10; ++i)
        ui->comboBox->addItem(QString::number(i));

    _clipRemed = clipRemed;
    _pos = pos;

    const auto symData = _clipRemed->at(pos.first).at(pos.second);
    openCtree rep(symData.path.filePath("symptom").toStdString());
    rep.at(symData.key.toStdString(), false);
    ui->label->setText(func::renderingLabel(rep, false, symData.codec));

    setDefault();
}
void setRemedy::accept(){
    _clipRemed->at(_pos.first)[_pos.second].cas = ui->checkBox->isChecked();
    _clipRemed->at(_pos.first)[_pos.second].elim = ui->checkBox_2->isChecked();

    _clipRemed->at(_pos.first)[_pos.second].group = ui->lineEdit->text();
    _clipRemed->at(_pos.first)[_pos.second].intensity = ui->comboBox->currentIndex();

    _clipRemed->at(_pos.first)[_pos.second].measure[0] = ui->checkBox_3->isChecked();
    _clipRemed->at(_pos.first)[_pos.second].measure[1] = ui->checkBox_4->isChecked();
    _clipRemed->at(_pos.first)[_pos.second].measure[2] = ui->checkBox_5->isChecked();
    _clipRemed->at(_pos.first)[_pos.second].measure[3] = ui->checkBox_6->isChecked();

    emit clipboardRemedChanged();
    close();
}
void setRemedy::setDefault(){
    ui->checkBox->setChecked(_clipRemed->at(_pos.first)[_pos.second].cas);
    ui->checkBox_2->setChecked(_clipRemed->at(_pos.first)[_pos.second].elim);

    ui->lineEdit->setText(_clipRemed->at(_pos.first)[_pos.second].group);
    ui->comboBox->setCurrentIndex(_clipRemed->at(_pos.first)[_pos.second].intensity);

    ui->checkBox_3->setChecked(_clipRemed->at(_pos.first)[_pos.second].measure[0]);
    ui->checkBox_4->setChecked(_clipRemed->at(_pos.first)[_pos.second].measure[1]);
    ui->checkBox_5->setChecked(_clipRemed->at(_pos.first)[_pos.second].measure[2]);
    ui->checkBox_6->setChecked(_clipRemed->at(_pos.first)[_pos.second].measure[3]);
}
setRemedy::~setRemedy()
{
    delete ui;
}
