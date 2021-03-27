#include "customItem.h"

customItem::customItem()
{
    _font.setFamily("default");
}
void customItem::paint(QPainter * painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->setPen(_color);
    painter->setFont(_font);
    painter->drawText(2, _bon.height() - 2, _nameRemed);
}
QRectF customItem::boundingRect() const{
    if(_changed)change();
    return _bon;
}
void customItem::setPlainText(const QString & name){
    _nameRemed = name;
    _changed = true;
}
void customItem::change() const{
    QFontMetrics met(_font);
    if(!_nameRemed.isEmpty()){
        auto tp1 = met.boundingRect(_nameRemed);
        _bon.setWidth(tp1.width() + 4);
        _bon.setHeight(tp1.height() + 4);
    }
    _changed = false;
}
void customItem::setDefaultTextColor(const QColor color){
    _color = color;
}
void customItem::setFont(const QFont font){
    _font = font;
    _changed = true;
}
QFont customItem::getFont() const{
    return _font;
}
QColor customItem::getDefaultTextColor() const{
    return _color;
}
/*void customItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    event->accept();
    emit click(this);
}*/
