#include "delegate.h"

void delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    auto style = option;
    style.decorationAlignment = Qt::AlignHCenter;
    style.decorationPosition = QStyleOptionViewItem::Top;
    style.decorationSize = QSize(64, 64);
    QStyledItemDelegate::paint(painter, style, index);
}
