#include "repertoryengine.h"
repertoryEngine::repertoryEngine(QGraphicsScene *parent): QObject(parent){
    initFonts();
}
repertoryEngine::repertoryEngine(const QDir filename, const std::shared_ptr<cache> & cache
                                 , QGraphicsScene *parent, QTextCodec * codec) : QObject(parent){
    initFonts();
    repertoryEngine::reset(filename, cache, codec);
}
void repertoryEngine::initFonts(){
    _fonts.fontName = "cursive";
    _fonts.italicFont = QFont(_fonts.fontName, 10, -1, true);
    _fonts.commonFont = QFont(_fonts.fontName, 10, QFont::DemiBold);
    _fonts.defaultFont = QFont(_fonts.fontName, 10, QFont::Light);
    _fonts.labelChapterFont = QFont(_fonts.fontName, 16, QFont::DemiBold);
    _fonts.smallFont = QFont(_fonts.fontName, 8);
    _fonts.boldFont = QFont(_fonts.fontName, 10, QFont::Bold);
}
void repertoryEngine::reset(const QDir filename, const std::shared_ptr<cache> &cache, QTextCodec *codec){
    _symptom.open(filename.filePath("symptom").toStdString());
    _symptom.back(false);
    _private.endIndex = QByteArray::fromStdString(_symptom.key());
    _cache = cache;
    _codec = codec;
}
void repertoryEngine::addRemeds(QGraphicsItem * temp){
    if(temp == nullptr)
        return;

    const auto bon = temp->boundingRect();
    if(_private.pos.x() + _private.size.width() + bon.width() + 3 >= _private.widthView){
        temp->setX(_private.labelWidth + 3);
        temp->setY(_private.pos.y() + _private.size.height() + _private.spaceHeight);
    }
    else{
        temp->setX(_private.pos.x() + _private.size.width() + 3);
        temp->setY(_private.pos.y());
    }

    qobject_cast<QGraphicsScene*>(parent())->addItem(temp);
    _private.size = bon;
    _private.pos = temp->pos();
}
void repertoryEngine::addLabel(QGraphicsItem * temp){
    temp->setY(_private.pos.y() + _private.size.height() + 1);
    qobject_cast<QGraphicsScene*>(parent())->addItem(temp);
    _private.size = temp->boundingRect();
    _private.pos = temp->pos();
}
void repertoryEngine::authorsSym(const QString & autr, const quint16 author, QGraphicsItemGroup * allrm, const bool next){
    auto aut = new QGraphicsSimpleTextItem;

    auto replaceText = [](const auto & str) -> QString{
        if(str == "kl2")
            return "*";
        else if(str == "zzz")
            return u8"\u2193";

        return str;
    };

    if(!next)
        aut->setText(replaceText(autr));
    else
        aut->setText(", " + replaceText(autr));

    aut->setBrush(Qt::magenta);
    aut->setFont(_fonts.smallFont);
    aut->setData(0, 4);
    aut->setData(1, author);

    aut->setX(allrm->boundingRect().width() + 3);
    allrm->addToGroup(aut);
}
void repertoryEngine::renderingChapter(const quint64 firstZero, const quint64 secondZero, std::array<QGraphicsSimpleTextItem*, 2> & lab){
    if((_filterType & repertoryFilterType::chapter) == 0)
        return;

    QString labelText[2];//first - native, second - localize
    labelText[0] = _codec->toUnicode(
                _private.fullStr.mid(_symptom.serviceDataLenght(), firstZero - _symptom.serviceDataLenght()));
    labelText[1] = _codec->toUnicode
                (_private.fullStr.mid(firstZero + 1, secondZero - firstZero - 1));

    const QString filler[] = {"", "", "-", ". ", "", "", " ", "  "};
    const auto isLocalize = ((_private.localize) ? 2 : 1);
    const bool star = (_private.fullStr.at(6) != '\0' || _private.fullStr.at(7) != '\0') ? true : false;//values with dup key adding * in the end
    const uint8_t lessAttach = (_private.attach > 4) ? 1 : _private.attach;

    for(auto it = 0; it < isLocalize; ++it){
        auto textItem = new QGraphicsSimpleTextItem;
        textItem->setFont(_fonts.commonFont);

        if(_private.attach == 1){
            if(it == 0)
                textItem->setBrush(QBrush(Qt::red));

            textItem->setFont(_fonts.labelChapterFont);
        }

        if(it == 1)
            textItem->setBrush(QBrush(Qt::blue));

        if(_private.hideLabel)
            textItem->setBrush(QBrush(Qt::gray));

        const auto fillStr = QString(_private.attach * _private.attachRatio, ' ')
                + filler[lessAttach - 1 + ((it == 0) ? 0 : 4)];

        textItem->setText(fillStr + labelText[it] + ((star) ? "*" : "") +
                 ((_private.maxDrug != 0 && it == isLocalize - 1) ? ": " : "  "));

        const QFontMetrics metric(textItem->font());
        _private.labelWidth = metric.horizontalAdvance(fillStr);

        textItem->setData(0, 0);
        textItem->setData(1, QByteArray::fromStdString(_symptom.key()));
        lab[it] = textItem;
    }
}
void repertoryEngine::setCurrentKey(const QByteArray key){
    _symptom.at(key.toStdString(), false);
    _private.index = key;
}
void repertoryEngine::setCurrentPosition(int pos){
    _symptom.at(pos, false);
    _private.index = QByteArray::fromStdString(_symptom.key());
}
void repertoryEngine::render(const int heightView, const int widthView, const bool oneChapter){
    _private.fullStr =
            QByteArray::fromStdString(_symptom.at(_private.index.toStdString()));

    _private.widthView = widthView;

    if(!oneChapter){
        _private.heightView = heightView;
        qobject_cast<QGraphicsScene*>(parent())->
                setSceneRect(0, 0, _private.widthView, _private.heightView);
    }
    else
        _private.heightView = std::numeric_limits<int>::max();

    _private.labelWidth = 0;
    qobject_cast<QGraphicsScene*>(parent())->clear();
    _private.pos = {0, 0};
    _private.size = {0, 0, 0, 0};
    _private.labelsVec.clear();

    if(!oneChapter){
        while(_private.pos.y() + _private.size.height() < _private.heightView)
            if(loopRender()) break;
    }
    else
       loopRender();
}
bool repertoryEngine::loopRender(){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    if(_private.pos.y() != 0 &&
            ((_filterType & (~repertoryFilterType::links)) != 0))
        _private.pos.setY(_private.pos.y() + _private.spaceHeight);

    _private.pos.setX(0);
    _private.size.setWidth(0);

    QVector<QGraphicsItem*> linksSynomys;
    std::array<QGraphicsSimpleTextItem*, 2> labels = {nullptr, nullptr};
    linksSynomys.reserve(100);

    _private.attach = qFromLittleEndian<quint8>(_private.fullStr.constData() + 21);
    _private.maxDrug = qFromLittleEndian<quint16>(_private.fullStr.constData() + 22);
    _private.filter = qFromLittleEndian<quint16>(_private.fullStr.constData() + 26);

    if(_private.attach == 0)
        return false;

    _private.hideLabel = !(_private.remFilter == (quint16)-1 || (_private.filter & _private.remFilter) != 0);
    _private.localize = false;
    _private.labelsEnd = 0;

    {
        const auto firstZero = return_size(
                    _private.fullStr.indexOf('\0', _symptom.serviceDataLenght()), _private.fullStr.size());
        const auto secondZero = return_size(
                    _private.fullStr.indexOf('\0', firstZero + 1), _private.fullStr.size());
        _private.labelsEnd = secondZero;
        _private.localize = (secondZero - firstZero - 1 <= 0) ? false : true;

        renderingChapter(firstZero, secondZero, labels);//label subfunction
        processingChapter(firstZero, secondZero);
    }

    linksRender(linksSynomys, labels);//synonyms and links

    if((_filterType & repertoryFilterType::chapter) != 0){
        addLabel(labels[0]);//add labels after added synomy text

        if(_private.localize)
            addLabel(labels[1]);

        if(_navigation && labels[0]->y() + labels[0]->boundingRect().height() <= _private.heightView){
            auto pos = new QGraphicsSimpleTextItem("->");
            pos->setX(labels[0]->x());
            pos->setY(labels[0]->y());
            pos->hide();
            pos->setData(0, 5);
            pos->setData(1, QByteArray::fromStdString(_symptom.key()));

            qobject_cast<QGraphicsScene*>(parent())->addItem(pos);
            _private.labelsVec.push_back(pos);
        }
    }

    if(!_private.hideLabel){
        quint64 remed_size = 0;
        QVector<QVector<QGraphicsItemGroup*>> remeds(1);
        processingRemeds();
        remedRender(remeds, _sorting, &remed_size);

        if((_filterType & repertoryFilterType::remeds) != 0){//remeds
            if(remed_size != 0 && _counter){
                auto siz = new QGraphicsSimpleTextItem;
                siz->setFont(QFont(_fonts.fontName, 10));
                siz->setText("(" + QString::number(remed_size) + ") ");
                addRemeds(siz);
            }

            sortRemeds(remeds);

            if(!remeds[0].empty())
                _private.pos.setY(_private.pos.y() + _private.spaceHeight);
        }

        for(const auto & it : linksSynomys)
            addLabel(it);
    }

    if(_symptom.key() == _private.endIndex.toStdString())
        return true;

    _private.fullStr = QByteArray::fromStdString(_symptom.next());

    return false;
}
void repertoryEngine::sortRemeds(QVector<QVector<QGraphicsItemGroup*>> & remeds){
    for(auto it = remeds.rbegin(); it != remeds.rend(); ++it){
        for(auto & iter : *it)
            addRemeds(iter);
    }
}
void repertoryEngine::linksItems(const quint8 type, const QString & synLinkText, QVector<QGraphicsItem *> & linksSynomys, std::array<QGraphicsSimpleTextItem*, 2> & chapters){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    const uint8_t lessAttach = (_private.attach > 4) ? 1 : _private.attach;

    if((_filterType & repertoryFilterType::chapter) != 0 &&
            synLinkText.size() > 2 && synLinkText.left(3) == " (="){

        if(type == 2 && _private.localize){
            auto tpText = chapters[1]->text();
            chapters[1]->setText(tpText.insert(tpText.size() - 2, synLinkText));
        }
        else if(type == 3){
            auto tpText = chapters[0]->text();
            chapters[0]->setText(tpText.insert(tpText.size() - 2, synLinkText));
        }
    }
    else if((_filterType & repertoryFilterType::links) != 0 && !_private.hideLabel){
        const QString constLinks[] =
                        {u8" \u2b08", u8" \u2b08", u8"  \u2b08", u8"   \u2b08"};
        const QString constSee[] = {"(see ", "(see ", " (see ", "  (see "};
        const QColor constColor[] = {Qt::darkRed, Qt::darkBlue, Qt::red, Qt::blue};

        for(auto it = 0; it != synLinkText.size(); ){
            const auto iterEnd = return_size(synLinkText.indexOf('/', it), synLinkText.size());
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

            if(type == 1 || type == 0){
                textItem->setFont(_fonts.defaultFont);
                textItem->setData(0, 2);
                tempLinkText.prepend("  " + constLinks[lessAttach - 1]);
            }
            else{
                textItem->setFont(_fonts.commonFont);
                textItem->setData(0, 1);
                tempLinkText.prepend(constSee[lessAttach - 1]);
                tempLinkText.append(')');
            }

            textItem->setText(QString(_private.attach * _private.attachRatio, ' ') + tempLinkText);
            textItem->setBrush(QBrush(constColor[type +
                               ((!_private.localize && (type + 1) % 2 != 0) ? 1 : 0)]));

            textItem->setData(1, linkText);
            textItem->setData(2, QByteArray::fromStdString(_symptom.key()));
            textItem->setData(3, !_private.localize);
            linksSynomys.push_back(textItem);
        }
    }
}
void repertoryEngine::linksRender(QVector<QGraphicsItem *> & linksSynomys, std::array<QGraphicsSimpleTextItem*, 2> & labels){
    //synonyms and links
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    quint8 type = 4;

    for(auto i = 0; i != 4; ++i){
        quint8 cnt = 0;

        for(auto it = 0; it != type + 1; ++it){
            if(_private.labelsEnd == _private.fullStr.size()
                    || _private.fullStr.at(_private.labelsEnd) != '\0')
                break;

            ++cnt;
            ++_private.labelsEnd;
        }

        if(cnt > type)
            break;

        type = type - cnt;

        const auto tmpLinkIter = return_size
                (_private.fullStr.indexOf('\0', _private.labelsEnd), _private.fullStr.size());

        auto synLinkText =
                _codec->toUnicode(_private.fullStr.mid
                    (_private.labelsEnd, tmpLinkIter - _private.labelsEnd));
        synLinkText.prepend(' ');
        _private.labelsEnd = tmpLinkIter;

        linksItems(type, synLinkText, linksSynomys, labels);
        processingLinks(type, synLinkText);

        if(type == 0)
            break;
    }
}
void repertoryEngine::remedRender(QVector<QVector<QGraphicsItemGroup*>> & array, bool sorting, quint64 * remedSize){
    if((_filterType & repertoryFilterType::remeds) == 0)
        return;

    if(_sorting)
        array.resize(4);

    quint16 prevRemed = 0;
    QVector<QGraphicsItemGroup*> * arrayPtr = nullptr;
    quint64 remed_size = 0;

    while(_private.labelsEnd < _private.fullStr.size()){
        quint16 remed = 0, author = 0, tLevel = 0;
        uint8_t rLevel = 0;

        if(_private.labelsEnd + 7 >= _private.fullStr.size())
            break;

        const auto startLabelsEnd = _private.labelsEnd;

        remed = qFromLittleEndian<quint16>(_private.fullStr.constData() + _private.labelsEnd);
        _private.labelsEnd += 2;
        rLevel = qFromLittleEndian<quint16>(_private.fullStr.constData() + _private.labelsEnd);
        ++_private.labelsEnd;
        author = qFromLittleEndian<quint16>(_private.fullStr.constData() + _private.labelsEnd);
        _private.labelsEnd += 2;
        tLevel = qFromLittleEndian<quint16>(_private.fullStr.constData() + _private.labelsEnd);
        _private.labelsEnd += 2;

        if(memcmp(&remed, "\x5a\x5a", 2) == 0 && rLevel == '\x5a')
            break;

        if(_private.remFilter != (quint16)-1 && (tLevel & _private.remFilter) == 0)
            continue;

        QString star;

        if((((char *)&author)[1] & (char)128) != 0){
            ((char *)&author)[1] ^= (char)128;//remed have * in the end
            star = '*';
        }

        try{
            const auto & rmStr = _cache->_cacheRemed.at(remed);
            const auto & auStr = _cache->_cacheAuthor.at(author);
            auto itN = rmStr.find('\0', _cache->_lenRem);
            auto itA = auStr.find('\0', _cache->_lenAuthor);
            auto authorName = QString::fromStdString(auStr.substr(_cache->_lenAuthor, itA - _cache->_lenAuthor)) + star;
            auto remedName = QString::fromStdString(rmStr.substr(_cache->_lenRem, itN - _cache->_lenRem));

            if(_private.pos.y() >= _private.heightView)
                break;

            if(rLevel == 0 || rLevel > 4)
                continue;

            if(prevRemed == remed)
                authorsSym(authorName, author, arrayPtr->back(), true);
            else{
                auto remedItem = new QGraphicsSimpleTextItem;
                const QFont * remedFonts[] = {&_fonts.defaultFont, &_fonts.italicFont, &_fonts.boldFont};
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

                if(sorting)
                    arrayPtr = &array[rLevel - 1];
                else
                    arrayPtr = &array[0];

                arrayPtr->push_back(group);

                authorsSym(authorName, author, arrayPtr->back());
                ++remed_size;
            }
        }
        catch(std::exception){ qDebug() << "error index" << author << remed; }

        prevRemed = remed;
    }

    if(remedSize != nullptr)
        *remedSize = remed_size;
}
QString repertoryEngine::renderingLabel(const bool pass){
    _symptom.at(_private.index.toStdString(), false);
    return functions::renderingLabel(_symptom, pass, _codec);
}
