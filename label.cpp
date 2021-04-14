#include "label.h"
#include "ui_label.h"

Label::Label(const cache & ch, const QDir & path, const QDir & system,
             const quint32 pos, const quint16 remFilter, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Label)
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
    _path = path;
    _system = system;

    _sym.open(path.filePath("symptom").toStdString());
    _pos = pos;

    rendering();

    ui->label_2->setFont(QFont("default", 10));
    ui->label_2->setText(QString::number(_rem1 + _rem2 + _rem3 + _rem4) + " Препараты : " +
                         QString::number(_rem4) + " + " +
                         QString::number(_rem3) + " + " +
                         QString::number(_rem2) + " + " +
                         QString::number(_rem1));
    ui->label->setFont(QFont("default", 10));

    ui->listWidget_3->addItem("Синонимы");
    ui->listWidget_3->addItem("Мастер синонимы");
    ui->listWidget_3->addItem("Перекрестные ссылки");

    if(_synomSL.isEmpty())
        ui->listWidget_3->item(0)->setHidden(true);
    if(_masterSL.isEmpty())
        ui->listWidget_3->item(1)->setHidden(true);
    if(_referSL.isEmpty())
        ui->listWidget_3->item(2)->setHidden(true);

    connect(ui->listWidget_3, &QListWidget::itemClicked, this, &Label::showTextInformation);

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

    switch (ui->listWidget_3->row(item)) {
    case 0:
        addStr(_synomSL);
        break;
    case 1:
        addStr(_masterSL);
        break;
    case 2:
        addStr(_referSL);
        break;
    }
}
Label::~Label()
{
    delete ui;
}

