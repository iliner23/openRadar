#ifndef REMED_AUTHOR_H
#define REMED_AUTHOR_H

#include <QDialog>
#include <QDir>
#include "openctree.h"
#include <QListWidgetItem>
#include <QListWidget>
#include <memory>
#include "cache.h"
#include <QStringList>
#include <QTextCodec>
#include "abstractengine.h"

namespace Ui {
class remed_author;
}

class remed_author : public QDialog
{
    Q_OBJECT
private:
    QDir _path, _system;
    openCtree _sym;
    QByteArray _pos;
    quint32 _localPos;
    quint16 _remFilter = -1;
    std::shared_ptr<cache> _cache;
    QStringList _authorsText;

    void rendering();
private slots:
    void showTextInformation(QListWidgetItem*);
public:
    explicit remed_author(const QDir &, const QDir &, std::shared_ptr<cache> &,
                          const QByteArray &, const quint16, const quint32, QWidget *parent = nullptr);
    ~remed_author();

private:
    Ui::remed_author *ui;
};

#endif // REMED_AUTHOR_H
