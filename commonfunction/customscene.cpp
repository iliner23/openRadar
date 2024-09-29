#include "customscene.h"

void customScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsItem * item = itemAt(mouseEvent->scenePos(), QTransform());
        QGraphicsSimpleTextItem * ell = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);

        if(ell) emit doubleClick(ell);
    }
}
void customScene::mouseReleaseEvent(QGraphicsSceneMouseEvent * mouseEvent){
    if (mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsItem * item = itemAt(mouseEvent->scenePos(), QTransform());
        QGraphicsSimpleTextItem * ell = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);

        if(ell) emit click(ell);
    }
}
void customScene::mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent){
    if (mouseEvent->button() == Qt::LeftButton)
    {
        QGraphicsItem * item = itemAt(mouseEvent->scenePos(), QTransform());
        QGraphicsSimpleTextItem * ell = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);

        if(ell && ell->data(0).toInt() == 0)
            _dragStartPosition = mouseEvent->scenePos();
        else
            _dragStartPosition = QPointF(0, 0);
    }
}
void customScene::mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent){
    if(!(mouseEvent->buttons() & Qt::LeftButton) || _dragStartPosition == QPointF(0, 0))
        return;

    if ((mouseEvent->scenePos() - _dragStartPosition).manhattanLength()
        < QApplication::startDragDistance())
        return;

    QGraphicsItem * item = itemAt(_dragStartPosition, QTransform());
    QGraphicsSimpleTextItem * ell = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item);

    if(!ell && ell->data(0).toInt() != 0)
        return;

    QDrag * drag = new QDrag(this);
    QMimeData * mimeData = new QMimeData;

    mimeData->setData("application/openRD.symptomPostion", ell->data(1).toByteArray());
    drag->setMimeData(mimeData);

    drag->exec(Qt::CopyAction);
}
