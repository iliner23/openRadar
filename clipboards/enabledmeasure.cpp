#include "enabledmeasure.h"
#include "ui_enabledmeasure.h"

enabledMeasure::enabledMeasure(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::enabledMeasure)
{
    ui->setupUi(this);
}

enabledMeasure::~enabledMeasure()
{
    delete ui;
}

void enabledMeasure::setValues(const std::array<bool, 4> & values){
    ui->checkBox->setChecked(values[0]);
    ui->checkBox_2->setChecked(values[1]);
    ui->checkBox_3->setChecked(values[2]);
    ui->checkBox_4->setChecked(values[3]);
}
std::array<bool, 4> enabledMeasure::getValues() const{
    std::array<bool, 4> values;

    values[0] = ui->checkBox->isChecked();
    values[1] = ui->checkBox_2->isChecked();
    values[2] = ui->checkBox_3->isChecked();
    values[3] = ui->checkBox_4->isChecked();

    return values;
}
