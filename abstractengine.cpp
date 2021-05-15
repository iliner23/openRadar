#include "abstractengine.h"

abstractEngine::abstractEngine(){
    _fonts.fontName = "cursive";
    _fonts.italicFont = QFont(_fonts.fontName, 10, -1, true);
    _fonts.commonFont = QFont(_fonts.fontName, 10, QFont::DemiBold);
    _fonts.defaultFont = QFont(_fonts.fontName, 10, QFont::Light);
    _fonts.labelChapterFont = QFont(_fonts.fontName, 16, QFont::DemiBold);
    _fonts.smallFont = QFont(_fonts.fontName, 8);
    _fonts.boldFont = QFont(_fonts.fontName, 10, QFont::Bold);
}
void abstractEngine::addRemeds(QGraphicsItem * temp, const int labelWidth, const int widthView){
    if(temp == nullptr)
        return;

    const auto bon = temp->boundingRect();
    if(_pos.x() + _size.width() + bon.width() + 3 >= widthView){
        temp->setX(labelWidth + 3);
        temp->setY(_pos.y() + _size.height() + _spaceHeight);
    }
    else{
        temp->setX(_pos.x() + _size.width() + 3);
        temp->setY(_pos.y());
    }

    _scene->addItem(temp);
    _size = bon;
    _pos = temp->pos();
}
void abstractEngine::addLabel(QGraphicsItem * temp){
    temp->setY(_pos.y() + _size.height() + 1);
    _scene->addItem(temp);
    _size = temp->boundingRect();
    _pos = temp->pos();
}
void abstractEngine::authorsSym(const QString & autr, const quint16 author, QGraphicsItemGroup * allrm, const bool next){
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
};
void abstractEngine::renderingView(const int heightView, const int widthView){
    _scene->clear();
    _scene->setSceneRect(0, 0, widthView, heightView);

    QByteArray fullStr;

    if(!_index.isEmpty())
        fullStr = QByteArray::fromStdString(_symptom.at(_index.toStdString()));
    else
        fullStr = QByteArray::fromStdString(_symptom.front());

    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    _pos = {0, 0};
    _size = {0, 0, 0, 0};

    while(_pos.y() + _size.height() < heightView){
        if(_pos.y() != 0)
            _pos.setY(_pos.y() + _spaceHeight);

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
            QString labelText[2];//first - native, second - localize
            const auto firstZero = return_size(fullStr.indexOf('\0', _symptom.serviceDataLenght()), fullStr.size());
            const auto secondZero = return_size(fullStr.indexOf('\0', firstZero + 1), fullStr.size());
            labelsEnd = secondZero;

            labelText[0] = _codec->toUnicode(
                        fullStr.mid(_symptom.serviceDataLenght(), firstZero - _symptom.serviceDataLenght()));
            labelText[1] = _codec->toUnicode
                        (fullStr.mid(firstZero + 1, secondZero - firstZero - 1));

            localize = !labelText[1].isEmpty();

            const QString filler[] = {"", "", "-", ". ", "", "", " ", "  "};
            const auto isLocalize = ((localize) ? 2 : 1);
            const bool star = (fullStr.at(6) != '\0' || fullStr.at(7) != '\0') ? true : false;//values with dup key adding * in the end

            for(auto it = 0; it < isLocalize; ++it){
                auto textItem = new QGraphicsSimpleTextItem;
                textItem->setFont(_fonts.commonFont);

                if(attach == 1){
                    if(it == 0)
                        textItem->setBrush(QBrush(Qt::red));

                    textItem->setFont(_fonts.labelChapterFont);
                }

                if(it == 1)
                    textItem->setBrush(QBrush(Qt::blue));

                if(hideLabel)
                    textItem->setBrush(QBrush(Qt::gray));

                const auto fillStr = QString(attach * _attachRatio, ' ')
                        + filler[lessAttach - 1 + ((it == 0) ? 0 : 4)];

                textItem->setText(fillStr + labelText[it] + ((star) ? "*" : "") +
                                  ((maxDrug != 0 && it == isLocalize - 1) ? ": " : ""));

                const QFontMetrics metric(textItem->font());
                labelWidth = metric.horizontalAdvance(fillStr);

                textItem->setData(0, 0);
                textItem->setData(1, QByteArray::fromStdString(_symptom.key()));
                addLabel(textItem);
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

                    const auto tmpLinkIter = return_size(fullStr.indexOf('\0', labelsEnd), fullStr.size());
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
                            textItem->setFont(_fonts.labelChapterFont);
                        else
                            textItem->setFont(_fonts.commonFont);

                        textItem->setText(QString(attach * _attachRatio, ' ')
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

                            textItem->setText(QString(attach * _attachRatio, ' ') + tempLinkText);
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
                quint16 prevRemed = 0;
                quint64 remed_size = 0;

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

                    if(memcmp(&remed, "\x5a\x5a", 2) == 0 && rLevel == '\x5a')
                        break;

                    if(_remFilter != (quint16)-1 && (tLevel & _remFilter) == 0)
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

                    if(_pos.y() >= heightView)
                        break;

                    if(rLevel == 0 || rLevel > 4)
                        continue;

                    if(prevRemed == remed)
                        authorsSym(authorName, author, remeds.back(), true);
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
                        remeds.push_back(group);

                        authorsSym(authorName, author, remeds.back());
                        ++remed_size;
                    }

                    prevRemed = remed;
                }

                if(remed_size != 0){
                    auto siz = new QGraphicsSimpleTextItem;
                    siz->setFont(QFont(_fonts.fontName, 10));
                    siz->setText("(" + QString::number(remed_size) + ") ");
                    addRemeds(siz, labelWidth, widthView);
                }

                for(const auto & it : remeds)
                    addRemeds(it, labelWidth, widthView);

                if(!remeds.isEmpty())
                    _pos.setY(_pos.y() + _spaceHeight);
            }

            for(const auto & it : linksSynomys)
                addLabel(it);
        }

        if(_symptom.key() == _endIndex.toStdString())
            break;

        fullStr = QByteArray::fromStdString(_symptom.next());
    }
}
QString abstractEngine::renderingLabel(const bool pass){
    return renderingLabel(QByteArray::fromStdString(_symptom.currentValue()), _symptom, pass, _codec);
}
QString abstractEngine::renderingLabel(QByteArray text, openCtree & symptom, bool pass, QTextCodec * codec){
    QStringList original, localization;
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
