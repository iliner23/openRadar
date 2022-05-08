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
    auto rendering = [&](const auto remed, bool twoDigits = false){
        auto blueprint = new QGraphicsItemGroup;
        blueprint->setHandlesChildEvents(false);
        blueprint->setFlag(QGraphicsItem::ItemHasNoContents);
        const auto oriClipHeight = (_ori == Qt::Vertical) ? _clipboardHeight : 0;

        auto digits = drawDigitsRemeds(remed, twoDigits);
        auto rmd = drawRemeds(remed, std::get<1>(digits));

        const auto heightLine = std::get<2>(digits) + rmd->boundingRect().height() + oriClipHeight;
        auto lines = drawLines(std::get<1>(digits), heightLine, remed.size());

        std::get<0>(digits)->setPos(0, 0);

        lines->setZValue(-1);
        rmd->setPos(-rmd->boundingRect().x(), std::get<2>(digits) + oriClipHeight);
        lines->setX(-rmd->boundingRect().x());

        if(std::get<0>(digits) != nullptr)
            std::get<0>(digits)->setX(-rmd->boundingRect().x());

        blueprint->addToGroup(std::get<0>(digits));
        blueprint->addToGroup(lines);
        blueprint->addToGroup(rmd);

        _labelHeight = -blueprint->boundingRect().y() + std::get<2>(digits);
        blueprint->setPos(0, -blueprint->boundingRect().y());

        return blueprint;
    };

    switch(_strategy){
        case strategy::sumRemediesAndDegrees : {
            const auto remed = sumDegreesAndRemedies();
            return rendering(remed);
        }
        case strategy::sumRemedies : {
            const auto remed = sumRemedies();
            return rendering(remed);
        }
        case strategy::sumDegrees : {
            const auto remed = sumDegrees();
            return rendering(remed);
        }
        case strategy::sumRemediesBySortDegrees : {
            const auto remed = sumRemediesBySortDegrees();
            return rendering(remed, true);
        }
        case strategy::sumDegreesBySortRemedies : {
            const auto remed = sumDegreesBySortRemedies();
            return rendering(remed, true);
        }
    }
}
QGraphicsItemGroup * researchRemedRender::drawLines(qreal lineWidth, qreal lengthText, int countLines){
    auto * lines = new QGraphicsItemGroup;
    lines->setHandlesChildEvents(false);
    lines->setFlag(QGraphicsItem::ItemHasNoContents);

    QPointF lastPos(-lineWidth, 0);
    const auto rad = std::acos(-1.0) / 180.0;
    const auto oblique = std::sin(-45.0 * rad) * lineWidth * 3.5;

    lengthText = (lengthText > _windowSize.height() + oblique) ? lengthText : _windowSize.height() + oblique;

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
                << QPointF(lineWidth + lastPos.x() + std::cos(45.0 * rad) * lineWidth * 3.5, oblique)
                << QPointF(lastPos.x() + std::cosf(45.0 * rad) * lineWidth * 3.5, oblique);

        line->setPolygon(polygon);
        line->setPen(QPen(Qt::yellow));
        line->setBrush(QBrush(Qt::yellow));
        lines->addToGroup(line);
    }

    return lines;
}
std::tuple<QGraphicsItemGroup*, qreal, qreal> researchRemedRender::drawDigitsRemeds(const QVector<std::tuple<QString, QVector<quint8>, int, int>> remeds, bool twoDigits){
    auto blueprint = new QGraphicsItemGroup;
    blueprint->setHandlesChildEvents(false);
    blueprint->setFlag(QGraphicsItem::ItemHasNoContents);
    //twoDigits = true if using for sorting by 2nd sign

    qreal boundHeight = 0;
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
        auto boundRect = text->boundingRect();

        if(i == 0){
            boundHeight = boundRect.height() * (1 + 0.5);
            offsetXPos = 0.2 * boundHeight;
            startXPos = 0.6 * boundHeight;

            upperLineYPos = startXPos + number->boundingRect().height();

            if(!twoDigits)
                lowerLineYPos = upperLineYPos + number->boundingRect().height() * (1 + 0.1);
            else{
                midLineYPos = upperLineYPos + number->boundingRect().height() * (1 + 0.1);
                lowerLineYPos = midLineYPos + sum->boundingRect().height() * (1 + 0.1);
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

    return std::make_tuple(blueprint, boundHeight, lowerLineYPos);
}
QVector<std::tuple<QString, QVector<quint8>, int, int>> researchRemedRender::sortFunction(std::function<void(QMap<QString, std::tuple<QVector<quint8>, int, int>> &, QString, const func::remedClipboardInfo &,std::tuple<quint16, quint8, quint16, quint16>)> compute){
    QMap<QString, std::tuple<QVector<quint8>, int, int>> remeds;
    QSet<QString> notIgnoreRemedies;
    //<name treatment, pair<QVector<intensity>, num> (num = count of coincidences)>
    auto numrep = 0, repSize = 0;
    QSet<size_t> elimCount;

    for(auto it = 0; it != _clipboards.size(); ++it){
        if(!_showClipboadrs.at(it))
            continue;

        repSize += _clipboards.at(it).size();
    }

    for(auto it = 0; it != _clipboards.size(); ++it){
        if(!_showClipboadrs.at(it))
            continue;

        const auto & clip = _clipboards.at(it);

        for(auto iter = 0; iter != clip.size(); ++iter){
            if(clip.at(iter).intensity == 0 && _consideIntencity)
                continue;

            if(clip.at(iter).elim)
                elimCount += iter + ((it > 0) ? _clipboards.at(it - 1).size() : 0);

            quint16 prevRemed = 0;
            openCtree data(clip.at(iter).path.filePath("symptom").toStdString());
            data.at(clip.at(iter).key.toStdString(), false);

            func::repertoryData repdata(data);
            const auto rem = repdata.remedsList();

            for(const auto & itrem : rem){
                if(prevRemed == std::get<0>(itrem))
                    continue;

                try{
                    const auto & rmStr = _cache->_cacheRemed.at(std::get<0>(itrem));
                    auto itN = rmStr.find('\0', _cache->_lenRem);
                    auto remedName = QString::fromStdString(rmStr.substr(_cache->_lenRem, itN - _cache->_lenRem));

                    const auto degree = std::get<1>(itrem);

                    if((degree == 1 && clip.at(iter).measure[0]) || (degree == 2 && clip.at(iter).measure[1])
                            || (degree == 3 && clip.at(iter).measure[2]) || (degree == 4 && clip.at(iter).measure[3])){

                        if(!remeds.contains(remedName)){
                            std::get<0>(remeds[remedName]).resize(repSize);
                            std::get<1>(remeds[remedName]) = 0;
                        }

                        std::get<0>(remeds[remedName])[numrep] = degree;

                        if(clip.at(iter).remFilter == (quint16)-1 || (std::get<3>(itrem) & clip.at(iter).remFilter) != 0){
                            compute(remeds, remedName, clip.at(iter), itrem);

                            if(clip.at(iter).elim)
                                notIgnoreRemedies.insert(remedName);
                        }
                    }

                } catch(...) { qDebug() << "error value" << std::get<0>(itrem); }

                prevRemed = std::get<0>(itrem);
            }

            ++numrep;
        }
    }

    QVector<std::tuple<QString, QVector<quint8>, int, int>> remed;

    for(auto iter = remeds.cbegin(); iter != remeds.cend(); ++iter){
        //For eliminate sort
        //TODO: need some test for this part
        const auto ignoreValue = notIgnoreRemedies.contains(iter.key());

        if(!notIgnoreRemedies.isEmpty() && !ignoreValue)
            continue;
        else if (ignoreValue){
            const auto & value = std::get<0>(iter.value());
            bool exit = false;

            for(auto it = 0; it != value.size(); ++it){
                if(value.at(it) == 0 && elimCount.contains(it)){
                    exit = true;
                    break;
                }
            }

            if(exit) continue;
        }

        remed.append(std::make_tuple(iter.key(), std::get<0>(*iter), std::get<1>(*iter), std::get<2>(*iter)));
    }

    remeds.clear();

    auto sort = [](const auto & front, const auto & end){
        return std::tie(std::get<2>(front), std::get<3>(front)) >
               std::tie(std::get<2>(end), std::get<3>(end));
    };

    std::sort(remed.begin(), remed.end(), sort);
    return remed;
}
QVector<std::tuple<QString, QVector<quint8>, int, int>> researchRemedRender::sumDegreesBySortRemedies(){
    auto func = [&](QMap<QString, std::tuple<QVector<quint8>, int, int>> & remeds,
                QString remedName, const func::remedClipboardInfo & iter,
                std::tuple<quint16, quint8, quint16, quint16> itrem){
        std::get<1>(remeds[remedName]) += std::get<1>(itrem) * ((_consideIntencity) ? iter.intensity : 1);
        std::get<2>(remeds[remedName]) += (_consideIntencity) ? iter.intensity : 1;
    };

    return sortFunction(func);
}
QVector<std::tuple<QString, QVector<quint8>, int, int>> researchRemedRender::sumRemediesBySortDegrees(){
    auto func = [&](QMap<QString, std::tuple<QVector<quint8>, int, int>> & remeds,
                QString remedName, const func::remedClipboardInfo & iter,
                std::tuple<quint16, quint8, quint16, quint16> itrem){
        std::get<1>(remeds[remedName]) += (_consideIntencity) ? iter.intensity : 1;
        std::get<2>(remeds[remedName]) += std::get<1>(itrem) * ((_consideIntencity) ? iter.intensity : 1);
    };

    return sortFunction(func);
}
QVector<std::tuple<QString, QVector<quint8>, int, int>> researchRemedRender::sumDegreesAndRemedies(){
    auto func = [&](QMap<QString, std::tuple<QVector<quint8>, int, int>> & remeds,
                QString remedName, const func::remedClipboardInfo & iter,
                std::tuple<quint16, quint8, quint16, quint16> itrem){
        std::get<1>(remeds[remedName]) += (std::get<1>(itrem) + 1) * ((_consideIntencity) ? iter.intensity : 1);
    };

    return sortFunction(func);
}
QVector<std::tuple<QString, QVector<quint8>, int, int>> researchRemedRender::sumDegrees(){
    auto func = [&](QMap<QString, std::tuple<QVector<quint8>, int, int>> & remeds,
                QString remedName, const func::remedClipboardInfo & iter,
                std::tuple<quint16, quint8, quint16, quint16> itrem){
        std::get<1>(remeds[remedName]) += std::get<1>(itrem) * ((_consideIntencity) ? iter.intensity : 1);
    };

    return sortFunction(func);
}
QVector<std::tuple<QString, QVector<quint8>, int, int>> researchRemedRender::sumRemedies(){
    auto func = [&](QMap<QString, std::tuple<QVector<quint8>, int, int>> & remeds,
                    QString remedName, const func::remedClipboardInfo & iter,
                    std::tuple<quint16, quint8, quint16, quint16>){
        std::get<1>(remeds[remedName]) += (_consideIntencity) ? iter.intensity : 1;
    };

    return sortFunction(func);
}
QGraphicsItemGroup * researchRemedRender::drawRemeds(const QVector<std::tuple<QString, QVector<quint8>, int, int>> remeds, qreal lineWidth){
    auto blueprint = new QGraphicsItemGroup;
    int pos = 0;
    QFontMetrics numMetric(_defFont);
    const QRectF rectDownPtr(0, _symptomHeight * 0.1, lineWidth * (1 - 0.2), _symptomHeight * (1 - 0.2));

    const auto maxSymptomsSize = numMetric.horizontalAdvance(QString::number(remeds.empty() ? 0 :
                                    std::get<1>(remeds.at(0)).size()));
    QPointF lastPos;

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

            if(_ori == Qt::Horizontal && !remeds.isEmpty()){
                auto counter = new QGraphicsSimpleTextItem(QString::number(pos));
                counter->setFont(_defFont);
                counter->setPos(-maxSymptomsSize * 2, lastPos.y() + rectDownPtr.height() / 2);
                blueprint->addToGroup(counter);
            }

            if(i != _clipboards.size() - 1)
                lastPos += QPointF(0, _symptomHeight);
        }

        if(_ori == Qt::Vertical)
            lastPos += QPointF(0, _clipboardHeight);
    }

    return blueprint;
}
