#ifndef DELEGATE_H
#define DELEGATE_H

#include <QStyledItemDelegate>

class delegate : public QStyledItemDelegate
{
public:
    delegate() = default;
    virtual void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
};

#endif // DELEGATE_H
