#include "label.h"
#include "ui_label.h"

Label::Label(std::shared_ptr<cache> & ch, const QDir & path, const QDir & system,
             const QByteArray & pos, const quint16 remFilter, QTextCodec * codec, QWidget *parent) :
    QDialog(parent), ui(new Ui::Label)
{
    ui->setupUi(this);
    setFixedSize(700, 700);
    setLayout(ui->gridLayout);
    ui->tab->setLayout(ui->verticalLayout);
    ui->tab_2->setLayout(ui->gridLayout_2);
    _scene = new customScene(ui->tab_2);
    ui->graphicsView->setScene(_scene);

    _cache = ch;
    _remFilter = remFilter;
    _filename = path;
    _system = system;

    if(codec == nullptr)
        _codec = QTextCodec::codecForName("system");
    else
        _codec = codec;

    _engine = new repertoryEngine(_filename, _cache, _scene, _codec);

    //_symptom.open(path.filePath("symptom").toStdString());
    _engine->setCurrentKey(pos);
    _engine->setChaptersFilter(remFilter);
    _engine->setFilter(repertoryFilterType::remeds);
    _engine->setSortingRemeds(true);
    _engine->setRemedsCounter(false);
    _engine->setGetLinksStrings(true);
    _engine->render(height(), width() - 10, true);

    _linksNames[0] = _engine->synomyList();
    _linksNames[1] = _engine->masterReferensesList();
    _linksNames[2] = _engine->crossReferensesList();

    if(!_localize){
        ui->label_4->setHidden(true);
        ui->listWidget_2->setHidden(true);
    }

    ui->label->setText(_engine->renderingLabel());

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

    if(_linksNames[0].isEmpty())
        ui->listWidget_3->item(0)->setHidden(true);
    if(_linksNames[1].isEmpty())
        ui->listWidget_3->item(1)->setHidden(true);
    if(_linksNames[2].isEmpty())
        ui->listWidget_3->item(2)->setHidden(true);

    connect(ui->listWidget_3, &QListWidget::itemClicked, this, &Label::showTextInformation);
    connect(_scene, &customScene::doubleClick, this, &Label::clickedAction);

    for(auto i = 0; i != 3; ++i){
        if(!ui->listWidget_3->item(i)->isHidden()){
            ui->listWidget_3->setCurrentRow(i);
            emit ui->listWidget_3->itemClicked(ui->listWidget_3->item(i));
            break;
        }
    }
}
void Label::showTextInformation(QListWidgetItem * item){
    auto addStr = [&](auto & list){
        ui->listWidget->clear();
        ui->listWidget_2->clear();

        for(auto it = 1; it != list.size() + 1; ++it){
            if(!list.at(it - 1).isEmpty()){
                if(it % 2 != 0 || !_localize)
                    ui->listWidget->addItem(list.at(it - 1));
                else
                    ui->listWidget_2->addItem(list.at(it - 1));
            }
        }
    };

    addStr(_linksNames[ui->listWidget_3->row(item)]);
}
Label::~Label()
{
    delete ui;
}
void Label::clickedAction(const QGraphicsSimpleTextItem * item){
    //0 - label num, 1 - (see, 2 - links, 3 - remed, 4 - author
    if(!item->data(0).isValid())
        return;

    QWidget * widget = nullptr;

    switch (item->data(0).toInt()) {
        case 0 :{
            widget = new Label(_cache, _filename,
                                     _system , item->data(1).toByteArray(), _engine->chaptersFilter(), _codec, this);
            break;
        }
        case 1 :
            return;
        case 2 :{
            return;
        }
        case 3 : {
            widget = new remed_author(_filename, _system, _cache, item->data(2).toByteArray()
                                          , _engine->chaptersFilter(), item->data(1).toUInt(), this);
            break;
        }
        case 4 : {
            widget = new author(_system, item->data(1).toUInt(), _cache, this);
            break;
        }
        default:
            return;
    }

    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->show();
}
