#include "researchremedrender.h"

void researchRemedRender::setSymptomHeight(quint32 height){
    if(height == 0)
        throw std::logic_error("incorrect value");

    _symptomHeight = height;
}

QGraphicsItemGroup * researchRemedRender::render(const QSize windowSize){
    switch (_showType) {
    case showType::waffle :
        return waffleRender();
    default :
        return nullptr;
    }
}
QGraphicsItemGroup * researchRemedRender::waffleRender(){
    //test code
    auto * blueprint = new QGraphicsItemGroup;
    QMap<QString, quint32> remeds;
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

                    remeds[remedName] = remeds.value(remedName, 0) + iter.intensity;
                } catch(...) { qDebug() << "error value" << std::get<0>(itrem); }

                prevRemed = std::get<0>(itrem);
            }
        }
    }

    QList<QPair<QString, quint32>> remed;

    for(auto & it : remeds.keys()){
        remed.append(std::pair(it, remeds.value(it)));
        remeds.remove(it);
    }

    auto sort = [](const auto & front, const auto & end){
        return front.second > end.second;
    };

    std::sort(remed.begin(), remed.end(), sort);

    return blueprint;
}
