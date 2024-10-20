#ifndef REMEDIESSORTCLASS_H
#define REMEDIESSORTCLASS_H

#include <QtWidgets>
#include "repertorydata.h"

class remediesSortClass
{
    using sortType = QVector<std::tuple<QString, QVector<quint8>, int, int>>;
    using rci = func::remedClipboardInfo;

public:
    remediesSortClass() = default;
    remediesSortClass(const std::shared_ptr<func::cache> & cache)
                                                { _cache = cache; }

    sortType sumDegreesBySortRemedies();
    sortType sumRemediesBySortDegrees();
    sortType sumDegreesAndRemedies();
    sortType sumDegrees();
    sortType sumRemedies();

    void setConsideIntencity(bool intencity) { _consideIntencity = intencity; }
    bool consideIntencity() const noexcept { return _consideIntencity; }

    void setShowedClipboards(std::array<bool, 10> clip)
                                        { _showClipboadrs = clip; }
    std::array<bool, 10> showedClipboards() const
                                        { return _showClipboadrs; }

    void setClipboards(std::array<QVector<rci>, 10> clipboards)
                                    { _clipboards = clipboards; }
    std::array<QVector<rci>, 10> clipboards() const
                                    { return _clipboards; }

    void setCache(const std::shared_ptr<func::cache> & cache)
                                                { _cache = cache; }
private:
    QVector<std::tuple<QString, QVector<quint8>, int, int>>
        sortFunction(std::function<void(
            QMap<QString, std::tuple<QVector<quint8>, int, int>> &, QString, const func::remedClipboardInfo &,
            std::tuple<quint16, quint8, quint16, quint16>)>);

    //Return value
    //QString - remedy name, QVector - intensity remedy for symptom
    //int - primal sort, int - secondary sort
    //if the 4th argument is null then the 3th - secondary sort
    //and remedy's name is primal sorting tag

    bool _consideIntencity = true;
    //false value is ignoring remedies intencity in computing

    std::array<bool, 10> _showClipboadrs;
    std::array<QVector<rci>, 10> _clipboards;
    std::shared_ptr<func::cache> _cache;
};

#endif // REMEDIESSORTCLASS_H
