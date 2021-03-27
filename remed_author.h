#ifndef REMED_AUTHOR_H
#define REMED_AUTHOR_H

#include <QDialog>
#include <QDir>
#include "openctree.h"
#include <QListWidgetItem>
#include <QListWidget>
#include "cache.h"
#include <QStringList>
#include <QTextCodec>

namespace Ui {
class remed_author;
}

class remed_author : public QDialog
{
    Q_OBJECT
private:
    QDir _path, _system;
    openCtree _sym;
    quint32 _pos, _localPos;
    quint16 _remFilter = -1;
    const cache * _cache;
    QStringList _authorsText;

    void rendering();
private slots:
    void showTextInformation(QListWidgetItem*);
public:
    explicit remed_author(const QDir &, const QDir &, const cache &,
                          const quint32, const quint16, const quint32, QWidget *parent = nullptr);
    ~remed_author();

private:
    Ui::remed_author *ui;
};

#endif // REMED_AUTHOR_H
