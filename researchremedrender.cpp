#include "researchremedrender.h"

void researchRemedRender::setSymptomHeight(quint32 height){
    if(height == 0)
        throw std::logic_error("incorrect value");

    _symptomHeight = height;
}
void researchRemedRender::initFont(){
    constexpr auto fontSize = 10;
    _defFont = QFont("default", fontSize);
    _numberFont = QFont("default", fontSize * 0.8);
}
QGraphicsItemGroup * researchRemedRender::render(const QSize windowSize){
    _windowSize = windowSize;

    switch (_showType) {
    case showType::waffle :
        return waffleRender();
    default :
        return nullptr;
    }
}
QGraphicsItemGroup * researchRemedRender::waffleRender(){
    //pasring remeds
    //**************************************************************************************************************************************
    auto * blueprint = new QGraphicsItemGroup;
    QMap<QString, std::pair<QVector<quint8>, int>> remeds;
    //<name treatment, pair<QVector<intensity>, num> (num = count of coincidences)>
    func::repertoryData repdata;
    constexpr quint16 _remFilter = -1;//temp

    blueprint->setHandlesChildEvents(false);
    blueprint->setFlag(QGraphicsItem::ItemHasNoContents);

    auto numrep = 0, repSize = 0;
    std::array<int, 10> clipList;

    for(auto it = 0; it != _clipboards.size(); ++it){
        repSize += _clipboards.at(it).size();
        clipList[it] = repSize;
    }

    for(const auto & it : _clipboards){
        for(const auto & iter : it){
            if(iter.intensity == 0)
                continue;

            quint16 prevRemed = 0;
            openCtree data(iter.path.filePath("symptom").toStdString());

            data.at(iter.key.toStdString(), false);
            repdata.reset(data);
            const auto rem = repdata.remedsList();

            for(const auto & itrem : rem){
                if(_remFilter != (quint16)-1 && (std::get<3>(itrem) & _remFilter) == 0)
                    continue;
                if(prevRemed == std::get<0>(itrem))
                    continue;

                try{
                    const auto & rmStr = _cache->_cacheRemed.at(std::get<0>(itrem));
                    auto itN = rmStr.find('\0', _cache->_lenRem);
                    auto remedName = QString::fromStdString(rmStr.substr(_cache->_lenRem, itN - _cache->_lenRem));

                    if(!remeds.contains(remedName)){
                        remeds[remedName].first.resize(repSize);
                        remeds[remedName].second = 0;
                    }

                    remeds[remedName].first[numrep] = std::get<1>(itrem);
                    remeds[remedName].second += iter.intensity;

                } catch(...) { qDebug() << "error value" << std::get<0>(itrem); }

                prevRemed = std::get<0>(itrem);
            }

            ++numrep;
        }
    }

    QVector<std::tuple<QString, QVector<quint8>, int>> remed;

    for(auto & it : remeds.keys()){
        const auto val = remeds.value(it);
        remed.append(std::tuple(it, val.first, val.second));
        remeds.remove(it);
    }

    auto sort = [](const auto & front, const auto & end){
        return std::get<2>(front) > std::get<2>(end);
    };

    std::sort(remed.begin(), remed.end(), sort);

    //drawing colour lines, symptom names and some digits
    //**************************************************************************************************************************************

    auto labels = new QGraphicsItemGroup;
    auto groupRemeds = new QGraphicsItemGroup;

    auto upperLine = new QGraphicsLineItem;
    auto lowerLine = new QGraphicsLineItem;

    QPointF lastPos;
    QVector<QPolygonF> lines;
    qreal minusHeight = 0, boundHeight = 0;
    qreal upperLineYPos = 0, lowerLineYPos = 0;
    qreal startXPos = 0, offsetXPos = 0;

    const auto rad = std::acos(-1.0) / 180.0;

    labels->setHandlesChildEvents(false);
    labels->setFlag(QGraphicsItem::ItemHasNoContents);

    groupRemeds->setHandlesChildEvents(false);
    groupRemeds->setFlag(QGraphicsItem::ItemHasNoContents);

    for(auto it = 0; it != remed.size(); ++it){
        auto text = new QGraphicsSimpleTextItem(std::get<0>(remed.at(it)));
        auto sum = new QGraphicsSimpleTextItem(QString::number(std::get<2>(remed.at(it))));//TODO : fix this counter
        auto number = new QGraphicsSimpleTextItem(QString::number(it + 1));

        text->setFont(_defFont);
        text->setPos(lastPos);
        text->setRotation(-45.0);
        auto boundRect = text->boundingRect();

        if(it == 0){
            boundHeight = boundRect.height() + boundRect.height() * 0.5;
            offsetXPos = 0.2 * boundHeight;
            startXPos = -0.3 * boundHeight;

            upperLineYPos = -startXPos * 2 + number->boundingRect().height();
            lowerLineYPos = upperLineYPos + number->boundingRect().height();
        }

        QPointF newPos(boundHeight, 0);

        number->setFont(_numberFont);
        number->setPos(lastPos.x() - offsetXPos, -startXPos * 2);

        sum->setFont(_numberFont);
        sum->setPos(lastPos.x() - offsetXPos, upperLineYPos);

        minusHeight = (minusHeight < boundRect.width() + boundRect.width() * 0.35) ?
                    boundRect.width() + boundRect.width() * 0.35 : minusHeight;

        if(it % 2 == 0){
            QPolygonF polygon;

            polygon << QPointF(startXPos + lastPos.x(), -startXPos)
                    << QPointF(startXPos + lastPos.x(), 1000)
                    << QPointF(startXPos + lastPos.x() + boundHeight, 1000)
                    << QPointF(startXPos + lastPos.x() + boundHeight, -startXPos);

            lines.append(polygon);
        }

        qreal roundPos = 0;

        for(auto deg = 0; deg != std::get<1>(remed.at(it)).size(); ++deg){//INFO : test
            const QColor remedColors[] = {Qt::white, Qt::darkGreen, Qt::blue, Qt::red, Qt::darkRed};
            auto rRect = new QGraphicsPolygonItem;
            QPainterPath pol;

            pol.addRoundedRect(0, 0, boundHeight - boundHeight * 0.2, _symptomHeight - _symptomHeight * 0.2, 5, 5);
            pol.translate(lastPos.x() -0.2 * boundHeight, roundPos);
            rRect->setPolygon(pol.toFillPolygon());
            rRect->setBrush(QBrush(remedColors[std::get<1>(remed.at(it)).at(deg)]));
            groupRemeds->addToGroup(rRect);

            if(std::find(clipList.cbegin(), clipList.cend(), deg + 1) != clipList.cend())
                roundPos += _clipboardHeight + rRect->boundingRect().height();
            else
                roundPos += _symptomHeight;
        }

        lastPos = lastPos + newPos;

        labels->addToGroup(number);
        labels->addToGroup(sum);
        labels->addToGroup(text);
    }

    for(auto & it : lines){
        auto line = new QGraphicsPolygonItem;

        it << QPointF(boundHeight + it.at(0).x() + std::cos(45.0 * rad) * minusHeight,
                                        -startXPos + std::sin(-45.0 * rad) * minusHeight)
                << QPointF(it.at(0).x() + std::cosf(45.0 * rad) * minusHeight,
                                        -startXPos + std::sinf(-45.0 * rad) * minusHeight);
        line->setZValue(-1);
        line->setPolygon(it);
        line->setPen(QPen(Qt::yellow));
        line->setBrush(QBrush(Qt::yellow));
        labels->addToGroup(line);
    }

    lines.clear();

    groupRemeds->setY(lowerLineYPos + _clipboardHeight);
    groupRemeds->setZValue(1);

    upperLine->setLine(startXPos, upperLineYPos, lastPos.x(), upperLineYPos);
    labels->addToGroup(upperLine);

    lowerLine->setLine(startXPos, lowerLineYPos, lastPos.x(), lowerLineYPos);
    labels->addToGroup(lowerLine);

    const auto tp = lowerLineYPos + std::sinf(45.0 * rad) * minusHeight;

    _labelHeight = tp - tp * 0.065;

    blueprint->addToGroup(groupRemeds);
    blueprint->addToGroup(labels);

    //**************************************************************************************************************************************

    return blueprint;
}
