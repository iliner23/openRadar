#ifndef REPERTORYRENDER_H
#define REPERTORYRENDER_H

#include <QtWidgets>
#include "repertorydata.h"
#include "cache.h"

class repertoryRender
{
public:
    enum class repertoryFilterType : quint8 {
        none = 0,
        chapter = 1,
        links = 2,
        remeds = 4,
        all = 7
    };
    enum class repertorySortRemedsType : quint8 {
        none,
        more,
        less
    };
    enum class repertoryLabelType : quint8 {
       none = 0,
       original = 1,
       localize = 2,
       all = 3
    };

    repertoryRender() { initFonts(); }
    repertoryRender(const QDir filename, const std::shared_ptr<cache> & cache,
                    QTextCodec * codec = QTextCodec::codecForName("system"));
    virtual void reset(const QDir filename, const std::shared_ptr<cache> & cache,
                       QTextCodec * codec = QTextCodec::codecForName("system"));

    virtual QVector<QGraphicsItemGroup*> render(const QSize resolution);

    void setCurrentPosition(int pos);
    void setCurrentKey(const QByteArray key);

    QString renderingLabel(const bool pass = false);//from current key

    QByteArray currentKey() const noexcept { return _index; }
    int currentPosition() { return _symptom.currentPosition(); }

    void setFilterElements(quint8 filter) noexcept { _filterType = filter; }
    quint8 filterElements() const noexcept { return _filterType; }

    void setChaptersFilter(quint16 filter) noexcept { _remFilter = filter; }
    quint16 chaptersFilter() const noexcept { return _remFilter; }

    void setSortRemed(repertorySortRemedsType sort){ _sortRemedType = sort; }
    repertorySortRemedsType sortRemed() const noexcept { return _sortRemedType; }

    void setShowingLabelTypes(repertoryLabelType types) { _labelTypes = types; }
    repertoryLabelType showingLabelTypes() const noexcept { return _labelTypes; }

    void setReverseLabels(bool reverse) { _reverseLabels = reverse; }
    bool reverseLabels() const noexcept { return _reverseLabels; }

    void setCounter(bool counter) { _counter = counter; }
    bool counter() const { return _counter; }

    QVector<QGraphicsItem*> navigationItems() const { return _navigation; }
    //for -> in repertory window

    openCtree symptomFile() const { return _symptom; }

    qint32 chapterSize() const { return _symptom.size(); }
protected:
    QTextCodec * _codec;
    std::shared_ptr<cache> _cache;
    QVector<QGraphicsItem*> _navigation;
    QByteArray _index, _endIndex;

    openCtree _symptom;

    quint16 _remFilter = -1;//repertory filter in widget menu
    quint8 _filterType = (quint8) repertoryFilterType::all;
    repertorySortRemedsType _sortRemedType = repertorySortRemedsType::none;
    repertoryLabelType _labelTypes = repertoryLabelType::all;
    bool _reverseLabels = false;
    bool _counter = true;
    QSize _resolution;

    struct{
        QString fontName;
        QFont italicFont,
              commonFont,
              defaultFont,
              labelChapterFont,
              smallFont,
              boldFont;
    } _fonts;

    void initFonts();
    inline QGraphicsItemGroup * subRender(openCtree data);
    inline void authorsSym(const QString & autr,
               const quint16 author, QGraphicsItemGroup * allrm,
               const bool next = false);
    inline void addRemeds(QGraphicsItemGroup *group, QPointF pos,
                          QVector<QVector<QGraphicsItemGroup*>> & array,
                          int labelWidth) const;
};

#endif // REPERTORYRENDER_H
