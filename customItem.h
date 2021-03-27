#ifndef CUSTOMITEM_H
#define CUSTOMITEM_H

#include <QGraphicsItem>
#include <QString>
#include <QFontMetrics>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

class customItem : public QGraphicsObject
{
    Q_OBJECT
private:
    QString _nameRemed;
    mutable QRectF _bon;
    QPointF _pos;
    QColor _color = Qt::black;
    QFont _font;
    mutable bool _changed = false;
    inline void change() const;
public:
    customItem();
    void setPlainText(const QString&);
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override;
    void setDefaultTextColor(const QColor);
    void setFont(const QFont);
    QFont getFont() const;
    QColor getDefaultTextColor() const;
};

#endif // CUSTOMITEM_H
