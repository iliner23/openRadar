#include "repertoryengine.h"
repertoryEngine::repertoryEngine(QGraphicsScene *parent): QObject(parent){
    initFonts();
}

repertoryEngine::repertoryEngine(const QDir & filename, const std::shared_ptr<cache> & cache
                                 , QGraphicsScene *parent, QTextCodec * codec) : QObject(parent){
    initFonts();
    reset(filename, cache, codec);
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
void repertoryEngine::reset(const QDir &filename, const std::shared_ptr<cache> &cache, QTextCodec *codec){
    _symptom.open(filename.filePath("symptom").toStdString());
    _symptom.back(false);
    _render.endIndex = QByteArray::fromStdString(_symptom.key());
    _cache = cache;
    _codec = codec;
}
void repertoryEngine::addRemeds(QGraphicsItem * temp){
    if(temp == nullptr)
        return;

    const auto bon = temp->boundingRect();
    if(_render.pos.x() + _render.size.width() + bon.width() + 3 >= _render.widthView){
        temp->setX(_render.labelWidth + 3);
        temp->setY(_render.pos.y() + _render.size.height() + _render.spaceHeight);
    }
    else{
        temp->setX(_render.pos.x() + _render.size.width() + 3);
        temp->setY(_render.pos.y());
    }

    qobject_cast<QGraphicsScene*>(parent())->addItem(temp);
    _render.size = bon;
    _render.pos = temp->pos();
}
void repertoryEngine::addLabel(QGraphicsItem * temp){
    temp->setY(_render.pos.y() + _render.size.height() + 1);
    qobject_cast<QGraphicsScene*>(parent())->addItem(temp);
    _render.size = temp->boundingRect();
    _render.pos = temp->pos();
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
void repertoryEngine::renderingChapter(){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    const auto firstZero = return_size(_render.fullStr.indexOf('\0', _symptom.serviceDataLenght()), _render.fullStr.size());
    const auto secondZero = return_size(_render.fullStr.indexOf('\0', firstZero + 1), _render.fullStr.size());
    _render.labelsEnd = secondZero;

    if((_filter & repertoryFilterType::chapter) == 0){
        _render.localize = (secondZero - firstZero - 1 <= 0) ? false : true;
        return;
    }

    QString labelText[2];//first - native, second - localize
    labelText[0] = _codec->toUnicode(
                _render.fullStr.mid(_symptom.serviceDataLenght(), firstZero - _symptom.serviceDataLenght()));
    labelText[1] = _codec->toUnicode
                (_render.fullStr.mid(firstZero + 1, secondZero - firstZero - 1));

    _render.localize = !labelText[1].isEmpty();

    const QString filler[] = {"", "", "-", ". ", "", "", " ", "  "};
    const auto isLocalize = ((_render.localize) ? 2 : 1);
    const bool star = (_render.fullStr.at(6) != '\0' || _render.fullStr.at(7) != '\0') ? true : false;//values with dup key adding * in the end
    const uint8_t lessAttach = (_render.attach > 4) ? 1 : _render.attach;

    for(auto it = 0; it < isLocalize; ++it){
        auto textItem = new QGraphicsSimpleTextItem;
        textItem->setFont(_fonts.commonFont);

        if(_render.attach == 1){
            if(it == 0)
                textItem->setBrush(QBrush(Qt::red));

            textItem->setFont(_fonts.labelChapterFont);
        }

        if(it == 1)
            textItem->setBrush(QBrush(Qt::blue));

        if(_render.hideLabel)
            textItem->setBrush(QBrush(Qt::gray));

        const auto fillStr = QString(_render.attach * _render.attachRatio, ' ')
                + filler[lessAttach - 1 + ((it == 0) ? 0 : 4)];

        textItem->setText(fillStr + labelText[it] + ((star) ? "*" : "") +
                          ((_render.maxDrug != 0 && it == isLocalize - 1) ? ": " : ""));

        const QFontMetrics metric(textItem->font());
        _render.labelWidth = metric.horizontalAdvance(fillStr);

        textItem->setData(0, 0);
        textItem->setData(1, QByteArray::fromStdString(_symptom.key()));
        addLabel(textItem);

        if(it == 0 && _navigation && textItem->y() + textItem->boundingRect().height() <= _render.heightView){
            auto pos = new QGraphicsSimpleTextItem("->");
            pos->setX(textItem->x());
            pos->setY(textItem->y());
            pos->hide();
            pos->setData(0, 5);
            pos->setData(1, QByteArray::fromStdString(_symptom.key()));

            qobject_cast<QGraphicsScene*>(parent())->addItem(pos);
            _render.labelsVec.push_back(pos);
        }
    }
}
void repertoryEngine::setCurrentKey(const QByteArray &key){
    _symptom.at(key.toStdString(), false);
    _render.index = key;
}
void repertoryEngine::setCurrentPosition(int pos){
    _symptom.at(pos, false);
    _render.index = QByteArray::fromStdString(_symptom.key());
}
void repertoryEngine::render(const int heightView, const int widthView, const bool oneChapter){
    _render.fullStr =
            QByteArray::fromStdString(_symptom.at(_render.index.toStdString()));
    _render.heightView = heightView;
    _render.widthView = widthView;

    qobject_cast<QGraphicsScene*>(parent())->
            setSceneRect(0, 0, _render.widthView, _render.heightView);

    _render.labelWidth = 0;
    qobject_cast<QGraphicsScene*>(parent())->clear();
    _render.pos = {0, 0};
    _render.size = {0, 0, 0, 0};
    _render.labelsVec.clear();

    for(auto & it : _render.linksNames)
        it.clear();

    if(!oneChapter){
        while(_render.pos.y() + _render.size.height() < _render.heightView)
            if(loopRender()) break;
    }
    else
       loopRender();
}
bool repertoryEngine::loopRender(){
    if(_render.pos.y() != 0 &&
            ((_filter & (~repertoryFilterType::links)) != 0))
        _render.pos.setY(_render.pos.y() + _render.spaceHeight);

    _render.pos.setX(0);
    _render.size.setWidth(0);

    QVector<QGraphicsItem*> linksSynomys;
    linksSynomys.reserve(100);

    _render.attach = qFromLittleEndian<quint8>(_render.fullStr.constData() + 21);
    _render.maxDrug = qFromLittleEndian<quint16>(_render.fullStr.constData() + 22);
    _render.filter = qFromLittleEndian<quint16>(_render.fullStr.constData() + 26);

    if(_render.attach == 0)
        return false;

    _render.hideLabel = !(_render.remFilter == (quint16)-1 || (_render.filter & _render.remFilter) != 0);
    _render.localize = false;
    _render.labelsEnd = 0;

    renderingChapter();//label subfunction

    if(!_render.hideLabel){
        linksRender(linksSynomys);//synonyms and links

        if((_filter & repertoryFilterType::remeds) != 0){//remeds
            quint64 remed_size = 0;
            QVector<QVector<QGraphicsItemGroup*>> remeds(1);
            remedRender(remeds, _sorting, &remed_size);

            if(remed_size != 0 && _counter){
                auto siz = new QGraphicsSimpleTextItem;
                siz->setFont(QFont(_fonts.fontName, 10));
                siz->setText("(" + QString::number(remed_size) + ") ");
                addRemeds(siz);
            }

            for(auto it = remeds.rbegin(); it != remeds.rend(); ++it){
                for(auto & iter : *it)
                    addRemeds(iter);
            }


            if(!remeds[0].empty())
                _render.pos.setY(_render.pos.y() + _render.spaceHeight);
        }

        for(const auto & it : linksSynomys)
            addLabel(it);
    }

    if(_symptom.key() == _render.endIndex.toStdString())
        return true;

    _render.fullStr = QByteArray::fromStdString(_symptom.next());

    return false;
}
void repertoryEngine::linksItems(const quint8 type, const QString & synLinkText, QVector<QGraphicsItem *> & linksSynomys){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    const uint8_t lessAttach = (_render.attach > 4) ? 1 : _render.attach;

    if(synLinkText.size() > 2 && synLinkText.left(3) == " (="){
        const QString constSynomy[] =
                { "Synomy : ", "Synomy : ", " Synomy : ", " Synomy : "};
        auto textItem = new QGraphicsSimpleTextItem;

        if(type == 2)
            textItem->setBrush(QBrush(Qt::blue));
        else if(type == 3){
            if(_render.localize)
                textItem->setBrush(QBrush(Qt::red));
            else
                textItem->setBrush(QBrush(Qt::blue));
        }
        else{
            delete textItem;
            return;
        }

        if(_render.attach == 1)
            textItem->setFont(_fonts.labelChapterFont);
        else
            textItem->setFont(_fonts.commonFont);

        textItem->setText(QString(_render.attach * _render.attachRatio, ' ')
                          + constSynomy[lessAttach - 1] + synLinkText.mid(1));
        linksSynomys.push_back(textItem);
    }
    else{
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

            textItem->setText(QString(_render.attach * _render.attachRatio, ' ') + tempLinkText);
            textItem->setBrush(QBrush(constColor[type +
                               ((!_render.localize && (type + 1) % 2 != 0) ? 1 : 0)]));

            textItem->setData(1, linkText);
            textItem->setData(2, QByteArray::fromStdString(_symptom.key()));
            textItem->setData(3, !_render.localize);
            linksSynomys.push_back(textItem);
        }
    }
}
void repertoryEngine::linksStrings(const quint8 type , const QString & synLinkText){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    if(synLinkText.size() > 2 && synLinkText.left(3) == " (="){
        const auto sz = _render.linksNames[0].size() + 1;
        if(type == 3){
            if(sz % 2 == 0)
                _render.linksNames[0].push_back("");
        }
        else if(type == 2){
            if((_render.localize && sz % 2 != 0) ||
              (!_render.localize && sz % 2 == 0))
                _render.linksNames[0].push_back("");
        }
        else
            return;

        _render.linksNames[0].push_back(synLinkText.mid(1));
    }
    else{
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

            const auto lnIndex = (type > 1) ? 1 : 2;
            const auto delFor2 = (type > 1) ? _render.linksNames[1].size() + 1 : _render.linksNames[2].size() + 1;

            if(type == 3 || type == 1){
                if(delFor2 % 2 == 0)
                    _render.linksNames[lnIndex].push_back("");

                _render.linksNames[lnIndex].push_back(linkText);
            }
            else{
                if((_render.localize && delFor2 % 2 != 0) ||
                  (!_render.localize && delFor2 % 2 == 0))
                    _render.linksNames[lnIndex].push_back("");

                _render.linksNames[lnIndex].push_back(linkText);
            }
        }
    }
}
void repertoryEngine::linksRender(QVector<QGraphicsItem *> & linksSynomys){
    //synonyms and links
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    quint8 type = 4;

    for(auto i = 0; i != 4; ++i){
        quint8 cnt = 0;

        for(auto it = 0; it != type + 1; ++it){
            if(_render.labelsEnd == _render.fullStr.size()
                    || _render.fullStr.at(_render.labelsEnd) != '\0')
                break;

            ++cnt;
            ++_render.labelsEnd;
        }

        if(cnt > type)
            break;

        type = type - cnt;

        const auto tmpLinkIter = return_size
                (_render.fullStr.indexOf('\0', _render.labelsEnd), _render.fullStr.size());

        auto synLinkText =
                _codec->toUnicode(_render.fullStr.mid
                    (_render.labelsEnd, tmpLinkIter - _render.labelsEnd));
        synLinkText.prepend(' ');
        _render.labelsEnd = tmpLinkIter;

        if(_getLinksStr)
            linksStrings(type, synLinkText);

        if((_filter & repertoryFilterType::links) == 0)
            continue;

        linksItems(type, synLinkText, linksSynomys);

        if(type == 0)
            break;
    }
}
void repertoryEngine::remedRender(QVector<QVector<QGraphicsItemGroup*>> & array, bool sorting, quint64 * remedSize){
    if(_sorting)
        array.resize(4);

    quint16 prevRemed = 0;
    QVector<QGraphicsItemGroup*> * arrayPtr = nullptr;
    quint64 remed_size = 0;

    while(_render.labelsEnd < _render.fullStr.size()){
        quint16 remed = 0, author = 0, tLevel = 0;
        uint8_t rLevel = 0;

        if(_render.labelsEnd + 7 >= _render.fullStr.size())
            break;

        const auto startLabelsEnd = _render.labelsEnd;

        remed = qFromLittleEndian<quint16>(_render.fullStr.constData() + _render.labelsEnd);
        _render.labelsEnd += 2;
        rLevel = qFromLittleEndian<quint16>(_render.fullStr.constData() + _render.labelsEnd);
        ++_render.labelsEnd;
        author = qFromLittleEndian<quint16>(_render.fullStr.constData() + _render.labelsEnd);
        _render.labelsEnd += 2;
        tLevel = qFromLittleEndian<quint16>(_render.fullStr.constData() + _render.labelsEnd);
        _render.labelsEnd += 2;

        if(memcmp(&remed, "\x5a\x5a", 2) == 0 && rLevel == '\x5a')
            break;

        if(_render.remFilter != (quint16)-1 && (tLevel & _render.remFilter) == 0)
            continue;

        QString star;

        if((((char *)&author)[1] & (char)128) != 0){
            ((char *)&author)[1] ^= (char)128;//remed have * in the end
            star = '*';
        }

        const auto & rmStr = _cache->_cacheRemed.at(remed);
        const auto & auStr = _cache->_cacheAuthor.at(author);
        auto itN = rmStr.find('\0', _cache->_lenRem);
        auto itA = auStr.find('\0', _cache->_lenAuthor);
        auto authorName = QString::fromStdString(auStr.substr(_cache->_lenAuthor, itA - _cache->_lenAuthor)) + star;
        auto remedName = QString::fromStdString(rmStr.substr(_cache->_lenRem, itN - _cache->_lenRem));

        if(_render.pos.y() >= _render.heightView)
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

        prevRemed = remed;
    }

    if(remedSize != nullptr)
        *remedSize = remed_size;
}
QString repertoryEngine::renderingLabel(const bool pass){
    _symptom.at(_render.index.toStdString(), false);
    return renderingLabel(_symptom, pass, _codec);
}
QVector<QByteArray> repertoryEngine::getRootPath(openCtree & symptom){
    QVector<QByteArray> array;
    auto text = QByteArray::fromStdString(symptom.currentValue());
    QByteArray ind(6, '\0');

    std::reverse_copy(text.cbegin() + 6, text.cbegin() + 12, ind.begin());

    while(true){
        quint8 attach = 0;

        array.push_back(ind);
        text = QByteArray::fromStdString(symptom.at(ind.toStdString()));

        attach = text.at(21);
        QByteArray midCompare(6, '\0');
        std::reverse_copy(text.cbegin() + 6, text.cbegin() + 12, midCompare.begin());
        std::reverse_copy(text.cbegin() + 12, text.cbegin() + 18, ind.begin());

        if(midCompare.left(4) != ind.left(4)){
            ind.replace(ind.size() - 2, 2, "\0\0", 2);}

        else if(ind.back() != '\0' || ind.at(ind.size() - 1) != '\0'){
            auto decrease = qFromBigEndian<quint16>(ind.right(2)) - 1;
            decrease = qToBigEndian<quint16>(decrease);
            ind.replace(ind.size() - 2, 2, (char *)&decrease, 2);
        }

        if(attach == 0)
            Q_ASSERT("attach == 0");
        else if(attach <= 1 || ind == "\0\0\0\0\0\0")
            break;
    }

    return array;
}
QString repertoryEngine::renderingLabel(openCtree & symptom, bool pass, QTextCodec * codec){
    QStringList original, localization;
    auto text = QByteArray::fromStdString(symptom.currentValue());
    QByteArray ind(6, '\0');
    bool fis = true;

    auto return_size = [](const auto & value, const auto & size){
        return (value == -1) ? size : value;
    };

    if(pass == false){
        std::reverse_copy(text.cbegin() + 6, text.cbegin() + 12, ind.begin());
        fis = false;
    }

    while(true){
        quint8 attach = 0;

        if(!fis){
            text = QByteArray::fromStdString(symptom.at(ind.toStdString()));
            const auto first = return_size(text.indexOf('\0', symptom.serviceDataLenght()), text.size());
            const auto second = return_size(text.indexOf('\0', first + 1), text.size()) - first - 1;
            const auto local = codec->toUnicode(text.mid(first + 1, second));

            original.push_back(codec->toUnicode(text.mid(symptom.serviceDataLenght()
                                                          , first - symptom.serviceDataLenght())));
            if(!local.isEmpty())
                localization.push_back(local);
        }

        attach = text.at(21);
        QByteArray midCompare(6, '\0');
        std::reverse_copy(text.cbegin() + 6, text.cbegin() + 12, midCompare.begin());
        std::reverse_copy(text.cbegin() + 12, text.cbegin() + 18, ind.begin());

        if(midCompare.left(4) != ind.left(4)){
            ind.replace(ind.size() - 2, 2, "\0\0", 2);}

        else if(ind.back() != '\0' || ind.at(ind.size() - 1) != '\0'){
            auto decrease = qFromBigEndian<quint16>(ind.right(2)) - 1;
            decrease = qToBigEndian<quint16>(decrease);
            ind.replace(ind.size() - 2, 2, (char *)&decrease, 2);
        }

        if(attach == 0)
            Q_ASSERT("attach == 0");
        else if(attach <= 1 || ind == "\0\0\0\0\0\0")
            break;

        fis = false;
    }

    if(original.isEmpty())
        return QString();

    QString org, lz;

    for(auto it = original.size() - 1; it != -1; --it){
        if(it == original.size() - 1)
            org += original[it];
        else
            org += " - " + original[it];
    }

    if(!localization.isEmpty()){
        for(auto it = localization.size() - 1; it != -1; --it){
            if(it == localization.size() - 1)
                lz += localization[it];
            else
                lz += " - " + localization[it];
        }
    }

    return org + ((localization.isEmpty()) ? "" : '\n' + lz);
}
