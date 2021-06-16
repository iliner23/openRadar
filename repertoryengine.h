#ifndef REPERTORYENGINE_H
#define REPERTORYENGINE_H

#include <QtWidgets>
#include <QTextCodec>
#include "openctree.h"
#include "cache.h"

class repertoryEngine : public QObject
{
public:
    enum repertoryFilterType : quint8 {
        none = 0,
        chapter = 1,
        links = 2,
        remeds = 4,
        all = 7
    };

    explicit repertoryEngine(QGraphicsScene *parent = nullptr);
    explicit repertoryEngine(const QDir filename, const std::shared_ptr<cache> & cache
                    ,QGraphicsScene * scene = nullptr
                    ,QTextCodec * codec = QTextCodec::codecForName("system"));

    virtual void reset(const QDir filename, const std::shared_ptr<cache> & cache
               , QTextCodec * codec = QTextCodec::codecForName("system"));

    void setCurrentPosition(int pos);
    void setCurrentKey(const QByteArray key);

    QByteArray currentKey() const noexcept { return _private.index; }
    int currentPosition() { return _symptom.currentPosition(); }

    virtual void render(const int heightView, const int widthView, const bool oneChapter = false);

    int chaptersSize() const { return _symptom.size(); }

    void setFilterElements(repertoryFilterType filter) noexcept { _filterType = filter; }
    repertoryFilterType filterElements() const noexcept { return _filterType; }

    void setSortingRemeds(bool sorting) noexcept { _sorting = sorting; }
    bool IsSortingRemeds() const noexcept { return _sorting; }

    QVector<QGraphicsItem*> navigationItems() const { return _private.labelsVec; }
    void setUsingNavigationItems(bool navigation) noexcept { _navigation = navigation; }
    bool IsUsingNavigationItems() const noexcept { return _navigation; }

    void setChaptersFilter(quint16 filter) noexcept { _private.remFilter = filter; }
    quint16 chaptersFilter() const noexcept { return _private.remFilter; }

    void setRemedsCounter(bool counter) noexcept { _counter = counter; }
    bool IsRemedsCounter() const noexcept { return _counter; }

    QString renderingLabel(const bool passLastChapter = false);//from current key
    QVector<QByteArray> getRootPath() { return getRootPath(_symptom); }
    static QString renderingLabel(openCtree &symptom,
                bool passLastChapter = true, QTextCodec * codec = QTextCodec::codecForName("system"));

    static QVector<QByteArray> getRootPath(openCtree &, quint16 deep = std::numeric_limits<quint16>::max());
protected:
    QTextCodec * _codec = nullptr;

    struct{
        QString fontName;
        QFont italicFont,
              commonFont,
              defaultFont,
              labelChapterFont,
              smallFont,
              boldFont;
    } _fonts;

    //for adding remeds in function sortRemeds
    inline void addRemeds(QGraphicsItem *);

    //this functions may be reinplement in child class
    virtual void processingChapter(const quint64, const quint64) {}
    virtual void processingLinks(const quint8, const QString &) {};
    virtual void processingRemeds() {};
    virtual void sortRemeds(QVector<QVector<QGraphicsItemGroup*>> &);

    //this functions may call in virtual functions for get some parametres
    bool IsHidden() const noexcept { return _private.hideLabel; }
    QByteArray rawKeyString() const noexcept { return _private.fullStr; }
    quint8 attach() const noexcept { return _private.attach; }
    quint16 maxDrug() const noexcept { return _private.maxDrug; }
    quint16 indexFilter() const noexcept { return _private.filter; }
    bool IsLocalize() const noexcept { return _private.localize; }
private:
    bool _counter = true;
    bool _sorting = false;
    bool _navigation = false;
    std::shared_ptr<cache> _cache;
    openCtree _symptom;
    repertoryFilterType _filterType = repertoryFilterType::all;

    struct {
        const quint8 attachRatio = 3;
        const quint8 spaceHeight = 5;
        QByteArray index, endIndex;
        QByteArray fullStr;
        QRectF size;
        QPointF pos;
        bool localize = false;
        bool hideLabel = false;
        QVector<QGraphicsItem*> labelsVec;
        int heightView = 0, widthView = 0;
        int labelsEnd = 0;
        quint16 remFilter = -1;//repertory filter in widget menu
        quint8 attach = 0;
        quint16 maxDrug = 0, filter = 0;
        int labelWidth = 0;
    } _private;

    inline void initFonts();
    inline void authorsSym(const QString &, const quint16, QGraphicsItemGroup *, const bool next = false);

    inline void linksRender(QVector<QGraphicsItem*> &, std::array<QGraphicsSimpleTextItem*, 2> &);
    inline bool loopRender();
    inline void remedRender(QVector<QVector<QGraphicsItemGroup*>> &,
                     bool sorting = false, quint64 * remedSize = nullptr);
    inline void addLabel(QGraphicsItem *);

    inline void renderingChapter(const quint64, const quint64, std::array<QGraphicsSimpleTextItem*, 2> &);
    inline void linksItems(const quint8, const QString &,
           QVector<QGraphicsItem *> &, std::array<QGraphicsSimpleTextItem*, 2> &);
};

#endif // REPERTORYENGINE_H
