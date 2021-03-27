#include "levels.h"
#include "ui_levels.h"

Levels::Levels(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Levels)
{
    ui->setupUi(this);
}
void Levels::showLevels(const QString & path){
    openCtree view(path.toStdString());
    auto codec = QTextCodec::codecForName("system");

    for(auto i = 0; i != view.dataEntries(); ++i){
        auto str = view.next();
        auto it = std::find(str.cbegin() + view.getReclen(), str.cend(), '\0');
        str = std::string(str.cbegin() + view.getReclen(), it);
        ui->listWidget->addItem(codec->toUnicode(str.c_str()));
    }

    ui->listWidget->setCurrentRow(0);
    show();
}
Levels::~Levels()
{
    delete ui;
}
void Levels::reject(){
    ui->listWidget->clear();
    QDialog::reject();
}
void Levels::accept(){
    QDialog::accept();

    quint16 rem = 1;
    const auto maxRow = ui->listWidget->currentRow();
    for(auto i = 0; i != maxRow; ++i){
        rem <<= 1;
    }

    ui->listWidget->clear();
    emit sendLevel(rem);
}
