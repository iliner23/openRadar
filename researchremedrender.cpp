#include "researchremedrender.h"

void researchRemedRender::setSymptomHeight(quint32 height){
    if(height == 0)
        throw std::logic_error("incorrect value");

    _symptomHeight = height;
}
void researchRemedRender::initFont(){
    _defFont = QFont("default", 10);
    _numberFont = QFont("default", 8);
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
    QMap<QString, QVector<quint8>> remeds;
    //<name treatment, QVector<intensity> (vec size = count of coincidences)>
    func::repertoryData repdata;
    constexpr quint16 _remFilter = -1;//temp

    blueprint->setHandlesChildEvents(false);
    blueprint->setFlag(QGraphicsItem::ItemHasNoContents);

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

                    //remeds[remedName] = remeds.value(remedName, 0) + iter.intensity;
                    remeds[remedName].append(std::get<1>(itrem));
                } catch(...) { qDebug() << "error value" << std::get<0>(itrem); }

                prevRemed = std::get<0>(itrem);
            }
        }
    }

    QVector<std::pair<QString, QVector<quint8>>> remed;

    for(auto & it : remeds.keys()){
        remed.append(std::pair(it, remeds.value(it)));
        remeds.remove(it);
    }

    auto sort = [](const auto & front, const auto & end){
        return front.second.size() > end.second.size();
    };

    std::sort(remed.begin(), remed.end(), sort);

    //drawing colour lines, symptom names and some digits
    //**************************************************************************************************************************************

    auto labels = new QGraphicsItemGroup;
    auto upperLine =new QGraphicsLineItem;
    auto lowerLine =new QGraphicsLineItem;

    labels->setHandlesChildEvents(false);
    labels->setFlag(QGraphicsItem::ItemHasNoContents);
    QPointF lastPos;
    QRectF boundRect;
    qreal upperLineYPos = 0, lowerLineYPos = 0, startXPos = 0;

    for(auto it = 0; it != remed.size(); ++it){
        auto text = new QGraphicsSimpleTextItem(remed.at(it).first);
        auto sum = new QGraphicsSimpleTextItem(QString::number(remed.at(it).second.size()));
        auto number = new QGraphicsSimpleTextItem(QString::number(it + 1));

        text->setFont(_defFont);
        text->setPos(lastPos);
        text->setRotation(-45.0);

        if(it == 0){
            boundRect = text->boundingRect();
            boundRect.setHeight(boundRect.height() + boundRect.height() * 0.5);
            startXPos = -0.3 * boundRect.height();
        }

        QPointF newPos(boundRect.height(), 0);

        number->setFont(_numberFont);
        number->setPos(lastPos.x() - 0.2 * boundRect.height(), -startXPos * 2);
        upperLineYPos = number->y() + number->boundingRect().height();

        sum->setFont(_numberFont);
        sum->setPos(lastPos.x() - 0.2 * boundRect.height(), upperLineYPos);
        lowerLineYPos = sum->y() + sum->boundingRect().height();

        if(it % 2 == 0){
            auto line = new QGraphicsPolygonItem;
            const auto rad = std::acos(-1.0) / 180.0;
            constexpr auto width = 50;
            QPolygonF polygon;

            polygon << QPointF(startXPos + lastPos.x(), -startXPos)
                    << QPointF(startXPos + lastPos.x(), 1000)
                    << QPointF(startXPos + lastPos.x() + boundRect.height(), 1000)
                    << QPointF(startXPos + lastPos.x() + boundRect.height(), -startXPos);

            polygon << QPointF(boundRect.height() + polygon.at(0).x() + std::cos(45.0 * rad) * width,
                                            -startXPos + std::sin(-45.0 * rad) * width)
                    << QPointF(polygon.at(0).x() + std::cosf(45.0 * rad) * width,
                                            -startXPos + std::sinf(-45.0 * rad) * width);
            line->setZValue(-1);
            line->setPolygon(polygon);
            line->setPen(QPen(Qt::yellow));
            line->setBrush(QBrush(Qt::yellow));
            labels->addToGroup(line);
        }

        lastPos = lastPos + newPos;

        labels->addToGroup(number);
        labels->addToGroup(sum);
        labels->addToGroup(text);
    }

    upperLine->setLine(startXPos, upperLineYPos, lastPos.x(), upperLineYPos);
    labels->addToGroup(upperLine);

    lowerLine->setLine(startXPos, lowerLineYPos, lastPos.x(), lowerLineYPos);
    labels->addToGroup(lowerLine);

    blueprint->addToGroup(labels);

    //**************************************************************************************************************************************

    //auto symptomYLine = lowerLineYPos + _clipboardHeight;

    return blueprint;
}
