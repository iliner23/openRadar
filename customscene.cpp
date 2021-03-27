#include "customscene.h"

void customScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsItem *item = itemAt(mouseEvent->scenePos(), QTransform());
        customItem *ell = qgraphicsitem_cast<customItem *>(item);
        if(ell)
            emit labelActivated(ell);
    }
}
