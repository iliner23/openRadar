#include "searchresult.h"
#include "ui_searchresult.h"

searchResult::searchResult(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::searchResult)
{
    ui->setupUi(this);
}

searchResult::~searchResult()
{
    delete ui;
}
