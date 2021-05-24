#ifndef REPERTORY_H
#define REPERTORY_H

#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>
#include <unordered_map>
#include <memory>
#include "openctree.h"
#include "customscene.h"
#include "label.h"
#include "remed_author.h"
#include "cache.h"
#include "author.h"
#include "abstractengine.h"

class repertory : public QWidget, public abstractEngine
{
    Q_OBJECT
public:
    explicit repertory(const QDir &, const QDir &,
                       std::shared_ptr<cache> &, QTextCodec *,
                       const quint16 = -1, QWidget * = nullptr);
    QDir getRepDir() const noexcept;
    QTextCodec * getTextCodec() const noexcept;
private:
    QGraphicsView * _viewLeft;
    QGraphicsView * _viewRight;
    QScrollBar * _bar;
    QMenu * _menu;
    QLabel * _label;

    inline void repaintView();
    void resizeEvent(QResizeEvent*);
private slots:
    void changeFilter(QAction *);
    void changedPos(const int);
    void doubleClickedAction(const QGraphicsSimpleTextItem *item);
    void clickedAction(const QGraphicsSimpleTextItem *item);
public slots:
    void setPosition(const QByteArray &);
};

#endif // REPERTORY_H
