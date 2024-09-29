#include "widgetaction.h"

widgetAction::widgetAction(QWidget * parent) : QToolButton{parent}{
    setAcceptDrops(true);
}
void widgetAction::dragEnterEvent(QDragEnterEvent * event){
    if (event->mimeData()->hasFormat("application/openRD.symptomPostion") ||
        event->mimeData()->hasFormat("application/openRD.repertoryFilePath"))
        event->acceptProposedAction();
}
void widgetAction::dropEvent(QDropEvent * event){
    QByteArray key = event->mimeData()->data("application/openRD.symptomPostion");
    emit sendDropKey(key);
    event->acceptProposedAction();
}
