#ifndef REPERTORY_H
#define REPERTORY_H

#include <QWidget>
#include <QTextCodec>
#include <QString>
#include <QPainter>
#include <QDir>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMenuBar>
#include <QResizeEvent>
#include <QLabel>
#include <QtEndian>
#include <QtConcurrent/QtConcurrent>
#include <unordered_map>
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
    explicit repertory(const QDir &, const QDir &, const cache &, const quint16 = -1, QWidget * = nullptr);
    QDir getRepDir() const noexcept;
private:
    QGraphicsView * _viewLeft;
    QGraphicsView * _viewRight;
    QScrollBar * _bar;
    QMenu * _menu;
    QLabel * _label;
    QDir _filename, _system;

    inline void repaintView();
    void resizeEvent(QResizeEvent*);
    //void renderingLabel(QByteArray);
private slots:
    void changeFilter(QAction *);
    void changedPos(const int);
    void clickedAction(const QGraphicsSimpleTextItem *item);
public slots:
    void setPosition(const QByteArray &);
};

#endif // REPERTORY_H
