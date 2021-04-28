#include "repertory.h"

QDir repertory::getRepDir() const noexcept{
    return _filename;
}
void repertory::changeFilter(QAction * action){
    if(action != nullptr){
        auto menu = qobject_cast<QMenu*>(action->parent());

        if(menu == nullptr)
            return;

        auto array = menu->actions();
        _remFilter = (quint16) action->data().toInt();
        menu->setTitle(action->text());

        for(auto & it : array){
            if(it != action){
                it->setChecked(false);
            }
        }

        if(!action->isChecked())
            action->setChecked(true);
        else
            renderingView();
    }
}
repertory::repertory(const QDir & filename, const QDir & system, const cache & ch, const quint16 remFilter, QWidget *parent) : QWidget(parent)
{
    _filename = filename;
    _system = system;
    _codec = QTextCodec::codecForName("system");
    _symptom.open(filename.filePath("symptom").toStdString());
    _cache = &ch;
    _remFilter = remFilter;

    auto vlayout = new QVBoxLayout(this);
    auto hlayout = new QHBoxLayout(this);
    _scene = new customScene(this);

    _viewLeft = new QGraphicsView(_scene, this);
    _viewRight = new QGraphicsView(_scene, this);

    _label = new QLabel(this);
    _label->hide();
    _label->setFont(QFont("default", 10));

    openCtree view(filename.filePath("view").toStdString());
    auto barm = new QMenuBar(this);
    _menu = new QMenu("Full remed", barm);
    barm->addMenu(_menu);

    auto frm = _menu->addAction("Full remed");
    frm->setCheckable(true);
    frm->setData(-1);

    if(_remFilter == (quint16)-1){
        changeFilter(frm);
    }

    auto mask = 1;

    for(auto it = 0; it != view.size(); ++it){
        auto str = view.next();
        auto ps = _menu->addAction(_codec->toUnicode(str.substr(view.serviceDataLenght(), str.find('0', view.serviceDataLenght())).c_str()));
        ps->setData(mask);
        ps->setCheckable(true);

        if(mask == _remFilter){
            changeFilter(ps);
        }

        mask <<= 1;
    }
    connect(_menu, &QMenu::triggered, this, &repertory::changeFilter);

    _bar = new QScrollBar(Qt::Vertical, this);
    _bar->setMinimum(0);
    _bar->setMaximum(_symptom.size() - 1);

    hlayout->addWidget(_viewLeft);
    hlayout->addWidget(_viewRight);
    hlayout->addWidget(_bar);

    vlayout->addWidget(barm);
    vlayout->addWidget(_label);
    vlayout->addLayout(hlayout);
    setLayout(vlayout);

    _viewLeft->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    _viewRight->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    _viewLeft->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    _viewLeft->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    _viewRight->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    _viewRight->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    _viewLeft->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing);
    _viewLeft->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    _viewRight->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing);
    _viewRight->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    _scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    _bar->setTracking(false);
    connect(_bar, &QScrollBar::valueChanged, this, &repertory::changedPos);
    connect(_scene, &customScene::labelActivated, this, &repertory::clickedAction);
    _symptom.back(false);
    _endIndex = QByteArray::fromStdString(_symptom.key());
    _symptom.front(false);
}

