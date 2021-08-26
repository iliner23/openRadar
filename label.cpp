#include "label.h"
#include "ui_label.h"

Label::Label(std::shared_ptr<func::cache> ch, const QDir path,
             const QByteArray pos, const quint16 remFilter, std::pair<QLocale, QLocale> lang, keysRemedList *remedList, QWidget *parent) :
    QDialog(parent), ui(new Ui::Label)
{
    ui->setupUi(this);
    _scene = new customScene(ui->tab_2);
    ui->graphicsView->setScene(_scene);
    setFixedSize(size());

    _cache = ch;
    _filename = path;
    _remedList = remedList;

    ui->label_3->setText(lang.first.nativeLanguageName());
    ui->label_4->setText(lang.second.nativeLanguageName());

    _codec = QTextCodec::codecForName(lang::chooseCodec(lang));
    _engine.reset(_filename.filePath("symptom"), _cache, _codec);

    _engine.setCurrentKey(pos);
    _engine.setChaptersFilter(remFilter);
    _scene->addItem(_engine.render(QSize(height(), width() - 40)).front());

    _synonyms = _engine.synonymList();
    _links = _engine.masterReferensesList();
    _crossLinks = _engine.crossReferensesList();;

    const auto rem = _engine.remedsCount();

    _remedSize[0] = rem.at(0);
    _remedSize[1] = rem.at(1);
    _remedSize[2] = rem.at(2);
    _remedSize[3] = rem.at(3);

    if(QLocale::AnyLanguage == lang.second){
        ui->label_4->setHidden(true);
        ui->listWidget_2->setHidden(true);
    }

    ui->label->setText(_engine.renderingLabel());

    ui->label_2->setFont(QFont("default", 10));
    ui->label_2->setText(QString::number(_remedSize[0] + _remedSize[1]
                                        + _remedSize[2] + _remedSize[3]) + " Препараты : " +
                         QString::number(_remedSize[3]) + " + " +
                         QString::number(_remedSize[2]) + " + " +
                         QString::number(_remedSize[1]) + " + " +
                         QString::number(_remedSize[0]));
    ui->label->setFont(QFont("default", 10));

    ui->listWidget_3->addItem("Синонимы");
    ui->listWidget_3->addItem("Мастер синонимы");
    ui->listWidget_3->addItem("Перекрестные ссылки");

    if(_synonyms.first.isEmpty() && _synonyms.second.isEmpty())
        ui->listWidget_3->item(0)->setHidden(true);
    if(_links.first.isEmpty() && _links.second.isEmpty())
        ui->listWidget_3->item(1)->setHidden(true);
    if(_crossLinks.first.isEmpty() && _crossLinks.second.isEmpty())
        ui->listWidget_3->item(2)->setHidden(true);

    connect(ui->listWidget_3, &QListWidget::itemSelectionChanged, this, &Label::showTextInformation);
    connect(_scene, &customScene::doubleClick, this, &Label::clickedAction);

    for(auto i = 0; i != 3; ++i){
        if(!ui->listWidget_3->item(i)->isHidden()){
            ui->listWidget_3->setCurrentRow(i);
            break;
        }
    }
}
void Label::showTextInformation(){
    ui->listWidget->clear();
    ui->listWidget_2->clear();

    if(ui->listWidget_3->currentRow() == 0){
        ui->listWidget->addItems(QStringList(_synonyms.first));
        ui->listWidget_2->addItems(QStringList(_synonyms.second));
    }
    else if(ui->listWidget_3->currentRow() == 1){
        ui->listWidget->addItems(_links.first);
        ui->listWidget_2->addItems(_links.second);
    }
    else{
        ui->listWidget->addItems(_crossLinks.first);
        ui->listWidget_2->addItems(_crossLinks.second);
    }
}
void Label::clickedAction(const QGraphicsSimpleTextItem * item){
    //0 - label num, 1 - (see, 2 - links, 3 - remed, 4 - author
    if(!item->data(0).isValid())
        return;

    QWidget * widget = nullptr;

    switch (item->data(0).toInt()) {
        case 3 : {
            widget = new remed_author(_filename, _cache, item->data(2).toByteArray()
                                          , _engine.chaptersFilter(), item->data(1).toUInt(),
                                      _remedList, _codec, this);
            break;
        }
        case 4 : {
            widget = new author(item->data(1).toUInt(), _cache, this);
            break;
        }
        default:
            return;
    }

    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->show();
}
Label::~Label(){
    delete ui;
}
