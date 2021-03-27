#ifndef CUSTOMSCENE_H
#define CUSTOMSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneEvent>
#include <customItem.h>

class customScene : public QGraphicsScene
{
Q_OBJECT
public:
    explicit customScene(QObject *parent = 0): QGraphicsScene(parent) {};
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent);
signals:
    void labelActivated(customItem *);
};

#endif // CUSTOMSCENE_H
