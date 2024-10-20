#ifndef WIDGETACTION_H
#define WIDGETACTION_H

#include <QToolButton>
#include <QDragEnterEvent>
#include <QMimeData>

class widgetAction : public QToolButton
{
    Q_OBJECT
public:
    explicit widgetAction(QWidget *parent = nullptr);
protected:
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;
signals:
    void sendDropKey(QByteArray);
};

#endif // WIDGETACTION_H
