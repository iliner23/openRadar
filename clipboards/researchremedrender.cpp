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
QGraphicsItemGroup * researchRemedRender::renderRemedies(const QSize windowSize, bool partlyRendering){
    _windowSize = windowSize;

    switch (_showType) {
    case showType::waffle :
        return waffleRender(partlyRendering);
    default :
        return nullptr;
    }
}
QGraphicsItemGroup * researchRemedRender::renderLabels(){
    switch(_strategy){
        case strategy::sumRemediesAndDegrees : {
            const auto remed = _sort.sumDegreesAndRemedies();//sumDegreesAndRemedies();
            return drawDigitsRemeds(remed);
        }
        case strategy::sumRemedies : {
            const auto remed = _sort.sumRemedies(); //sumRemedies();
            return drawDigitsRemeds(remed);
        }
        case strategy::sumDegrees : {
            const auto remed = _sort.sumDegrees(); //sumDegrees();
            return drawDigitsRemeds(remed);
        }
        case strategy::sumRemediesBySortDegrees : {
            const auto remed = _sort.sumRemediesBySortDegrees();// sumRemediesBySortDegrees();
            return drawDigitsRemeds(remed, true);
        }
        case strategy::sumDegreesBySortRemedies : {
            const auto remed = _sort.sumDegreesBySortRemedies();// sumDegreesBySortRemedies();
            return drawDigitsRemeds(remed, true);
        }
    }
}
QGraphicsItemGroup * researchRemedRender::waffleRender(bool partlyRendering){
    auto rendering = [&](const auto remed) -> QGraphicsItemGroup * {
        auto blueprint = new QGraphicsItemGroup;

        blueprint->setHandlesChildEvents(false);
        blueprint->setFlag(QGraphicsItem::ItemHasNoContents);

        if(remed.isEmpty())
            return nullptr;

        const qreal widthLine = QFontMetrics(_defFont).height() * (1 + 0.5);
        auto rmd = drawRemeds(remed, widthLine);

        const auto heightLine = rmd->boundingRect().height() + _clipboardHeight;
        auto lines = drawLines(widthLine, heightLine, remed.size());

        lines->setZValue(-1);
        rmd->setPos(-rmd->boundingRect().x(), 0);
        lines->setX(-rmd->boundingRect().x());

        blueprint->addToGroup(lines);
        blueprint->addToGroup(rmd);

        return blueprint;
    };

    sortRemedies remed;

    if(partlyRendering && !_oldSortRemedies.isEmpty())
        return rendering(_oldSortRemedies);

    switch(_strategy){
        case strategy::sumRemediesAndDegrees : {
            remed = _sort.sumDegreesAndRemedies();//sumDegreesAndRemedies();
            break;
        }
        case strategy::sumRemedies : {
            remed = _sort.sumRemedies(); //sumRemedies();
            break;
        }
        case strategy::sumDegrees : {
            remed = _sort.sumDegrees(); //sumDegrees();
            break;
        }
        case strategy::sumRemediesBySortDegrees : {
            remed = _sort.sumRemediesBySortDegrees();// sumRemediesBySortDegrees();
            break;
        }
        case strategy::sumDegreesBySortRemedies : {
            remed = _sort.sumDegreesBySortRemedies();// sumDegreesBySortRemedies();
            break;
        }
    }

    _oldSortRemedies = remed;

    if(!_oldSortRemedies.isEmpty()){
        const auto clipbd = _sort.clipboards();
        const auto showClip = _sort.showedClipboards();

        QList<bool> value;

        for(qsizetype i = 0; i != clipbd.size(); ++i){
            if(!showClip.at(i))
                continue;

            QList<bool> tempVal;

            for(auto it = 0; it != clipbd.at(i).size(); ++it)
                tempVal.append(false);

            if((!tempVal.isEmpty()) && i != clipbd.size() - 1)
                value.append(true);

            value.append(tempVal);
        }

        _crNumbers = std::make_pair(_oldSortRemedies.size(), value);
    }
    else
        _crNumbers = std::make_pair(0, QList<bool>());

    return rendering(remed);
}
std::pair<qsizetype, QList<bool>> researchRemedRender::clipboardAndRemediesNumbers() const{
    if(_crNumbers.second.isEmpty())
        throw std::logic_error("don't called render");

    return _crNumbers;
}
QGraphicsItemGroup * researchRemedRender::symptomsHCounter(){
    if(_symptomsHCounter == nullptr)
        throw std::logic_error("don't called render");

    auto temp = _symptomsHCounter;
    _symptomsHCounter = nullptr;

    return temp;
}
QGraphicsItemGroup * researchRemedRender::drawLines(qreal lineWidth, qreal lengthText, int countLines){
    auto * lines = new QGraphicsItemGroup;
    lines->setHandlesChildEvents(false);
    lines->setFlag(QGraphicsItem::ItemHasNoContents);

    QPointF lastPos(-lineWidth, 0);
    lengthText += _windowSize.height();

    for(auto i = 0; i != countLines; ++i){
        lastPos += QPointF(lineWidth, 0);

        if(i % 2 != 0)
            continue;

        auto line = new QGraphicsPolygonItem;
        QPolygonF polygon;

        polygon << QPointF(lastPos.x(), 0)
                << QPointF(lastPos.x(), lengthText)
                << QPointF(lastPos.x() + lineWidth, lengthText)
                << QPointF(lastPos.x() + lineWidth, 0);

        line->setPolygon(polygon);
        line->setPen(QPen(Qt::yellow));
        line->setBrush(QBrush(Qt::yellow));
        lines->addToGroup(line);
    }

    return lines;
}
QGraphicsItemGroup * researchRemedRender::drawDigitsRemeds(const QVector<std::tuple<QString, QVector<quint8>, int, int>> remeds, bool twoDigits){
    auto blueprint = new QGraphicsItemGroup;
    blueprint->setHandlesChildEvents(false);
    blueprint->setFlag(QGraphicsItem::ItemHasNoContents);
    //twoDigits = true if using for sorting by 2nd sign

    const QFontMetrics defFontMetr(_defFont);
    const qreal boundHeight = defFontMetr.height() * (1 + 0.5);
    qreal upperLineYPos = 0, lowerLineYPos = 0;
    qreal midLineYPos = 0;
    qreal startXPos = 0, offsetXPos = 0;
    QPointF lastPos;

    for(auto i = 0; i != remeds.size(); ++i){
        auto text = new QGraphicsSimpleTextItem(std::get<0>(remeds.at(i)));//remedy text
        auto sum = new QGraphicsSimpleTextItem(QString::number(std::get<2>(remeds.at(i))));//main sort sign
        auto sort = (twoDigits) ? new QGraphicsSimpleTextItem(QString::number(std::get<3>(remeds.at(i)))) : nullptr;//for 2nd digits (sort sign)
        auto number = new QGraphicsSimpleTextItem(QString::number(i + 1));//number of remedy

        text->setFont(_defFont);
        text->setPos(lastPos);
        text->setRotation(-45.0);

        if(i == 0){
            offsetXPos = 0.2 * boundHeight;
            startXPos = 0.6 * boundHeight;

            upperLineYPos = startXPos + number->boundingRect().height();

            if(!twoDigits)
                lowerLineYPos = upperLineYPos + number->boundingRect().height() * (1.1);
            else{
                midLineYPos = upperLineYPos + number->boundingRect().height() * (1.1);
                lowerLineYPos = midLineYPos + sum->boundingRect().height() * (1.1);
            }
        }

        number->setFont(_numberFont);
        number->setPos(lastPos.x() + offsetXPos, startXPos);

        sum->setFont(_numberFont);
        sum->setPos(lastPos.x() + offsetXPos, upperLineYPos + number->boundingRect().height() * 0.2);

        if(twoDigits)
            sort->setPos(lastPos.x() + offsetXPos, midLineYPos);

        lastPos += QPointF(boundHeight, 0);

        blueprint->addToGroup(number);
        blueprint->addToGroup(sum);
        blueprint->addToGroup(text);

        if(twoDigits)
            blueprint->addToGroup(sort);
    }

    auto upperLine = new QGraphicsLineItem;
    auto lowerLine = new QGraphicsLineItem;

    upperLine->setLine(0, upperLineYPos, lastPos.x(), upperLineYPos);
    blueprint->addToGroup(upperLine);

    lowerLine->setLine(0, lowerLineYPos, lastPos.x(), lowerLineYPos);
    blueprint->addToGroup(lowerLine);

    auto blue = new QGraphicsItemGroup;
    blue->setHandlesChildEvents(false);
    blue->setFlag(QGraphicsItem::ItemHasNoContents);

    QPointF pos(-boundHeight, 0);
    const auto rad = std::acos(-1.0) / 180.0;
    const auto oblique = std::sin(-45.0 * rad) * boundHeight * 3.5;

    for(auto i = 0; i != remeds.size(); ++i){
        pos += QPointF(boundHeight, 0);

        if(i % 2 != 0)
            continue;

        auto line = new QGraphicsPolygonItem;
        QPolygonF polygon;

        polygon << QPointF(pos.x() + std::cos(45.0 * rad) * boundHeight * 3.5, oblique)
                << QPointF(pos.x(), 0)
                << QPointF(pos.x(), lowerLineYPos)
                << QPointF(pos.x() + boundHeight, lowerLineYPos)
                << QPointF(pos.x() + boundHeight, 0)
                << QPointF(boundHeight + pos.x() + std::cos(45.0 * rad) * boundHeight * 3.5, oblique);

        line->setPolygon(polygon);
        line->setPen(QPen(Qt::yellow));
        line->setBrush(QBrush(Qt::yellow));
        blue->addToGroup(line);
    }

    blue->setZValue(-1);
    blueprint->addToGroup(blue);

    const auto brect = blueprint->boundingRect();
    blueprint->setPos(-brect.x(), -brect.y());
    return blueprint;
}
QGraphicsItemGroup * researchRemedRender::drawRemeds(const QVector<std::tuple<QString, QVector<quint8>, int, int>> remeds, qreal lineWidth){
    auto blueprint = new QGraphicsItemGroup;
    _symptomsHCounter = new QGraphicsItemGroup;

    int pos = 0;
    QFontMetrics numMetric(_defFont);
    const QRectF rectDownPtr(0, _symptomHeight * 0.1, lineWidth * (1 - 0.2), _symptomHeight * (1 - 0.2));

    QPointF lastPos(0, _clipboardHeight);
    const auto clipbd = _sort.clipboards();
    const auto showClip = _sort.showedClipboards();

    _symptomsHCounter->setHandlesChildEvents(false);
    _symptomsHCounter->setFlag(QGraphicsItem::ItemHasNoContents);

    blueprint->setHandlesChildEvents(false);
    blueprint->setFlag(QGraphicsItem::ItemHasNoContents);

    for(auto i = 0; i != clipbd.size(); ++i){
        if(!showClip.at(i))
            continue;

        for(auto it = 0; it != clipbd.at(i).size(); ++it){
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

            if(_ori == Qt::Horizontal && !remeds.isEmpty()){
                auto counter = new QGraphicsSimpleTextItem(QString::number(pos));
                counter->setFont(_defFont);
                counter->setPos(0, lastPos.y() + rectDownPtr.height() / 2);
                _symptomsHCounter->addToGroup(counter);
            }

            if(i != clipbd.size() - 1)
                lastPos += QPointF(0, _symptomHeight);
        }

        if(_ori == Qt::Vertical)
            lastPos += QPointF(0, _clipboardHeight);
    }

    return blueprint;
}
