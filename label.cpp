#include "label.h"
#include "ui_label.h"

Label::Label(std::shared_ptr<cache> & ch, const QDir & path, const QDir & system,
             const QByteArray & pos, const quint16 remFilter, QTextCodec * codec, QWidget *parent) :
    QDialog(parent), abstractEngine(codec), ui(new Ui::Label)
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

    _symptom.open(path.filePath("symptom").toStdString());
    _index = pos;

    renderingView(height(), width() - 10);

    if(!_localize){
        ui->label_4->setHidden(true);
        ui->listWidget_2->setHidden(true);
    }

    ui->label->setText(renderingLabel());

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
    connect(_scene, &customScene::labelActivated, this, &Label::clickedAction);

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

void Label::renderingView(const int heightView, const int widthView){
    _scene->clear();
    _heightView = heightView;
    _widthView = widthView;

    Q_ASSERT(!_index.isEmpty());
    _fullStr = QByteArray::fromStdString(_symptom.at(_index.toStdString()));

    auto returnSize = [](const auto & value, const auto & size){
        return (value == -1) ? size : value;
    };

    quint8 attach = 0;
    quint16 maxDrug = 0, filter = 0;
    int labelWidth = 0;

    attach = qFromLittleEndian<quint8>(_fullStr.constData() + 21);
    maxDrug = qFromLittleEndian<quint16>(_fullStr.constData() + 22);
    filter = qFromLittleEndian<quint16>(_fullStr.constData() + 26);

    if(attach == 0)
        return;

    bool hideLabel = !(_remFilter == (quint16)-1 || (filter & _remFilter) != 0);
    bool localize = false;
    _labelsEnd = 0;

    {//label subfunction
        const auto firstZero = returnSize(_fullStr.indexOf('\0', _symptom.serviceDataLenght()), _fullStr.size());
        const auto secondZero = returnSize(_fullStr.indexOf('\0', firstZero + 1), _fullStr.size());

        _labelsEnd = secondZero;
        _localize = localize = (secondZero - firstZero - 1 <= 0) ? false : true;
    }
    if(!hideLabel){
        {//synonyms and links
            quint8 type = 4;

            for(auto i = 0; i != 4; ++i){
                quint8 cnt = 0;

                for(auto it = 0; it != type + 1; ++it){
                    if(_labelsEnd == _fullStr.size() || _fullStr.at(_labelsEnd) != '\0')
                        break;

                    ++cnt;
                    ++_labelsEnd;
                }

                if(cnt > type)
                    break;

                type = type - cnt;

                const auto tmpLinkIter = returnSize(_fullStr.indexOf('\0', _labelsEnd), _fullStr.size());
                auto synLinkText =
                        _codec->toUnicode(_fullStr.mid(_labelsEnd, tmpLinkIter - _labelsEnd));
                synLinkText.prepend(' ');
                _labelsEnd = tmpLinkIter;

                if(synLinkText.size() > 2 && synLinkText.left(3) == " (="){
                    const auto sz = _linksNames[0].size() + 1;
                    if(type == 3){
                        if(sz % 2 == 0)
                            _linksNames[0].push_back("");
                    }
                    else if(type == 2){
                        if((localize && sz % 2 != 0) ||
                          (!localize && sz % 2 == 0))
                            _linksNames[0].push_back("");
                    }
                    else
                        continue;

                    _linksNames[0].push_back(synLinkText.mid(1));
                }
                else{
                    for(auto it = 0; it != synLinkText.size(); ){
                        const auto iterEnd = returnSize(synLinkText.indexOf('/', it), synLinkText.size());
                        QString linkText;

                        if(iterEnd != synLinkText.size()){
                            linkText = synLinkText.mid(it + 1, iterEnd - it - 2);
                            it = iterEnd + 1;
                        }
                        else{
                            linkText = synLinkText.mid(it + 1, iterEnd - it - 1);
                            it = iterEnd;
                        }

                        const auto lnIndex = (type > 1) ? 1 : 2;
                        const auto delFor2 = (type > 1) ? _linksNames[1].size() + 1 : _linksNames[2].size() + 1;

                        if(type == 3 || type == 1){
                            if(delFor2 % 2 == 0)
                                _linksNames[lnIndex].push_back("");

                            _linksNames[lnIndex].push_back(linkText);
                        }
                        else{
                            if((localize && delFor2 % 2 != 0) ||
                              (!localize && delFor2 % 2 == 0))
                                _linksNames[lnIndex].push_back("");

                            _linksNames[lnIndex].push_back(linkText);
                        }
                    }
                }

                if(type == 0)
                    break;
            }
        }
        {//remeds
            QVector<QVector<QGraphicsItemGroup*>> array(4);
            remedRender(array, true);

            for(auto it = 3; it != -1; --it){
                _remedSize[it] = array[it].size();

                for(auto & iter : array[it])
                    addRemeds(iter, labelWidth, _widthView);
            }
        }
    }
}
void Label::clickedAction(const QGraphicsSimpleTextItem * item){
    //0 - label num, 1 - (see, 2 - links, 3 - remed, 4 - author
    if(!item->data(0).isValid())
        return;

    QWidget * widget = nullptr;

    switch (item->data(0).toInt()) {
        case 0 :{
            widget = new Label(_cache, _filename,
                                     _system , item->data(1).toByteArray(), _remFilter, _codec, this);
            break;
        }
        case 1 :
            return;
        case 2 :{
            return;
        }
        case 3 : {
            widget = new remed_author(_filename, _system, _cache, item->data(2).toByteArray()
                                          , _remFilter, item->data(1).toUInt(), this);
            break;
        }
        case 4 : {
            widget = new author(_system, item->data(1).toUInt(), _cache, this);
            break;
        }
    }

    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->show();
}
