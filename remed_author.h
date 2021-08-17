#ifndef REMED_AUTHOR_H
#define REMED_AUTHOR_H

#include <QtWidgets>
#include <QTextCodec>
#include "openctree.h"
#include <memory>
#include "cache.h"
#include "commonfunctions.h"

namespace Ui {
class remed_author;
}

class remed_author : public QDialog
{
    Q_OBJECT
private:
    QDir _path;
    openCtree _sym;
    QByteArray _pos;
    int _localPos;
    quint16 _remFilter = -1;
    std::shared_ptr<cache> _cache;
    QStringList _authorsText;
    QTextCodec * _codec;

    void rendering();
private slots:
    void showTextInformation(QListWidgetItem*);
public:
    explicit remed_author(const QDir, std::shared_ptr<cache>,
                          const QByteArray, const quint16, const quint32,
                          QTextCodec * codec = QTextCodec::codecForName("system"),
                          QWidget *parent = nullptr);
    ~remed_author();

private:
    Ui::remed_author *ui;
};

#endif // REMED_AUTHOR_H
