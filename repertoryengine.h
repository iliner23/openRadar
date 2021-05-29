#ifndef REPERTORYENGINE_H
#define REPERTORYENGINE_H

#include <QtWidgets>
#include <QTextCodec>
#include "openctree.h"
#include "cache.h"

enum repertoryFilterType : quint8 {
    none = 0,
    chapter = 1,
    links = 2,
    remeds = 4,
    all = 7
};

class repertoryEngine : public QObject
{
public:
    explicit repertoryEngine(QGraphicsScene *parent = nullptr);
    explicit repertoryEngine(const QDir & filename, const std::shared_ptr<cache> & cache
                    ,QGraphicsScene * scene = nullptr
                    ,QTextCodec * codec = QTextCodec::codecForName("system"));

    void reset(const QDir & filename, const std::shared_ptr<cache> & cache
               , QTextCodec * codec = QTextCodec::codecForName("system"));

    void setCurrentPosition(int pos);
    void setCurrentKey(const QByteArray & key);

    QByteArray currentKey() const noexcept { return _render.index; }
    int currentPosition() { return _symptom.currentPosition(); }

    void render(const int heightView, const int widthView);
    void render(const QByteArray & startKey, const QByteArray & endKey = "");

    int chaptersSize() const { return _symptom.size(); }

    void setFilter(repertoryFilterType filter) noexcept { _filter = filter; }
    repertoryFilterType filter() const noexcept { return _filter; }

    void setSortingRemeds(bool sorting) noexcept { _sorting = sorting; }
    bool IsSortingRemeds() const noexcept { return _sorting; }

    QVector<QGraphicsItem*> navigationItems() const { return _render.labelsVec; }
    void setUsingNavigationItems(bool navigation) noexcept { _navigation = navigation; }
    bool IsUsingNavigationItems() const noexcept { return _navigation; }

    void setChaptersFilter(quint16 filter) noexcept { _render.remFilter = filter; }
    quint16 chaptersFilter() const noexcept { return _render.filter; }

    QString renderingLabel(const bool passLastChapter = false);//from current key

    void setRemedsCounter(bool counter) noexcept { _counter = counter; }
    bool IsRemedsCounter() const noexcept { return _counter; }

    static QString renderingLabel(openCtree &symptom,
                bool passLastChapter = true, QTextCodec * codec = QTextCodec::codecForName("system"));
    static QVector<QByteArray> getRootPath(openCtree &);
private:
    inline void initFonts();

    QTextCodec * _codec = nullptr;
    std::shared_ptr<cache> _cache;
    openCtree _symptom;
    repertoryFilterType _filter = repertoryFilterType::all;
    bool _sorting = false;
    QVector<QGraphicsItem*> _navItems;
    bool _navigation = false;
    bool _counter = true;

    struct{
        QString fontName;
        QFont italicFont,
              commonFont,
              defaultFont,
              labelChapterFont,
              smallFont,
              boldFont;
    } _fonts;

    struct{
        quint16 remFilter = -1;//repertory filter in widget menu
        QByteArray index, endIndex;
        quint8 attach = 0;
        quint16 maxDrug = 0, filter = 0;
        int labelWidth = 0;

        const quint8 attachRatio = 3;
        const quint8 spaceHeight = 5;

        int heightView = 0, widthView = 0;
        int labelsEnd = 0;
        QByteArray fullStr;

        QRectF size;
        QPointF pos;

        bool localize = false;
        bool hideLabel = false;

        QVector<QGraphicsItem*> labelsVec;
    } _render;

    void renderingChapter();
    inline void authorsSym(const QString & autr, const quint16 author, QGraphicsItemGroup * allrm, const bool next = false);
    inline void remedRender(QVector<QVector<QGraphicsItemGroup*>> &,
                     bool sorting = false, quint64 * remedSize = nullptr);
    inline void addLabel(QGraphicsItem *);
    void addRemeds(QGraphicsItem * temp);
    inline void linksRender(QVector<QGraphicsItem*> &);
    inline void loopRender();
};

#endif // REPERTORYENGINE_H
