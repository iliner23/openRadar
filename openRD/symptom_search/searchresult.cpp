#include "searchresult.h"
#include "ui_searchresult.h"

searchResult::searchResult(QWidget * parent):
    QDialog(parent),
    ui(new Ui::searchResult)
{
    ui->setupUi(this);
}
searchResult::searchResult(const QFileInfo word, const QFileInfo symptom, const QString expression, QTextCodec * codec, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::searchResult)
{
    ui->setupUi(this);
    setData(word, symptom, expression, codec);
}
void searchResult::setData(const QFileInfo word, const QFileInfo symptom, const QString expression, QTextCodec * codec){
    _codec = codec;

    _word.open(word.filePath().toStdString());
    _symptom.open(symptom.filePath().toStdString());

    const auto list = func::linksParser()(_symptom, _word, expression, codec);
    ui->label->setText(expression);

    ui->listWidget->setUpdatesEnabled(false);

    ui->listWidget->addItems(list.first);
    _keys = list.second;

    ui->label_2->setText(QString::number(list.first.size()));
    ui->listWidget->setUpdatesEnabled(true);
}
void searchResult::acceptKey(QListWidgetItem * item){
    ui->pushButton->setEnabled(item != nullptr);
}
searchResult::~searchResult()
{
    delete ui;
}
void searchResult::clearValues(){
    ui->listWidget->clear();
    _keys.clear();
    ui->pushButton->setEnabled(false);
}
void searchResult::reject(){
    clearValues();
    QDialog::reject();
}
void searchResult::accept(){
    _key = _keys.at(ui->listWidget->currentRow());
    clearValues();
    QDialog::accept();
}
