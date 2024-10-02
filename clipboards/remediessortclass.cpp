#include "remediessortclass.h"

QVector<std::tuple<QString, QVector<quint8>, int, int>> remediesSortClass::sumDegreesBySortRemedies(){
    auto func = [&](QMap<QString, std::tuple<QVector<quint8>, int, int>> & remeds,
                QString remedName, const func::remedClipboardInfo & iter,
                std::tuple<quint16, quint8, quint16, quint16> itrem){
        std::get<1>(remeds[remedName]) += std::get<1>(itrem) * ((_consideIntencity) ? iter.intensity : 1);
        std::get<2>(remeds[remedName]) += (_consideIntencity) ? iter.intensity : 1;
    };

    return sortFunction(func);
}
QVector<std::tuple<QString, QVector<quint8>, int, int>> remediesSortClass::sumRemediesBySortDegrees(){
    auto func = [&](QMap<QString, std::tuple<QVector<quint8>, int, int>> & remeds,
                QString remedName, const func::remedClipboardInfo & iter,
                std::tuple<quint16, quint8, quint16, quint16> itrem){
        std::get<1>(remeds[remedName]) += (_consideIntencity) ? iter.intensity : 1;
        std::get<2>(remeds[remedName]) += std::get<1>(itrem) * ((_consideIntencity) ? iter.intensity : 1);
    };

    return sortFunction(func);
}
QVector<std::tuple<QString, QVector<quint8>, int, int>> remediesSortClass::sumDegreesAndRemedies(){
    auto func = [&](QMap<QString, std::tuple<QVector<quint8>, int, int>> & remeds,
                QString remedName, const func::remedClipboardInfo & iter,
                std::tuple<quint16, quint8, quint16, quint16> itrem){
        std::get<1>(remeds[remedName]) += (std::get<1>(itrem) + 1) * ((_consideIntencity) ? iter.intensity : 1);
    };

    return sortFunction(func);
}
QVector<std::tuple<QString, QVector<quint8>, int, int>> remediesSortClass::sumDegrees(){
    auto func = [&](QMap<QString, std::tuple<QVector<quint8>, int, int>> & remeds,
                QString remedName, const func::remedClipboardInfo & iter,
                std::tuple<quint16, quint8, quint16, quint16> itrem){
        std::get<1>(remeds[remedName]) += std::get<1>(itrem) * ((_consideIntencity) ? iter.intensity : 1);
    };

    return sortFunction(func);
}
QVector<std::tuple<QString, QVector<quint8>, int, int>> remediesSortClass::sumRemedies(){
    auto func = [&](QMap<QString, std::tuple<QVector<quint8>, int, int>> & remeds,
                    QString remedName, const func::remedClipboardInfo & iter,
                    std::tuple<quint16, quint8, quint16, quint16>){
        std::get<1>(remeds[remedName]) += (_consideIntencity) ? iter.intensity : 1;
    };

    return sortFunction(func);
}
QVector<std::tuple<QString, QVector<quint8>, int, int>> remediesSortClass::sortFunction(std::function<void(QMap<QString, std::tuple<QVector<quint8>, int, int>> &, QString, const func::remedClipboardInfo &,std::tuple<quint16, quint8, quint16, quint16>)> compute){
    if(_cache.get() == nullptr)
        throw std::logic_error("cache null pointer");

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

                        if(clip.at(iter).remFilter != (quint16)-1 && (std::get<3>(itrem) & clip.at(iter).remFilter) == 0)
                            continue;

                        if(!remeds.contains(remedName)){
                            std::get<0>(remeds[remedName]).resize(repSize);
                            std::get<1>(remeds[remedName]) = 0;
                        }

                        std::get<0>(remeds[remedName])[numrep] = degree;

                        compute(remeds, remedName, clip.at(iter), itrem);

                        if(clip.at(iter).elim)
                            notIgnoreRemedies.insert(remedName);
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
