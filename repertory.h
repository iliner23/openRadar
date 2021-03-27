#ifndef REPERTORY_H
#define REPERTORY_H

#include <QWidget>
#include <QTextCodec>
#include <QString>
#include <QPainter>
#include <QDir>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QElapsedTimer>
#include <QTimer>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMenuBar>
#include <QResizeEvent>
#include <QLabel>
#include <unordered_map>
#include "openctree.h"
#include "customItem.h"
#include "customscene.h"
#include "label.h"
#include "remed_author.h"
#include "cache.h"
#include "author.h"

class repertory : public QWidget
{
    Q_OBJECT
public:
    explicit repertory(const QDir &, const QDir &, const cache &, const quint16 = -1, QWidget * = nullptr);
    QDir getRepDir() const noexcept;
private:
    openCtree _symptom;
    customScene * _scene;
    QGraphicsView * _viewLeft;
    QGraphicsView * _viewRight;
    QScrollBar * _bar;
    QTimer _timer;
    QMenu * _menu;
    quint16 _remFilter = -1;//repertory filter in widget menu
    QTextCodec * _codec;
    QLabel * _label;
    QDir _filename, _system;

    uint32_t _index = 0;
    const cache * _cache;

    void rendering();
    void resizeEvent(QResizeEvent*);
private slots:
    void changeFilter(QAction *);
    void changedPos(const int);
    void clickedAction(const customItem * item);
};

#endif // REPERTORY_H
