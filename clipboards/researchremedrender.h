#ifndef RESEARCHREMEDRENDER_H
#define RESEARCHREMEDRENDER_H

#include <QtWidgets>
#include <algorithm>
#include <cmath>
#include "commonfunction/repertorydata.h"

class researchRemedRender
{
public:
    using rci = func::remedClipboardInfo;
    enum class showType{ waffle, grid, clipboard, strategy, result };
    enum class strategy{ sumRemediesAndDegrees, sumRemedies, sumDegrees,
                         sumRemediesBySortDegrees, sumDegreesBySortRemedies };

    researchRemedRender() { initFont(); }
    researchRemedRender(const std::shared_ptr<func::cache> & cache)
                                                { _cache = cache; initFont(); }
    virtual void reset(const std::shared_ptr<func::cache> & cache)
                                                { _cache = cache; }
    virtual QGraphicsItemGroup * render(const QSize);

    void setClipboards(std::array<QVector<rci>, 10> clipboards)
                                    { _clipboards = clipboards; }
    std::array<QVector<rci>, 10> clipboards() const
                                    { return _clipboards; }

    void setSymptomHeight(quint32);
    int symptomHeight() const { return _symptomHeight; }

    void setClipboardHeight(quint32 height)
                            { _clipboardHeight = height; }
    int clipboardHeight() const { return _clipboardHeight; }

    void setOrientation(Qt::Orientation orientation)
                                { _ori = orientation; }
    Qt::Orientation orientation() { return _ori; }

    void setAnalysisType(showType analysis)
                                { _showType = analysis; }
    showType analysisType() const { return _showType; }

    void setStrategyType(strategy strat) { _strategy = strat; }
    strategy strategyType() const { return _strategy; }

    void setShowedClipboards(std::array<bool, 10> clip)
                                        { _showClipboadrs = clip; }
    std::array<bool, 10> showedClipboards() const
                                        { return _showClipboadrs; }

    void setConsideIntencity(bool intencity) { _consideIntencity = intencity; }
    bool consideIntencity() const noexcept { return _consideIntencity; }

    int labelHeight() const { return _labelHeight; }
protected:
    showType _showType = showType::waffle;
    strategy _strategy = strategy::sumRemedies;
    Qt::Orientation _ori = Qt::Vertical;
    quint32 _symptomHeight = 0;
    quint32 _clipboardHeight = 0;

    std::array<bool, 10> _showClipboadrs;
    std::array<QVector<rci>, 10> _clipboards;
    std::shared_ptr<func::cache> _cache;
    QSize _windowSize;

    QFont _defFont, _numberFont;
    int _labelHeight = 0;

    void initFont();
    QGraphicsItemGroup * waffleRender();
    QGraphicsItemGroup * drawLines(qreal, qreal, int);
    QGraphicsItemGroup * drawRemeds(const QVector<std::tuple<QString, QVector<quint8>, int, int>>, qreal);

    QVector<std::tuple<QString, QVector<quint8>, int, int>> sumRemedies();
    QVector<std::tuple<QString, QVector<quint8>, int, int>> sumDegrees();
    QVector<std::tuple<QString, QVector<quint8>, int, int>> sumDegreesAndRemedies();
    QVector<std::tuple<QString, QVector<quint8>, int, int>> sumRemediesBySortDegrees();
    QVector<std::tuple<QString, QVector<quint8>, int, int>> sumDegreesBySortRemedies();
    QVector<std::tuple<QString, QVector<quint8>, int, int>> sortFunction(
            std::function<void(QMap<QString, std::tuple<QVector<quint8>, int, int>> &, QString, const func::remedClipboardInfo &,
                                                                                          std::tuple<quint16, quint8, quint16, quint16>)>);

    //Return value
    //QString - remed name, QVector - intensity remedy for symptom
    //int - primal sort, int - secondary sort
    //if the 4th argument is null then the 3th - secondary sort
    //and remedy's name is primal sorting tag

    std::tuple<QGraphicsItemGroup*, qreal, qreal> drawDigitsRemeds(const QVector<std::tuple<QString, QVector<quint8>, int, int>>, bool twoDigits = false);
    //1 - width line and remedy, 2 - height until 2nd horizon line

    bool _consideIntencity = true;
    //false value is ignoring remedies intencity in computing
};

#endif // RESEARCHREMEDRENDER_H
