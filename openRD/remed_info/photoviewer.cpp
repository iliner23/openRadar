#include "photoviewer.h"
#include "ui_photoviewer.h"

photoViewer::photoViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::photoViewer)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::Window);
    ui->photo->setScaledContents(true);
}
void photoViewer::show(const QList<QPixmap> & photos){
    _photos = photos;

    if(_photos.isEmpty())
        throw std::logic_error("no photos loaded");

    setPos(0);
    QWidget::show();
}
void photoViewer::back(){
    setPos(_pos - 1);
}
void photoViewer::forward(){
    setPos(_pos + 1);
}
void photoViewer::setPos(int pos){
    if(pos < 0)
        _pos = 0;
    else if(pos >= _photos.size())
        _pos = _photos.size() - 1;
    else
        _pos = pos;

    ui->back->setEnabled(pos > 0);
    ui->forward->setEnabled(pos < _photos.size() - 1);

    ui->photo->setPixmap(_photos.at(_pos));

}
photoViewer::~photoViewer()
{
    delete ui;
}
