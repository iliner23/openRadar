#include "labelrender.h"

labelRender::labelRender(const QDir filename, const std::shared_ptr<func::cache> & cache, QTextCodec * codec){
    labelRender::reset(filename, cache, codec);
}
void labelRender::reset(const QDir filename, const std::shared_ptr<func::cache> & cache, QTextCodec * codec){
    for(auto & it : _remedsCount)
        it = 0;

    _localize = false;
    _hideLabel = false;

    _synonyms = {"" , ""};

    _links.first.clear();
    _links.second.clear();

    _crossLinks.first.clear();
    _crossLinks.second.clear();

    repertoryRender::reset(filename, cache, codec);

    setSortRemed(repertorySortRemedsType::more);
    setFilterElements((quint8) repertoryFilterType::remeds);
}
QVector<QGraphicsItemGroup*> labelRender::render(const QSize resolution){
    QVector<QGraphicsItemGroup*> endData;
    _navigation.clear();
    _resolution = resolution;

    endData.push_back(subRender(_symptom));

    func::repertoryData dataParser(_symptom, _codec);
    auto rem = dataParser.remedsList();

    quint16 prevRemed = 0;

    _hideLabel = !(_remFilter == (quint16)-1 ||
                       (dataParser.labelIndexFilter() & _remFilter) != 0);

    _synonyms = dataParser.synonymsList();
    _links = dataParser.linksList();
    _crossLinks = dataParser.crossLinksList();

    if(!dataParser.repertoryLabel().second.isEmpty())
        _localize = true;

    for(auto & it : rem){
        if(prevRemed != std::get<0>(it) && ((std::get<3>(it) & _remFilter) != 0
                                            || _remFilter == (quint16)-1))
            _remedsCount.at(std::get<1>(it) - 1) += 1;

        prevRemed = std::get<0>(it);
    }

    return endData;
}
