#ifndef CUSTOMSCENE_H
#define CUSTOMSCENE_H

#include <QApplication>
#include <QDrag>
#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <QGraphicsSimpleTextItem>
#include "repertorydata.h"

class customScene : public QGraphicsScene
{
Q_OBJECT
public:
    explicit customScene(QObject *parent = 0): QGraphicsScene(parent) {};
protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;

    QPointF _dragStartPosition;
signals:
    void doubleClick(QGraphicsSimpleTextItem *);
    void click(QGraphicsSimpleTextItem *);
};

#endif // CUSTOMSCENE_H