void repertory::renderingView(){
    const QString fontName("cursive");
    customItem * lastItem = nullptr;
    _scene->clear();
    _scene->setSceneRect(0, 0, _viewLeft->width(), _viewLeft->height() * 2);

    QRectF size;
    QPointF pos;
    bool localize = false;
    std::string full;

    if(_index.isEmpty()){
        full = _symptom.front();
        _index = QByteArray::fromStdString(_symptom.key());
    }
    else
        full = _symptom.at(_index.toStdString());

    QByteArray str;
    str = _index;

    while(true){
        if(pos.y() + size.height() >= _viewLeft->height() * 2){
            break;
        }

        std::vector<customItem*> links;
        links.reserve(30);
        std::string label1, label2;

        qint8 symbol;
        symbol = full.at(21);
        quint16 maxDrug = 0, filter = 0;
        ((char *)&maxDrug)[0] = full.at(22);
        ((char *)&maxDrug)[1] = full.at(23);
        ((char *)&filter)[0] = full.at(26);
        ((char *)&filter)[1] = full.at(27);

        auto update = [&](auto * temp){
            temp->setY(pos.y() + size.height() + 5);
            _scene->addItem(temp);
            size = temp->boundingRect();
            pos = temp->pos();
        };

        bool hideLabel = !(_remFilter == (quint16)-1 || (filter & _remFilter) != 0);

        auto firstIt = std::find(full.cbegin() + _symptom.serviceDataLenght(), full.cend(), '\0');
        auto secondIt = std::find(firstIt + 1, full.cend(), '\0');
        {//Label subfunction
            label2 = std::string(firstIt + 1, secondIt);
            label1 = std::string(full.cbegin() + _symptom.serviceDataLenght(), firstIt);

            for(auto i = 0; i != 2; ++i)
            {
                const std::string * lab = nullptr;
                if(i == 0)
                    lab = &label1;
                else{
                    if(label2.empty())
                        break;

                    lab = &label2;
                    localize = true;
                }

                auto temp = new customItem;

                switch (symbol) {
                case 1 :
                    temp->setFont(QFont("fontName", 16, QFont::DemiBold));
                    if(i == 0)
                        temp->setDefaultTextColor(Qt::red);
                    else
                        temp->setDefaultTextColor(Qt::blue);

                    temp->setX(10);
                    temp->setPlainText(_codec->toUnicode(lab->c_str()));
                    break;
                case 2 :
                    temp->setPlainText(_codec->toUnicode(lab->c_str()));
                    temp->setX(20);
                    if(i == 1)
                        temp->setDefaultTextColor(Qt::blue);

                    temp->setFont(QFont("fontName", 10, QFont::DemiBold));
                    break;
                case 3 :
                    temp->setX(30);
                    if(i == 0)
                        temp->setPlainText('-' + _codec->toUnicode(lab->c_str()));
                    else{
                        temp->setPlainText(' ' + _codec->toUnicode(lab->c_str()));
                        temp->setDefaultTextColor(Qt::blue);
                    }

                    temp->setFont(QFont("fontName", 10, QFont::DemiBold));
                    break;
                case 4 :
                    temp->setX(40);
                    if(i == 0)
                        temp->setPlainText(". " + _codec->toUnicode(lab->c_str()));
                    else{
                        temp->setPlainText("  " + _codec->toUnicode(lab->c_str()));
                        temp->setDefaultTextColor(Qt::blue);
                    }

                    temp->setFont(QFont("fontName", 10, QFont::DemiBold));
                    break;
                default:
                    temp->setX(50 + (symbol - 4) * 5);
                    temp->setPlainText(_codec->toUnicode(lab->c_str()));
                    if(i == 1)
                        temp->setDefaultTextColor(Qt::blue);

                    temp->setFont(QFont(fontName, 10, QFont::DemiBold));
                }

                if(hideLabel)
                    temp->setDefaultTextColor(Qt::gray);

                lastItem = temp;
                temp->setData(1, str);
                temp->setData(0, 0);
                update(temp);
            }
        }
        if(!hideLabel){
            {//synonyms and links
                uint8_t max = 4;
                for(uint8_t i = 0; i != 4; ++i){
                    uint8_t cnt = 0;

                    /*for(auto it = 0; it != 5; ++it){*/
                    for(auto it = 0; it != max + 1; ++it){
                        if(secondIt != full.cend() && *secondIt == '\0'){
                            ++cnt;
                            ++secondIt;
                        }
                        else
                            break;
                    }

                    if(cnt > max)
                        break;

                    auto iter = std::find(secondIt, full.cend(), '\0');
                    auto strg = std::string(secondIt, iter);
                    strg.insert(strg.begin(), ' ');
                    secondIt = iter;

                    if(strg.size() > 2 && strg.substr(0, 3) == " (="){
                        auto temp = new customItem;

                        if(max - cnt == 2)
                            temp->setDefaultTextColor(Qt::blue);
                        else if(max - cnt == 3){
                            if(localize)
                                temp->setDefaultTextColor(Qt::red);
                            else
                                temp->setDefaultTextColor(Qt::blue);
                        }
                        else
                            continue;

                        switch (symbol) {
                        case 1 :
                            temp->setFont(QFont(fontName, 16, QFont::DemiBold));
                            temp->setX(10);
                            temp->setPlainText("Synomy : " + _codec->toUnicode(std::string(strg.begin() + 1, strg.end()).c_str()));
                            break;
                        case 2 :
                            temp->setX(20);
                            temp->setFont(QFont(fontName, 10, QFont::DemiBold));
                            temp->setPlainText("Synomy : " + _codec->toUnicode(std::string(strg.begin() + 1, strg.end()).c_str()));
                            break;
                        case 3 :
                            temp->setX(30);
                            temp->setFont(QFont(fontName, 10, QFont::DemiBold));
                            temp->setPlainText(" Synomy : " + _codec->toUnicode(std::string(strg.begin() + 1, strg.end()).c_str()));
                            break;
                        case 4 :
                            temp->setX(40);
                            temp->setFont(QFont(fontName, 10, QFont::DemiBold));
                            temp->setPlainText(" Synomy : " + _codec->toUnicode(std::string(strg.begin() + 1, strg.end()).c_str()));
                            break;
                        default :
                            temp->setX(50 + (symbol - 4) * 5);
                            temp->setFont(QFont(fontName, 10, QFont::DemiBold));
                            temp->setPlainText("Synomy : " + _codec->toUnicode(std::string(strg.begin() + 1, strg.end()).c_str()));
                            break;
                        }

                        lastItem = temp;
                        update(temp);
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

                            auto temp = new customItem;

                            switch (symbol) {
                            case 1 :
                                if(max - cnt == 1 || max - cnt == 0){
                                    temp->setX(10);
                                    temp->setPlainText(u8" \u2b08" + _codec->toUnicode(tpstr.c_str() + 1));
                                    temp->setFont(QFont(fontName, 8));
                                }
                                else{
                                    temp->setX(10);
                                    temp->setPlainText("(see " + _codec->toUnicode(tpstr.c_str() + 1) + ')');
                                    temp->setFont(QFont(fontName, 16, QFont::DemiBold));
                                }
                                break;
                            case 2 :
                                if(max - cnt == 1 || max - cnt == 0){
                                    temp->setX(20);
                                    temp->setPlainText(u8" \u2b08" + _codec->toUnicode(tpstr.c_str() + 1));
                                    temp->setFont(QFont(fontName, 8));
                                }
                                else{
                                    temp->setX(20);
                                    temp->setPlainText("(see " + _codec->toUnicode(tpstr.c_str() + 1) + ')');
                                    temp->setFont(QFont(fontName, 10, QFont::DemiBold));
                                }
                                break;
                            case 3 :
                                if(max - cnt == 1 || max - cnt == 0){
                                    temp->setX(30);
                                    temp->setPlainText(u8"  \u2b08" + _codec->toUnicode(tpstr.c_str() + 1));
                                    temp->setFont(QFont(fontName, 8));
                                }
                                else{
                                    temp->setX(30);
                                    temp->setPlainText(" (see " + _codec->toUnicode(tpstr.c_str() + 1) + ')');
                                    temp->setFont(QFont(fontName, 10, QFont::DemiBold));
                                }
                                break;
                            case 4 :
                                if(max - cnt == 1 || max - cnt == 0){
                                    temp->setX(40);
                                    temp->setPlainText(u8"   \u2b08" + _codec->toUnicode(tpstr.c_str() + 1));
                                    temp->setFont(QFont(fontName, 8));
                                }
                                else{
                                    temp->setX(40);
                                    temp->setPlainText("  (see " + _codec->toUnicode(tpstr.c_str() + 1) + ')');
                                    temp->setFont(QFont(fontName, 10, QFont::DemiBold));
                                }
                                break;
                            default :
                                if(max - cnt == 1 || max - cnt == 0){
                                    temp->setX(50 + (symbol - 3) * 5);
                                    temp->setPlainText(u8" \u2b08" + _codec->toUnicode(tpstr.c_str() + 1));
                                    temp->setFont(QFont(fontName, 8));
                                }
                                else{
                                    temp->setX(50 + (symbol - 3) * 5);
                                    temp->setPlainText("(see " + _codec->toUnicode(tpstr.c_str() + 1) + ')');
                                    temp->setFont(QFont(fontName, 10, QFont::DemiBold));
                                }
                            }

                            temp->setData(1, QString::fromStdString(tpstr));
                            temp->setData(2, str);

                            switch (max - cnt) {
                            case 3:
nolocs:                         temp->setDefaultTextColor(Qt::blue);
                                temp->setData(0, 1);
                                temp->setData(3, true);
                                lastItem = temp;
                                update(temp);
                                break;
                            case 2:
                                if(localize){
                                    temp->setDefaultTextColor(Qt::red);
                                    temp->setData(3, false);
                                }
                                else
                                    goto nolocs;

                                temp->setData(0, 1);
                                lastItem = temp;
                                update(temp);
                                break;
                            case 1:
nolocl:                         temp->setDefaultTextColor(Qt::darkBlue);
                                temp->setData(3, true);
                                temp->setData(0, 2);
                                links.push_back(temp);
                                break;
                            case 0:
                                if(localize){
                                    temp->setDefaultTextColor(Qt::darkRed);
                                    temp->setData(3, false);
                                }
                                else
                                    goto nolocl;

                                temp->setData(0, 2);
                                links.push_back(temp);
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
                    if(pos.x() + size.width() + bon.width() + 5 >= _viewLeft->width()){
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

                if(maxDrug != 0 && lastItem != nullptr){
                    auto temp = new customItem;
                    temp->setPlainText(": ");
                    temp->setFont(lastItem->getFont());
                    temp->setDefaultTextColor(lastItem->getDefaultTextColor());
                    horizon(temp);
                }

                customItem * siz = nullptr;
                bool exit = false;
                std::vector<QGraphicsItemGroup*> array;
                array.reserve(maxDrug);

                QGraphicsItemGroup * allrm = nullptr;

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

                for(auto it = std::find_if(secondIt, full.cend(), pred); it != full.cend(); ++secondIt){
                    quint16 remed = 0, author = 0, tLevel = 0;
                    uint8_t rLevel;
                    const auto szr = secondIt - full.cbegin();

                    if(full.cend() - secondIt <= 3)
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

                        const auto & rmStr = _cache->_cacheRemed.at(remed);
                        const auto & auStr = _cache->_cacheAuthor.at(author);
                        auto itN = rmStr.find('\0', _cache->_lenRem);
                        auto itA = auStr.find('\0', _cache->_lenAuthor);
                        auto autr = auStr.substr(_cache->_lenAuthor, itA - _cache->_lenAuthor);

                        if(prevRem != remed){
                            if(allrm != nullptr){
                                array.push_back(allrm);
                                allrm = nullptr;
                            }

                            if(pos.y() >= _viewLeft->height() * 2){
                                exit = true;
                            }

                            if(!exit){
                                allrm = new QGraphicsItemGroup;
                                allrm->setHandlesChildEvents(false);
                                auto rem = new customItem;
                                switch (rLevel) {
                                case 1 :
                                    rem->setDefaultTextColor(Qt::darkGreen);
                                    rem->setFont(QFont(fontName, 10));
                                    rem->setPlainText(QString::fromStdString(rmStr.substr(_cache->_lenRem, itN - _cache->_lenRem)));
                                    break;
                                case 2 :
                                    rem->setDefaultTextColor(Qt::blue);
                                    rem->setFont(QFont(fontName, 10, -1, true));
                                    rem->setPlainText(QString::fromStdString(rmStr.substr(_cache->_lenRem, itN - _cache->_lenRem)));
                                    break;
                                case 3 :
                                    rem->setDefaultTextColor(Qt::red);
                                    rem->setFont(QFont(fontName, 10, QFont::Bold));
                                    rem->setPlainText(QString::fromStdString(rmStr.substr(_cache->_lenRem, itN - _cache->_lenRem)).toUpper());
                                    break;
                                case 4 :
                                    rem->setDefaultTextColor(Qt::darkRed);
                                    rem->setFont(QFont(fontName, 10, QFont::Bold));
                                    rem->setPlainText(QString::fromStdString(rmStr.substr(_cache->_lenRem, itN - _cache->_lenRem)).toUpper());
                                    break;
                                default:
                                    delete rem;
                                    rem = nullptr;
                                    continue;
                                }

                                rem->setData(0, 3);
                                rem->setData(1, szr);
                                rem->setData(2, str);
                                allrm->addToGroup(rem);
                                authorsSym(autr, author);
                            }
                        }
                        else{
                            if(allrm != nullptr && exit == false){
                                authorsSym(autr, author, true);
                            }
                        }
                        prevRem = remed;
                    }
                }

                if(allrm != nullptr){
                    array.push_back(allrm);
                }

                const auto arsize = array.size();
                if(arsize != 0){
                    siz = new customItem;
                    siz->setFont(QFont(fontName, 10));
                    siz->setPlainText("(" + QString::number(arsize) + ") ");
                    horizon(siz);
                }

                for(const auto & it : array)
                    horizon(it);
            }
        }

        for(const auto & it : links)
            update(it);

        if(str == _endIndex)
            break;

        full = _symptom.next();
        str = QByteArray::fromStdString(_symptom.key());
    }
    _viewLeft->setSceneRect(0, 0, 1, 1);
    _viewRight->setSceneRect(0, _viewLeft->height(), 1, 1);
    _viewLeft->viewport()->update();
    _viewRight->viewport()->update();
}
void repertory::resizeEvent(QResizeEvent * event){
    event->ignore();
    renderingView();
}
void repertory::renderingLabel(std::string text){
    QStringList orig, loz;
    std::string ind(6, '\0');
    bool fis = true;

    while(true){
        quint8 caption = 0;
        if(!fis){
            text = _symptom.at(ind);
            auto first = std::find(text.cbegin() + _symptom.serviceDataLenght(), text.cend(), '\0');
            auto localize = std::string(first + 1, std::find(first + 1, text.cend(), '\0'));
            orig.push_back(_codec->toUnicode(std::string(text.cbegin() + _symptom.serviceDataLenght(), first).c_str()));

            if(!localize.empty())
                loz.push_back(_codec->toUnicode(localize.c_str()));
        }
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

        _label->setText(org + ((loz.isEmpty()) ? "" : '\n' + lz));
        _label->show();
    }
    else{
        _label->hide();
    }
}
void repertory::changedPos(const int pos){
    const auto str = _symptom.at(pos);
    _index = QByteArray::fromStdString(_symptom.key());
    renderingLabel(str);
    renderingView();
}
void repertory::clickedAction(const customItem * item){
    //0 - label num, 1 - (see, 2 - links, 3 - remed, 4 - author

    if(!item->data(0).isValid())
        return;

    switch (item->data(0).toInt()) {
        case 0 :{
            auto * label = new Label(*_cache, _filename,
                                     _system , item->data(1).toByteArray(), _remFilter, this);
            label->setAttribute(Qt::WA_DeleteOnClose);
            label->show();
            break;
        }
        case 1 :
            break;
        case 2 :{
            /*auto link = item->data(1).toString().toStdString();
            quint32 pos = item->data(2).toUInt();
            quint8 caption = 0;
            bool lang = item->data(3).toBool();//true - orig, false - local
            auto beg = 0;
            auto fnd = 0;
            bool fir = true;
            std::vector<std::string> val;

            while(true){
                fnd = link.find('-', fnd);

                if(fir){
                    fir = false;
                    caption = link.at(0);
                    ++beg;
                }

                if(fnd == -1){
                    val.push_back(link.substr(beg, fnd - beg));
                    break;
                }
                else
                    val.push_back(link.substr(beg, fnd - beg - 1));

                ++fnd;
                beg = ++fnd;
            }

            if(lang){
                std::string key(6, '\0');
                auto full = _symptom.getData(pos);

                while(true){
                    quint8 loccap = std::to_string(full.at(21))[0];

                    for(auto it = 0; it != 6; ++it){
                        key[0 + it] = full.at(17 - it);
                    }

                    if(loccap > caption || key == "\0\0\0\0\0\0"){
                        _symptom.getData(key);
                    }
                    else if(loccap == caption){
                        _symptom.setMember(1);
                        const auto tmp = QString::fromStdString(val.at(0)).toUpper().toStdString();
                        key += _symptom.convertString(tmp);

                        if(key.size() < 26)
                            key.append(26 - key.size(), _symptom.getPaddingChar());

                        qDebug() << QString::fromStdString(_symptom.getData(key));

                        _symptom.setMember(0);
                        break;
                    }
                    else
                        break;
                }
            }*/
            break;
        }
        case 3 : {
            auto remed = new remed_author(_filename, _system, *_cache, item->data(2).toByteArray()
                                          , _remFilter, item->data(1).toUInt(), this);
            remed->setAttribute(Qt::WA_DeleteOnClose);
            remed->show();
            break;
        }
        case 4 : {
            auto author = new  class author(_system, item->data(1).toUInt(), *_cache, this);
            author->setAttribute(Qt::WA_DeleteOnClose);
            author->show();
            break;
        }
    }
}
void repertory::setPosition(const QByteArray & pos){
    _index = pos;//TODO : optimize
    quint32 ipos = 0;
    bool first = true;

    while(true){
        if(first){
            _symptom.front(false);
            first = false;
        }
        else
            _symptom.next(false);

        const auto compare = QByteArray::fromStdString(_symptom.key());

        if(compare == _index)
            break;

        ++ipos;
    }

    _bar->setValue(ipos);
}
