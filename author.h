#ifndef AUTHOR_H
#define AUTHOR_H

#include <QDialog>
#include "cache.h"
#include <QDir>
#include <QTextCodec>

namespace Ui {
class author;
}

class author : public QDialog
{
    Q_OBJECT

public:
    explicit author(const QDir &, const quint32, const cache &, QWidget *parent = nullptr);
    ~author();

private:
    Ui::author *ui;
    cache _cache;
    QDir _system;
    quint32 _authorPos;
};

#endif // AUTHOR_H
