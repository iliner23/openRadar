#include "customscene.h"

void customScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsItem *item = itemAt(mouseEvent->scenePos(), QTransform());
        QGraphicsSimpleTextItem *ell = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(ell)
            emit doubleClick(ell);
    }
}
void customScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent){
    if (mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsItem *item = itemAt(mouseEvent->scenePos(), QTransform());
        QGraphicsSimpleTextItem *ell = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(ell)
            emit click(ell);
    }
}
