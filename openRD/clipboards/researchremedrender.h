#ifndef RESEARCHREMEDRENDER_H
#define RESEARCHREMEDRENDER_H

#include <QtWidgets>
#include <algorithm>
#include <cmath>
#include "remediessortclass.h"
#include "repertorydata.h"

class researchRemedRender
{
public:
    using rci = func::remedClipboardInfo;
    using sortRemedies = QVector<std::tuple<QString, QVector<quint8>, int, int>>;

    enum class showType{ waffle, grid, clipboard, strategy, result };
    enum class strategy{ sumRemediesAndDegrees, sumRemedies, sumDegrees,
                         sumRemediesBySortDegrees, sumDegreesBySortRemedies };

    researchRemedRender() { initFont(); }
    researchRemedRender(const std::shared_ptr<func::cache> & cache)
                            { _cache = cache; _sort.setCache(cache); initFont(); }
    void setCache(const std::shared_ptr<func::cache> & cache)
                            { _cache = cache; _sort.setCache(cache); }

    virtual QGraphicsItemGroup * renderRemedies(const QSize, bool partlyRendering);
    virtual QGraphicsItemGroup * renderLabels();

    void setClipboards(std::array<QVector<rci>, 10> clipboards)
                                    { _sort.setClipboards(clipboards); }
    std::array<QVector<rci>, 10> clipboards() const
                                    { return _sort.clipboards(); }

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
                                        { _sort.setShowedClipboards(clip); }
    std::array<bool, 10> showedClipboards() const
                                        { return _sort.showedClipboards(); }

    void setConsideIntencity(bool intencity)
                            { _sort.setConsideIntencity(intencity); }
    bool consideIntencity() const noexcept
                            { return _sort.consideIntencity(); }

    std::pair<qsizetype, QList<bool>> clipboardAndRemediesNumbers() const;
    //Return values: first - numbers of remedies
    //               second - array where bool value: true - spacer between clipboards
    //                                                false - it's symptom
    //If function renderRemedies newer called, will be called exception

    QGraphicsItemGroup * symptomsHCounter();
    //Return numbers in QGraphicsItemGroup for horizontal mode

    //int labelHeight() const { return _labelHeight; }
protected:
    showType _showType = showType::waffle;
    strategy _strategy = strategy::sumRemedies;
    Qt::Orientation _ori = Qt::Vertical;
    quint32 _symptomHeight = 0;
    quint32 _clipboardHeight = 0;
    std::shared_ptr<func::cache> _cache;
    QSize _windowSize;
    QFont _defFont, _numberFont;
    sortRemedies _oldSortRemedies;
    std::pair<qsizetype, QList<bool>> _crNumbers;
    QGraphicsItemGroup * _symptomsHCounter = nullptr;

    void initFont();
    QGraphicsItemGroup * waffleRender(bool partlyRendering);
    QGraphicsItemGroup * drawLines(qreal, qreal, int);
    QGraphicsItemGroup * drawRemeds(const QVector<std::tuple<QString, QVector<quint8>, int, int>>, qreal);
    QGraphicsItemGroup * drawDigitsRemeds(const QVector<std::tuple<QString, QVector<quint8>, int, int>>, bool twoDigits = false);

    remediesSortClass _sort;
    //Class for sorting remedies and clipboadrs data
};

#endif // RESEARCHREMEDRENDER_H
