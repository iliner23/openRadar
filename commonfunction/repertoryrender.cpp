#include "repertoryrender.h"

void repertoryRender::initFonts(){
    _fonts.fontName = "cursive";
    _fonts.italicFont = QFont(_fonts.fontName, 10, -1, true);
    _fonts.commonFont = QFont(_fonts.fontName, 10, QFont::DemiBold);
    _fonts.defaultFont = QFont(_fonts.fontName, 10, QFont::Light);
    _fonts.labelChapterFont = QFont(_fonts.fontName, 16, QFont::DemiBold);
    _fonts.smallFont = QFont(_fonts.fontName, 8);
    _fonts.boldFont = QFont(_fonts.fontName, 10, QFont::Bold);
}
repertoryRender::repertoryRender(const QDir filename, const std::shared_ptr<func::cache> & cache, QTextCodec * codec){
    initFonts();
    repertoryRender::reset(filename, cache, codec);
}
void repertoryRender::reset(const QDir filename, const std::shared_ptr<func::cache> &cache, QTextCodec *codec){
    _symptom.open(filename.path().toStdString());
    _symptom.back(false);
    _endIndex = QByteArray::fromStdString(_symptom.key());
    _cache = cache;
    _codec = codec;

    _remFilter = -1;//repertory filter in widget menu
    _filterType = (quint8) repertoryFilterType::all;
    _sortRemedType = repertorySortRemedsType::none;
    _labelTypes = repertoryLabelType::all;
    _reverseLabels = false;
    _counter = true;
    _navigation.clear();
    _index.clear();
}
void repertoryRender::setCurrentKey(const QByteArray key){
    _symptom.at(key.toStdString(), false);
    _index = key;
}
void repertoryRender::setCurrentPosition(int pos){
    _symptom.at(pos, false);
    _index = QByteArray::fromStdString(_symptom.key());
}
QString repertoryRender::renderingLabel(const bool pass){
    _symptom.at(_index.toStdString(), false);
    return func::renderingLabel(_symptom, pass, _codec);
}
void repertoryRender::addRemeds(QGraphicsItemGroup * group, QPointF pos, QVector<QVector<QGraphicsItemGroup*>> & array, int labelWidth) const{
    QRectF size;

    auto func = [&](auto it){
        for(auto & iter : *it){
            if(iter == nullptr)
                continue;

            const auto bon = iter->boundingRect();
            if(pos.x() + size.width() + bon.width() + 3 >= _resolution.width()){
                iter->setX(labelWidth + 3);

                if(!size.isEmpty())
                    iter->setY(pos.y() + size.height() + 5);
                else
                    iter->setY(pos.y() + iter->boundingRect().height() + 5);
            }
            else{
                iter->setX(pos.x() + size.width() + 3);
                iter->setY(pos.y());
            }

            group->addToGroup(iter);
            size = bon;
            pos = iter->pos();
        }
    };

    if(_sortRemedType == repertorySortRemedsType::more){
        for(auto it = array.rbegin(); it != array.rend(); ++it)
            func(it);
    }
    else{
        for(auto it = array.begin(); it != array.end(); ++it)
            func(it);
    }
}
void repertoryRender::authorsSym(const QString & autr, const quint16 author, QGraphicsItemGroup * allrm, const bool next){
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
QVector<QGraphicsItemGroup*> repertoryRender::render(const QSize resolution){
    QVector<QGraphicsItemGroup*> endData;
    _navigation.clear();

    openCtree data = _symptom;
    _resolution = resolution;

    for(auto heigth = 0; heigth < resolution.height();
            heigth += endData.back()->boundingRect().height() + 3){

        auto gr = subRender(data);
        gr->setY(heigth);
        endData.push_back(gr);

        if(data.key() == _endIndex.toStdString())
            break;

        data.next();
    }

    return endData;
}
QGraphicsItemGroup * repertoryRender::subRender(openCtree data){
    func::repertoryData dataParser(data, _codec);
    bool hideLabel = !(_remFilter == (quint16)-1 ||
                       (dataParser.labelIndexFilter() & _remFilter) != 0);

    if(dataParser.level() == 0)
        return nullptr;

    constexpr auto attachRatio = 3;
    int labelWidth = 0;
    const bool localization = !dataParser.repertoryLabel().second.isEmpty();

    auto group = new QGraphicsItemGroup;
    group->setHandlesChildEvents(false);
    group->setFlag(QGraphicsItem::ItemHasNoContents);

    QVector<QGraphicsSimpleTextItem*> crsLks;
    QVector<QVector<QGraphicsItemGroup*>> array(1);
    QGraphicsSimpleTextItem * ptrLastLabel = nullptr;

    if((_filterType & (quint8) repertoryFilterType::chapter) != 0){
        //label
        const QString filler[] = {"", "", "-", ". ", "", "", " ", "  "};
        const auto dub = dataParser.secondCurrentIndex();
        const bool star = (dub.at(5) != '\0' ||
                dub.at(4) != '\0') ? true : false;
        //values with dup key adding * in the end

        const uint8_t lessAttach = (dataParser.level() > 4) ? 1 : dataParser.level();
        const auto labels = dataParser.repertoryLabel();
        bool iterLab = (_reverseLabels) ? true : false;

        for(auto it = 0; it != 2; ++it){
            if(it == 1)
                iterLab = !iterLab;

            if(!iterLab && (labels.first.isEmpty() ||
                    ((quint8) _labelTypes & (quint8) repertoryLabelType::original) == 0))
                continue;
            else if(iterLab && (labels.second.isEmpty() ||
                    ((quint8) _labelTypes & (quint8) repertoryLabelType::localize) == 0))
                continue;

            auto textItem = new QGraphicsSimpleTextItem;

            if(dataParser.level() == 1){
                if(!iterLab)
                    textItem->setBrush(QBrush(Qt::red));

                textItem->setFont(_fonts.labelChapterFont);
            }
            else
                textItem->setFont(_fonts.commonFont);

            if(hideLabel)
                textItem->setBrush(QBrush(Qt::gray));
            else if(iterLab)
                textItem->setBrush(QBrush(Qt::blue));

            const auto fillStr = QString(dataParser.level() * attachRatio, ' ')
                    + filler[lessAttach - 1 + ((it == 0) ? 0 : 4)];

            const auto syn = (!iterLab) ? dataParser.synonymsList().first :
                                          dataParser.synonymsList().second;

            textItem->setText(fillStr % ((!iterLab) ? labels.first : labels.second) % ((star) ? "*" : "") %
                        ((!syn.isEmpty()) ? ' ' + syn : "")
                      % ((dataParser.maxDrugs() != 0 && it == 1) ? ": " : "  "));

            const QFontMetrics metric(textItem->font());
            labelWidth = metric.horizontalAdvance(fillStr);

            textItem->setData(0, 0);
            textItem->setData(1, QByteArray::fromStdString(dataParser.key()));
            textItem->setY(group->boundingRect().height() + 1);

            ptrLastLabel = textItem;
            group->addToGroup(textItem);
        }

        //position marker
        auto pos = new QGraphicsSimpleTextItem("->");
        pos->hide();
        pos->setPos(0, 0);
        pos->setData(0, 5);
        pos->setData(1, QByteArray::fromStdString(dataParser.key()));
        _navigation.push_back(pos);
        group->addToGroup(pos);
    }
    if((_filterType & (quint8) repertoryFilterType::links) != 0 && !hideLabel){
        //links
        const QString constLinks[] =
                        {u8" \u2b08", u8" \u2b08", u8"  \u2b08", u8"   \u2b08"};
        const QString constSee[] = {"(see ", "(see ", " (see ", "  (see "};
        const QColor constColor[] = {Qt::darkRed, Qt::darkBlue, Qt::red, Qt::blue};

        const auto links = dataParser.linksList();
        const auto crossLinks = dataParser.crossLinksList();

        const uint8_t lessAttach = (dataParser.level() > 4) ? 1 : dataParser.level();

        auto func = [&](const auto & data, const auto type){
            QVector<QGraphicsSimpleTextItem *> ptr;

            for(auto i = 0; i != data.size(); ++i){
                auto textItem = new QGraphicsSimpleTextItem;
                auto tempLinkText = data.at(i).mid(1);

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

                textItem->setText(QString(dataParser.level() * attachRatio, ' ') + tempLinkText);
                textItem->setBrush(QBrush(constColor[type +
                                   ((!localization && (type + 1) % 2 != 0) ? 1 : 0)]));

                textItem->setData(1, data.at(i));
                textItem->setData(2, QByteArray::fromStdString(dataParser.key()));
                textItem->setData(3, type == 2 || type == 0);//localization
                ptr.push_back(textItem);
            }

            return ptr;
        };

        if(!_reverseLabels){
            if(((quint8) _labelTypes & (quint8) repertoryLabelType::original) != 0)
                crsLks += func(links.first, 3);
            if(((quint8) _labelTypes & (quint8) repertoryLabelType::localize) != 0)
                crsLks += func(links.second, 2);//loc
            if(((quint8) _labelTypes & (quint8) repertoryLabelType::original) != 0)
                crsLks += func(crossLinks.first, 1);
            if(((quint8) _labelTypes & (quint8) repertoryLabelType::localize) != 0)
                crsLks += func(crossLinks.second, 0);//loc
        }
        else{
            if(((quint8) _labelTypes & (quint8) repertoryLabelType::localize) != 0)
                crsLks += func(links.second, 2);//loc
            if(((quint8) _labelTypes & (quint8) repertoryLabelType::original) != 0)
                crsLks += func(links.first, 3);
            if(((quint8) _labelTypes & (quint8) repertoryLabelType::localize) != 0)
                crsLks += func(crossLinks.second, 0);//loc
            if(((quint8) _labelTypes & (quint8) repertoryLabelType::original) != 0)
                crsLks += func(crossLinks.first, 1);
        }
    }
    if((_filterType & (quint8) repertoryFilterType::remeds) != 0 && !hideLabel){//remeds
        if(_sortRemedType != repertorySortRemedsType::none)
            array.resize(4);

        const auto remeds = dataParser.remedsList();
        quint16 prevRemed = 0;
        auto counter = -1;
        QVector<QGraphicsItemGroup*> * arrayPtr = nullptr;
        //remed, type, author, filter

        for(const auto & remIt : remeds){
            ++counter;

            if(_remFilter != (quint16)-1 && (std::get<3>(remIt) & _remFilter) == 0)
                continue;

            QString star;
            auto author = std::get<2>(remIt);

            if((((char *)&author)[1] & (char)128) != 0){
                ((char *)&author)[1] ^= (char)128;//remed have * in the end
                star = '*';
            }

            try{
                const auto & rmStr = _cache->_cacheRemed.at(std::get<0>(remIt));
                const auto & auStr = _cache->_cacheAuthor.at(author);
                auto itN = rmStr.find('\0', _cache->_lenRem);
                auto itA = auStr.find('\0', _cache->_lenAuthor);
                auto authorName = QString::fromStdString(auStr.substr(_cache->_lenAuthor, itA - _cache->_lenAuthor)) + star;
                auto remedName = QString::fromStdString(rmStr.substr(_cache->_lenRem, itN - _cache->_lenRem));

                if(prevRemed == std::get<0>(remIt)){
                    authorsSym(authorName, author, arrayPtr->back(), true);
                }
                else{
                    auto remedItem = new QGraphicsSimpleTextItem;
                    const QFont * remedFonts[] = {&_fonts.defaultFont, &_fonts.italicFont, &_fonts.boldFont};
                    const QColor remedColors[] = {Qt::darkGreen, Qt::blue, Qt::red, Qt::darkRed};

                    remedItem->setFont(*remedFonts[((std::get<1>(remIt) == 4) ? 2 : std::get<1>(remIt) - 1)]);
                    remedItem->setBrush(remedColors[std::get<1>(remIt) - 1]);
                    remedItem->setText((std::get<1>(remIt) > 2) ? remedName.toUpper() : remedName);

                    remedItem->setData(0, 3);
                    remedItem->setData(1, counter);
                    remedItem->setData(2, QByteArray::fromStdString(dataParser.key()));

                    auto gr = new QGraphicsItemGroup;
                    gr->addToGroup(remedItem);
                    gr->setHandlesChildEvents(false);
                    gr->setFlag(QGraphicsItem::ItemHasNoContents);

                    if(_sortRemedType != repertorySortRemedsType::none)
                        arrayPtr = &array[std::get<1>(remIt) - 1];
                    else
                        arrayPtr = &array[0];

                    arrayPtr->push_back(gr);
                    authorsSym(authorName, author, arrayPtr->back());
                }
            }
            catch(std::exception)
                { qDebug() << "error index" << author << std::get<0>(remIt); }

            prevRemed = std::get<0>(remIt);
        }
    }

    QPointF point;

    if(ptrLastLabel != nullptr){
        //remeds counter
        int remed_count = 0;

        for(const auto & it : array)
            remed_count += it.size();

        if(remed_count != 0 && _counter){
            auto ins = ptrLastLabel->text();
            ptrLastLabel->setText(ins.insert(ins.size() - 2 , " (" %
                              QString::number(remed_count) % ") "));
        }

        point.setX(ptrLastLabel->boundingRect().width());
        point.setY(ptrLastLabel->y() + 1);
    }

    addRemeds(group, point, array, labelWidth);

    for(auto & it : crsLks){
        it->setY(group->boundingRect().height() + 1);
        group->addToGroup(it);
    }

    return group;
}
