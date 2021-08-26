#include "researchremed.h"
#include "ui_researchremed.h"

researchRemed::researchRemed(QStringList clipNames, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::researchRemed){
    ui->setupUi(this);
    _clipNames = clipNames;

    for(auto it = 0; it != (int) _labels.size(); ++it){
        _labels.at(it).widget = new QWidget(ui->listWidget);
        //_labels.at(it).widget->hide();

        auto layout = new QVBoxLayout(_labels.at(it).widget);

        _labels.at(it).label = new QLabel(_labels.at(it).widget);
        _labels.at(it).label->setText(_clipNames.at(it));
        QPalette pal = _labels.at(it).label->palette();
        pal.setColor(_labels.at(it).label->backgroundRole(), Qt::yellow);
        _labels.at(it).label->setPalette(pal);

        _labels.at(it).list = new QListWidget(_labels.at(it).widget);

        layout->addWidget(_labels.at(it).label);
        layout->addWidget(_labels.at(it).list);
    }
}
void researchRemed::setClipboard(quint8 pos){  
    if(pos <= 9){
        /*for(auto & it : _labels)
            it.widget->hide();*/

        _pos = pos;
        _labels.at(pos).list->clear();

        for(auto & it : _clipboadrs.at(_pos)){
            openCtree data(it.path.filePath("symptom").toStdString());
            data.at(it.key.toStdString(), false);
            _labels.at(pos).list->addItem(func::renderingLabel(data, false, it.codec));
            //_labels.at(pos).widget->show();
        }
    }
}
void researchRemed::setClipboardName(QStringList name){
    _clipNames = name;
}
void researchRemed::setClipboardRemed(std::array<QVector<rci>, 10> array){
    _clipboadrs = array;
}
researchRemed::~researchRemed(){
    delete ui;
}