void Label::rendering(){
    auto codec = QTextCodec::codecForName("system");
    const auto lenSym = _sym.serviceDataLenght();

    const QString fontName("cursive");
    const auto symptom = _sym.at(_pos);

    QRectF size;
    QPointF pos;
    std::string label2;

    qint8 symbol;
    symbol = symptom.at(21);
    quint16 maxDrug = 0, filter = 0;
    ((char *)&maxDrug)[0] = symptom.at(22);
    ((char *)&maxDrug)[1] = symptom.at(23);
    ((char *)&filter)[0] = symptom.at(26);
    ((char *)&filter)[1] = symptom.at(27);

    bool hideLabel = !(_remFilter == (quint16)-1 || (filter & _remFilter) != 0);

    auto firstIt = std::find(symptom.cbegin() + lenSym, symptom.cend(), '\0');
    auto secondIt = std::find(firstIt + 1, symptom.cend(), '\0');
    {//Label subfunction
        label2 = std::string(firstIt + 1, secondIt);

        if(!label2.empty())
            _localize = true;
    }
    if(_localize == false){
        ui->label_4->hide();
        ui->listWidget_2->hide();
    }
    if(!hideLabel){
        {//synonyms and links
            uint8_t max = 4;
            for(uint8_t i = 0; i != 4; ++i){
                uint8_t cnt = 0;

                for(auto it = 0; it != max + 1; ++it){
                    if(secondIt != symptom.cend() && *secondIt == '\0'){
                        ++cnt;
                        ++secondIt;
                    }
                    else
                        break;
                }

                if(cnt > max)
                    break;

                auto iter = std::find(secondIt, symptom.cend(), '\0');
                auto strg = std::string(secondIt, iter);
                strg.insert(strg.begin(), ' ');
                secondIt = iter;

                if(strg.size() > 2 && strg.substr(0, 3) == " (="){
                    const auto sz = _synomSL.size() + 1;
                    if(max - cnt == 3){
                        if(sz % 2 == 0)
                            _synomSL.push_back("");
                    }
                    else if(max - cnt == 2){
                        if((_localize && sz % 2 != 0) ||
                          (!_localize && sz % 2 == 0))
                            _synomSL.push_back("");
                    }
                    else
                        continue;

                    _synomSL.push_back(codec->toUnicode(std::string(strg.begin() + 1, strg.end()).c_str()));
                }
                else{
                    for(auto it = strg.begin(); it != strg.end();){
                        auto iter = std::find(it, strg.end(), '/');
                        std::string tpstr;

                        if(iter != strg.end()){
                            tpstr = std::string(it + 1, iter - 1);
                            it = iter + 1;
                        }
                        else{
                            tpstr = std::string(it + 1, iter);
                            it = iter;
                        }

                        const auto sr = _referSL.size() + 1;
                        const auto sm = _masterSL.size() + 1;

                        if(max - cnt == 3){
                            if(sm % 2 == 0)
                                _masterSL.push_back("");

                            _masterSL.push_back(codec->toUnicode(tpstr.c_str()));
                        }
                        else if(max - cnt == 2){
                            if((_localize && sm % 2 != 0) ||
                              (!_localize && sm % 2 == 0))
                                _masterSL.push_back("");

                            _masterSL.push_back(codec->toUnicode(tpstr.c_str()));
                        }
                        else if(max - cnt == 1){
                            if(sr % 2 == 0)
                                _referSL.push_back("");

                            _referSL.push_back(codec->toUnicode(tpstr.c_str()));
                        }
                        else if(max - cnt == 0){
                            if((_localize && sr % 2 != 0) ||
                              (!_localize && sr % 2 == 0))
                                _referSL.push_back("");

                            _referSL.push_back(codec->toUnicode(tpstr.c_str()));
                        }
                    }
                }

                if(max - cnt == 0)
                    break;

                max = max - cnt;
            }
        }
        {//remed
            auto pred = [](auto it){
                if(it != '\0')
                    return true;
                return false;
            };

            quint16 prevRem = 0;
            const auto baseX = pos.x();

            auto horizon = [&](auto * temp){
                if(temp == nullptr)
                    return;

                const auto bon = temp->boundingRect();
                if(pos.x() + size.width() + bon.width() + 5 >= width() - 20){
                    temp->setX(baseX);
                    temp->setY(pos.y() + size.height());
                }
                else{
                    temp->setX(pos.x() + size.width());
                    temp->setY(pos.y());
                }

                _scene->addItem(temp);
                size = bon;
                pos = temp->pos();
            };

            std::vector<QGraphicsItemGroup*> array_1, array_2, array_3, array_4;
            array_1.reserve(maxDrug / 4 + 1);
            array_2.reserve(maxDrug / 4 + 1);
            array_3.reserve(maxDrug / 4 + 1);
            array_4.reserve(maxDrug / 4 + 1);

            QGraphicsItemGroup * allrm = nullptr;
            qint8 type = 0;

            auto authorsSym = [&](auto autr, const auto author, const bool next = false){
                auto aut = new customItem;
                if(autr == "kl2")
                    aut->setPlainText("*");
                else if(autr == "zzz")
                    aut->setPlainText(u8"\u2193");
                else{
                    if(!next)
                        aut->setPlainText(QString::fromStdString(autr));
                    else
                        aut->setPlainText(", " + QString::fromStdString(autr));
                }

                aut->setDefaultTextColor(Qt::magenta);
                aut->setFont(QFont(fontName, 8));
                aut->setX(allrm->boundingRect().width());

                aut->setData(0, 4);
                aut->setData(1, author);

                allrm->addToGroup(aut);
            };

            for(auto it = std::find_if(secondIt, symptom.cend(), pred); it != symptom.cend(); ++secondIt){
                //if(*it == 0)
                    //break;

                quint16 remed = 0, author = 0, tLevel = 0;
                uint8_t rLevel;

                if(symptom.cend() - secondIt <= 3)
                    break;

                ((char *)&remed)[0] = *secondIt;
                ((char *)&remed)[1] = *(++secondIt);

                rLevel = *(++secondIt);

                ((char *)&author)[0] = *(++secondIt);
                ((char *)&author)[1] = *(++secondIt);

                ((char *)&tLevel)[0] = *(++secondIt);
                ((char *)&tLevel)[1] = *(++secondIt);

                if(((char *)&remed)[0] == '\x5a' && ((char *)&remed)[1] == '\x5a' && rLevel == '\x5a')
                    break;

                if(_remFilter == (quint16)-1 || (tLevel & _remFilter) != 0){
                    if((((char *)&author)[1] & (char)128) != 0){
                        ((char *)&author)[1] ^= (char)128;//remed have * in the end
                    }

                    const auto & rmStr = _cache._cacheRemed.at(remed);
                    const auto & auStr = _cache._cacheAuthor.at(author);
                    auto itN = rmStr.find('\0', _cache._lenRem);
                    auto itA = auStr.find('\0', _cache._lenAuthor);
                    auto autr = auStr.substr(_cache._lenAuthor, itA - _cache._lenAuthor);

                    if(prevRem != remed){
                        if(allrm != nullptr){
                            switch (type) {
                            case 1 :
                                array_1.push_back(allrm);
                                break;
                            case 2 :
                                array_2.push_back(allrm);
                                break;
                            case 3 :
                                array_3.push_back(allrm);
                                break;
                            case 4 :
                                array_4.push_back(allrm);
                                break;
                            }
                            allrm = nullptr;
                        }

                        allrm = new QGraphicsItemGroup;
                        allrm->setHandlesChildEvents(false);
                        auto rem = new customItem;
                        type = rLevel;
                        switch (rLevel) {
                        case 1 :
                            rem->setDefaultTextColor(Qt::darkGreen);
                            rem->setFont(QFont(fontName, 10));
                            rem->setPlainText(QString::fromStdString(rmStr.substr(_cache._lenRem, itN - _cache._lenRem)));
                            break;
                        case 2 :
                            rem->setDefaultTextColor(Qt::blue);
                            rem->setFont(QFont(fontName, 10, -1, true));
                            rem->setPlainText(QString::fromStdString(rmStr.substr(_cache._lenRem, itN - _cache._lenRem)));
                            break;
                        case 3 :
                            rem->setDefaultTextColor(Qt::red);
                            rem->setFont(QFont(fontName, 10, QFont::Bold));
                            rem->setPlainText(QString::fromStdString(rmStr.substr(_cache._lenRem, itN - _cache._lenRem)).toUpper());
                            break;
                        case 4 :
                            rem->setDefaultTextColor(Qt::darkRed);
                            rem->setFont(QFont(fontName, 10, QFont::Bold));
                            rem->setPlainText(QString::fromStdString(rmStr.substr(_cache._lenRem, itN - _cache._lenRem)).toUpper());
                            break;
                        default:
                            delete rem;
                            rem = nullptr;
                            continue;
                        }

                        rem->setData(0, 3);
                        rem->setData(1, remed);
                        allrm->addToGroup(rem);
                        authorsSym(autr, author);
                    }
                    else{
                        if(allrm != nullptr){
                            authorsSym(autr, author, true);
                        }
                    }
                    prevRem = remed;
                }
            }

            if(allrm != nullptr){
                switch (type) {
                case 1 :
                    array_1.push_back(allrm);
                    break;
                case 2 :
                    array_2.push_back(allrm);
                    break;
                case 3 :
                    array_3.push_back(allrm);
                    break;
                case 4 :
                    array_4.push_back(allrm);
                    break;
                }
            }

            for(const auto & it : array_4)
                horizon(it);
            for(const auto & it : array_3)
                horizon(it);
            for(const auto & it : array_2)
                horizon(it);
            for(const auto & it : array_1)
                horizon(it);

            _rem1 = array_1.size();
            _rem2 = array_2.size();
            _rem3 = array_3.size();
            _rem4 = array_4.size();
        }
    }
    {
        //Chapters shower
        std::string text;
        QString endlab;
        QStringList orig, loz;
        std::string ind(6, '\0');
        bool fis = true;

        while(true){
            quint8 caption = 0;

            if(fis)
                text = _sym.at(_pos);
            else
                text = _sym.at(ind);

            auto first = std::find(text.cbegin() + _sym.serviceDataLenght(), text.cend(), '\0');
            auto localize = std::string(first + 1, std::find(first + 1, text.cend(), '\0'));
            orig.push_back(codec->toUnicode(std::string(text.cbegin() + _sym.serviceDataLenght(), first).c_str()));

            if(!localize.empty())
                loz.push_back(codec->toUnicode(localize.c_str()));

            caption = text.at(21);

            for(auto it = 0; it != 6; ++it){
                ind[0 + it] = text.at(17 - it);
            }

            if(caption <= 1 || ind == "\0\0\0\0\0\0")
                break;

            fis = false;
        }

        if(!orig.isEmpty()){
            QString org, lz;

            for(auto it = orig.size() - 1; it != -1; --it){
                if(it == orig.size() - 1){
                    org += orig[it];
                }
                else{
                    org += " - " + orig[it];
                }
            }
            if(!loz.isEmpty()){
                for(auto it = loz.size() - 1; it != -1; --it){
                    if(it == loz.size() - 1){
                        lz += loz[it];
                    }
                    else{
                        lz += " - " + loz[it];
                    }
                }
            }

            endlab = org + ((loz.isEmpty()) ? "" : '\n' + lz);
        }

        ui->label->setText(endlab);
    }
}
