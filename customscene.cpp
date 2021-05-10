#include "customscene.h"

void customScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsItem *item = itemAt(mouseEvent->scenePos(), QTransform());
        QGraphicsSimpleTextItem *ell = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);
        if(ell)
            emit labelActivated(ell);
    }
}
