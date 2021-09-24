#include "researchremed.h"
#include "ui_researchremed.h"

researchRemed::researchRemed(QStringList clipNames, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::researchRemed){
    ui->setupUi(this);
    _clipNames = clipNames;

    for(auto it = 0; it != (int) _labels.size(); ++it){
        _labels.at(it).widget = new QWidget(ui->scrollAreaWidgetContents);
        _labels.at(it).widget->hide();

        auto layout = new QVBoxLayout(_labels.at(it).widget);

        _labels.at(it).label = new QLabel(_labels.at(it).widget);
        _labels.at(it).list = new QListWidget(_labels.at(it).widget);

        layout->addWidget(_labels.at(it).label);
        layout->addWidget(_labels.at(it).list);
    }
}
void researchRemed::setClipboards(std::array<bool, 10> act){
    for(auto & it : _labels){
        it.widget->hide();
        it.widget->move(0, 0);
        it.list->clear();
    }

    QWidget * prev = nullptr;

    for(auto iter = 0; iter != act.size(); ++iter){
        if(!act.at(iter))
            continue;

        for(auto & it : _clipboadrs.at(iter)){
            openCtree data(it.path.filePath("symptom").toStdString());
            data.at(it.key.toStdString(), false);
            _labels.at(iter).list->addItem(func::renderingLabel(data, false, it.codec));
        }

        _labels.at(iter).label->setText(_clipNames.at(iter));

        if(prev != nullptr)
            _labels.at(iter).widget->move(0, prev->y() + prev->height() + 5);

        _labels.at(iter).widget->show();
        prev = _labels.at(iter).widget;
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
