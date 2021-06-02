#include "vocabulary.h"
#include "ui_vocabulary.h"

vocabulary::vocabulary(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::vocabulary)
{
    ui->setupUi(this);
    setLayout(ui->verticalLayout);
}

vocabulary::~vocabulary()
{
    delete ui;
}
