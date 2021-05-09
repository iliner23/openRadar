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
    _label->close();
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

    _viewLeft->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
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
    const QFont italicFont(fontName, 10, -1, true);
    const QFont commonFont(fontName, 10, QFont::DemiBold);
    const QFont defaultFont(fontName, 10, QFont::Light);
    const QFont labelChapterFont(fontName, 16, QFont::DemiBold);
    const QFont smallFont(fontName, 8);
    const QFont boldFont(fontName, 10, QFont::Bold);
    const auto heightView = _viewLeft->height() * 2;
    const auto widthView = _viewLeft->width();
    constexpr auto attachRatio = 3;
    constexpr auto spaceHeight = 5;

    _scene->clear();
    _scene->setSceneRect(0, 0, widthView, heightView);

    QByteArray fullStr;

    if(!_index.isEmpty())
        fullStr = QByteArray::fromStdString(_symptom.at(_index.toStdString()));
    else
        fullStr = QByteArray::fromStdString(_symptom.front());

    QRectF size;
    QPointF pos;
    QString labelText[2];//first - native, second - localize

    auto returnSize = [](const auto & value, const auto & size){
        return (value == -1) ? size : value;
    };

    while(pos.y() + size.height() < heightView){
        auto update = [&](auto * temp){
            temp->setY(pos.y() + size.height() + 1);
            _scene->addItem(temp);
            size = temp->boundingRect();
            pos = temp->pos();
        };

        if(pos.y() != 0)
            pos.setY(pos.y() + spaceHeight);

        QVector<QGraphicsSimpleTextItem*> linksSynomys;
        linksSynomys.reserve(100);

        quint8 attach = 0;
        quint16 maxDrug = 0, filter = 0;
        int labelWidth = 0;

        attach = qFromLittleEndian<quint8>(fullStr.constData() + 21);
        maxDrug = qFromLittleEndian<quint16>(fullStr.constData() + 22);
        filter = qFromLittleEndian<quint16>(fullStr.constData() + 26);

        if(attach == 0)
            continue;

        const uint8_t lessAttach = (attach > 4) ? 1 : attach;

        bool hideLabel = !(_remFilter == (quint16)-1 || (filter & _remFilter) != 0);
        bool localize = false;
        int labelsEnd = 0;

        {//label subfunction
            const auto firstZero = returnSize(fullStr.indexOf('\0', _symptom.serviceDataLenght()), fullStr.size());
            const auto secondZero = returnSize(fullStr.indexOf('\0', firstZero + 1), fullStr.size());
            labelsEnd = secondZero;

            labelText[0] = _codec->toUnicode(
                        fullStr.mid(_symptom.serviceDataLenght(), firstZero - _symptom.serviceDataLenght()));
            labelText[1] = _codec->toUnicode
                        (fullStr.mid(firstZero + 1, secondZero - firstZero - 1));

            localize = !labelText[1].isEmpty();
            const QString filler[] = {"", "", "-", ". ", "", "", " ", "  "};
            const auto isLocalize = ((localize) ? 2 : 1);

            for(auto it = 0; it < isLocalize; ++it){
                auto textItem = new QGraphicsSimpleTextItem;
                textItem->setFont(commonFont);

                if(attach == 1){
                    if(it == 0)
                        textItem->setBrush(QBrush(Qt::red));

                    textItem->setFont(labelChapterFont);
                }

                if(it == 1)
                    textItem->setBrush(QBrush(Qt::blue));

                if(hideLabel)
                    textItem->setBrush(QBrush(Qt::gray));

                const auto fillStr = QString(attach * attachRatio, ' ')
                        + filler[lessAttach - 1 + ((it == 0) ? 0 : 4)];

                textItem->setText(fillStr + labelText[it] +
                                  ((maxDrug != 0 && it == isLocalize - 1) ? ": " : ""));

                const QFontMetrics metric(textItem->font());
                labelWidth = metric.horizontalAdvance(fillStr);

                textItem->setData(0, 0);
                textItem->setData(1, QByteArray::fromStdString(_symptom.key()));
                update(textItem);
            }
        }
        if(!hideLabel){
            {//synonyms and links
                quint8 type = 4;

                for(auto i = 0; i != 4; ++i){
                    quint8 cnt = 0;

                    for(auto it = 0; it != type + 1; ++it){
                        if(labelsEnd == fullStr.size() || fullStr.at(labelsEnd) != '\0')
                            break;

                        ++cnt;
                        ++labelsEnd;
                    }

                    if(cnt > type)
                        break;

                    type = type - cnt;

                    const auto tmpLinkIter = returnSize(fullStr.indexOf('\0', labelsEnd), fullStr.size());
                    auto synLinkText =
                            _codec->toUnicode(fullStr.mid(labelsEnd, tmpLinkIter - labelsEnd));
                    synLinkText.prepend(' ');
                    labelsEnd = tmpLinkIter;

                    if(synLinkText.size() > 2 && synLinkText.left(3) == " (="){
                        const QString constSynomy[] =
                                { "Synomy : ", "Synomy : ", " Synomy : ", " Synomy : "};
                        auto textItem = new QGraphicsSimpleTextItem;

                        if(type == 2)
                            textItem->setBrush(QBrush(Qt::blue));
                        else if(type == 3){
                            if(localize)
                                textItem->setBrush(QBrush(Qt::red));
                            else
                                textItem->setBrush(QBrush(Qt::blue));
                        }
                        else
                            continue;

                        if(attach == 1)
                            textItem->setFont(labelChapterFont);
                        else
                            textItem->setFont(commonFont);

                        textItem->setText(QString(attach * attachRatio, ' ')
                                          + constSynomy[lessAttach - 1] + synLinkText.mid(1));
                        linksSynomys.push_back(textItem);
                    }
                    else{
                        const QString constLinks[] =
                                        {u8" \u2b08", u8" \u2b08", u8"  \u2b08", u8"   \u2b08"};
                        const QString constSee[] = {"(see ", "(see ", " (see ", "  (see "};
                        const QColor constColor[] = {Qt::darkRed, Qt::darkBlue, Qt::red, Qt::blue};

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

                            auto textItem = new QGraphicsSimpleTextItem;
                            QString tempLinkText = linkText.mid(1);

                            //TODO 462 line
                            if(type == 1 || type == 0){
                                textItem->setFont(defaultFont);
                                textItem->setData(0, 2);
                                tempLinkText.prepend("  " + constLinks[lessAttach - 1]);
                            }
                            else{
                                textItem->setFont(commonFont);
                                textItem->setData(0, 1);
                                tempLinkText.prepend(constSee[lessAttach - 1]);
                                tempLinkText.append(')');
                            }

                            textItem->setText(QString(attach * attachRatio, ' ') + tempLinkText);
                            textItem->setBrush(QBrush(constColor[type +
                                               ((!localize && (type + 1) % 2 != 0) ? 1 : 0)]));

                            textItem->setData(1, linkText);
                            textItem->setData(2, QByteArray::fromStdString(_symptom.key()));
                            textItem->setData(3, !localize);
                            linksSynomys.push_back(textItem);
                        }
                    }

                    if(type == 0)
                        break;
                }
            }
            {//remeds
                 auto horizon = [&](auto * temp){
                    if(temp == nullptr)
                        return;

                    const auto bon = temp->boundingRect();
                    if(pos.x() + size.width() + bon.width() + 3 >= widthView){
                        temp->setX(labelWidth + 3);
                        temp->setY(pos.y() + size.height() + spaceHeight);
                    }
                    else{
                        temp->setX(pos.x() + size.width() + 3);
                        temp->setY(pos.y());
                    }

                    _scene->addItem(temp);
                    size = bon;
                    pos = temp->pos();
                };

                auto authorsSym = [&](const auto & autr, const auto author, auto & allrm, const bool next = false){
                    auto aut = new QGraphicsSimpleTextItem;
                    if(autr == "kl2")
                        aut->setText("*");
                    else if(autr == "zzz")
                        aut->setText(u8"\u2193");
                    else{
                        if(!next)
                            aut->setText(autr);
                        else
                            aut->setText(", " + autr);
                    }

                    aut->setBrush(Qt::magenta);
                    aut->setFont(smallFont);
                    aut->setData(0, 4);
                    aut->setData(1, author);

                    aut->setX(allrm->boundingRect().width() + 3);
                    allrm->addToGroup(aut);
                };

                quint16 prevRemed = 0;
                quint64 remedSize = 0;

                QVector<QGraphicsItemGroup*> remeds;
                remeds.reserve(maxDrug);

                while(labelsEnd < fullStr.size()){
                    quint16 remed = 0, author = 0, tLevel = 0;
                    uint8_t rLevel = 0;

                    if(labelsEnd + 7 >= fullStr.size())
                        break;

                    const auto startLabelsEnd = labelsEnd;

                    remed = qFromLittleEndian<quint16>(fullStr.constData() + labelsEnd);
                    labelsEnd += 2;
                    rLevel = qFromLittleEndian<quint16>(fullStr.constData() + labelsEnd);
                    ++labelsEnd;
                    author = qFromLittleEndian<quint16>(fullStr.constData() + labelsEnd);
                    labelsEnd += 2;
                    tLevel = qFromLittleEndian<quint16>(fullStr.constData() + labelsEnd);
                    labelsEnd += 2;

                    if(((char *)&remed)[0] == '\x5a' && ((char *)&remed)[1] == '\x5a' && rLevel == '\x5a')
                        break;

                    if(_remFilter != (quint16)-1 && (tLevel & _remFilter) == 0)
                        continue;

                    if((((char *)&author)[1] & (char)128) != 0)
                        ((char *)&author)[1] ^= (char)128;//remed have * in the end

                    const auto & rmStr = _cache->_cacheRemed.at(remed);
                    const auto & auStr = _cache->_cacheAuthor.at(author);
                    auto itN = rmStr.find('\0', _cache->_lenRem);
                    auto itA = auStr.find('\0', _cache->_lenAuthor);
                    auto authorName = QString::fromStdString(auStr.substr(_cache->_lenAuthor, itA - _cache->_lenAuthor));
                    auto remedName = QString::fromStdString(rmStr.substr(_cache->_lenRem, itN - _cache->_lenRem));

                    if(pos.y() >= heightView)
                        break;

                    if(rLevel == 0 || rLevel > 4)
                        continue;

                    if(prevRemed == remed)
                        authorsSym(authorName, author, remeds.back(), true);
                    else{
                        auto remedItem = new QGraphicsSimpleTextItem;
                        const QFont * remedFonts[] = {&defaultFont, &italicFont, &boldFont};
                        const QColor remedColors[] = {Qt::darkGreen, Qt::blue, Qt::red, Qt::darkRed};

                        remedItem->setFont(*remedFonts[((rLevel == 4) ? 2 : rLevel - 1)]);
                        remedItem->setBrush(remedColors[rLevel - 1]);
                        remedItem->setText((rLevel > 2) ? remedName.toUpper() : remedName);

                        remedItem->setData(0, 3);
                        remedItem->setData(1, startLabelsEnd);
                        remedItem->setData(2, QByteArray::fromStdString(_symptom.key()));

                        auto group = new QGraphicsItemGroup;
                        group->addToGroup(remedItem);
                        group->setHandlesChildEvents(false);
                        group->setFlag(QGraphicsItem::ItemHasNoContents);
                        remeds.push_back(group);

                        authorsSym(authorName, author, remeds.back());
                        ++remedSize;
                    }

                    prevRemed = remed;
                }

                if(remedSize != 0){
                    auto siz = new QGraphicsSimpleTextItem;
                    siz->setFont(QFont(fontName, 10));
                    siz->setText("(" + QString::number(remedSize) + ") ");
                    horizon(siz);
                }

                for(const auto & it : remeds)
                    horizon(it);

                if(!remeds.isEmpty())
                    pos.setY(pos.y() + spaceHeight);
            }

            for(const auto & it : linksSynomys)
                update(it);
        }

        if(_symptom.key() == _endIndex.toStdString())
            break;

        fullStr = QByteArray::fromStdString(_symptom.next());
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

        _label->show();
        _label->setText(org + ((loz.isEmpty()) ? "" : '\n' + lz));
    }
    else{
        _label->clear();
        _label->close();
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
    _symptom.at(pos.toStdString(), false);
    const auto tp = _symptom.currentPosition();
    _bar->setValue(tp);
}
