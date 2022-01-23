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
    return sumRemeds();//TODO : switch strategy
}
QGraphicsItemGroup * researchRemedRender::drawLines(qreal lineWidth, qreal lengthText, int countLines){
    auto * lines = new QGraphicsItemGroup;
    lines->setHandlesChildEvents(false);
    lines->setFlag(QGraphicsItem::ItemHasNoContents);

    QPointF lastPos(-lineWidth, 0);
    const auto rad = std::acos(-1.0) / 180.0;

    for(auto i = 0; i != countLines; ++i){
        lastPos += QPointF(lineWidth, 0);

        if(i % 2 != 0)
            continue;

        auto line = new QGraphicsPolygonItem;
        QPolygonF polygon;

        polygon << QPointF(lastPos.x(), 0)
                << QPointF(lastPos.x(), lengthText)
                << QPointF(lastPos.x() + lineWidth, lengthText)
                << QPointF(lastPos.x() + lineWidth, 0)
                << QPointF(lineWidth + lastPos.x() + std::cos(45.0 * rad) * lineWidth * 3.5,
                                                std::sin(-45.0 * rad) * lineWidth * 3.5)
                << QPointF(lastPos.x() + std::cosf(45.0 * rad) * lineWidth * 3.5,
                                                std::sinf(-45.0 * rad) * lineWidth * 3.5);

        line->setPolygon(polygon);
        line->setPen(QPen(Qt::yellow));
        line->setBrush(QBrush(Qt::yellow));
        lines->addToGroup(line);
    }

    return lines;
}
std::tuple<QGraphicsItemGroup*, qreal, qreal> researchRemedRender::drawOneDigitsRemeds(const QVector<std::tuple<QString, QVector<quint8>, int, int>> remeds){
    auto blueprint = new QGraphicsItemGroup;
    blueprint->setHandlesChildEvents(false);
    blueprint->setFlag(QGraphicsItem::ItemHasNoContents);

    qreal boundHeight = 0;
    qreal upperLineYPos = 0, lowerLineYPos = 0;
    qreal startXPos = 0, offsetXPos = 0;
    QPointF lastPos;

    for(auto i = 0; i != remeds.size(); ++i){
        auto text = new QGraphicsSimpleTextItem(std::get<0>(remeds.at(i)));
        auto sum = new QGraphicsSimpleTextItem(QString::number(std::get<2>(remeds.at(i))));
        auto number = new QGraphicsSimpleTextItem(QString::number(i + 1));

        text->setFont(_defFont);
        text->setPos(lastPos);
        text->setRotation(-45.0);
        auto boundRect = text->boundingRect();

        if(i == 0){
            boundHeight = boundRect.height() * (1 + 0.5);
            offsetXPos = 0.2 * boundHeight;
            startXPos = 0.6 * boundHeight;

            upperLineYPos = startXPos + number->boundingRect().height();
            lowerLineYPos = upperLineYPos + number->boundingRect().height() * (1 + 0.1);
        }

        number->setFont(_numberFont);
        number->setPos(lastPos.x() + offsetXPos, startXPos);

        sum->setFont(_numberFont);
        sum->setPos(lastPos.x() + offsetXPos, upperLineYPos + number->boundingRect().height() * 0.2);

        lastPos += QPointF(boundHeight, 0);

        blueprint->addToGroup(number);
        blueprint->addToGroup(sum);
        blueprint->addToGroup(text);
    }

    auto upperLine = new QGraphicsLineItem;
    auto lowerLine = new QGraphicsLineItem;

    upperLine->setLine(0, upperLineYPos, lastPos.x(), upperLineYPos);
    blueprint->addToGroup(upperLine);

    lowerLine->setLine(0, lowerLineYPos, lastPos.x(), lowerLineYPos);
    blueprint->addToGroup(lowerLine);

    return std::make_tuple(blueprint, boundHeight, lowerLineYPos);
}
QGraphicsItemGroup * researchRemedRender::sumRemeds(){
    QMap<QString, std::pair<QVector<quint8>, int>> remeds;
    //<name treatment, pair<QVector<intensity>, num> (num = count of coincidences)>
    auto numrep = 0, repSize = 0;

    for(auto it = 0; it != _clipboards.size(); ++it){
        if(!_showClipboadrs.at(it))
            continue;

        repSize += _clipboards.at(it).size();
    }

    for(auto it = 0; it != _clipboards.size(); ++it){
        if(!_showClipboadrs.at(it))
            continue;

        for(const auto & iter : _clipboards.at(it)){
            if(iter.intensity == 0)
                continue;

            quint16 prevRemed = 0;
            openCtree data(iter.path.filePath("symptom").toStdString());
            data.at(iter.key.toStdString(), false);

            func::repertoryData repdata(data);
            const auto rem = repdata.remedsList();

            for(const auto & itrem : rem){
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

                    if(iter.remFilter == (quint16)-1 || (std::get<3>(itrem) & iter.remFilter) != 0)
                        remeds[remedName].second += iter.intensity;

                } catch(...) { qDebug() << "error value" << std::get<0>(itrem); }

                prevRemed = std::get<0>(itrem);
            }

            ++numrep;
        }
    }

    QVector<std::tuple<QString, QVector<quint8>, int, int>> remed;

    for(auto & it : remeds.keys()){
        const auto val = remeds.value(it);
        remed.append(std::make_tuple(it, val.first, val.second, 0));
    }

    remeds.clear();

    auto sort = [](const auto & front, const auto & end){
        return std::get<2>(front) > std::get<2>(end);
    };

    std::sort(remed.begin(), remed.end(), sort);

    auto blueprint = new QGraphicsItemGroup;
    blueprint->setHandlesChildEvents(false);
    blueprint->setFlag(QGraphicsItem::ItemHasNoContents);

    auto digits = drawOneDigitsRemeds(remed);
    auto rmd = drawRemeds(remed, std::get<1>(digits));

    const auto heightLine = std::get<2>(digits) + rmd->boundingRect().height() + _clipboardHeight;

    auto lines = drawLines(std::get<1>(digits),
                           (heightLine > _windowSize.height())
                                ? heightLine : _windowSize.height(),
                           remed.size());

    std::get<0>(digits)->setPos(0, 0);

    lines->setPos(0, 0);
    lines->setZValue(-1);

    rmd->setPos(0, std::get<2>(digits) + _clipboardHeight);

    blueprint->addToGroup(std::get<0>(digits));
    blueprint->addToGroup(lines);
    blueprint->addToGroup(rmd);

    return blueprint;
}
QGraphicsItemGroup * researchRemedRender::drawRemeds(const QVector<std::tuple<QString, QVector<quint8>, int, int>> remeds, qreal lineWidth){
    auto blueprint = new QGraphicsItemGroup;
    int pos = 0;
    QPointF lastPos;
    const QRectF rectDownPtr(0, 0, lineWidth * (1 - 0.2), _symptomHeight * (1 - 0.2));

    blueprint->setHandlesChildEvents(false);
    blueprint->setFlag(QGraphicsItem::ItemHasNoContents);

    for(auto i = 0; i != _clipboards.size(); ++i){
        if(!_showClipboadrs.at(i))
            continue;

        for(auto it = 0; it != _clipboards.at(i).size(); ++it){
            for(const auto & rem : remeds){
                const QColor remedColors[] = {Qt::white, Qt::darkGreen, Qt::blue, Qt::red, Qt::darkRed};
                auto rRect = new QGraphicsPolygonItem;
                QPainterPath pol;

                pol.addRoundedRect(rectDownPtr, 5, 5);
                pol.translate(lastPos.x() + 0.1 * lineWidth, lastPos.y());
                rRect->setPolygon(pol.toFillPolygon());
                rRect->setBrush(QBrush(remedColors[std::get<1>(rem).at(pos)]));
                blueprint->addToGroup(rRect);

                lastPos += QPointF(lineWidth, 0);
            }

            ++pos;
            lastPos.setX(0);

            if(i != _clipboards.size() - 1)
                lastPos += QPointF(0, _symptomHeight);
        }

        lastPos += QPointF(0, _clipboardHeight);
    }

    return blueprint;
}
