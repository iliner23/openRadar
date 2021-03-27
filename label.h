#ifndef LABEL_H
#define LABEL_H

#include <QDialog>
#include <customscene.h>
#include <QGraphicsItemGroup>
#include <QResizeEvent>
#include <QTextCodec>
#include <QStringList>
#include <QDir>
#include <QListWidgetItem>
#include <unordered_map>
#include "customItem.h"
#include "openctree.h"
#include "cache.h"

namespace Ui {
class Label;
}

class Label : public QDialog
{
    Q_OBJECT
private:
    Ui::Label *ui;
    cache _cache;
    customScene * _scene;
    QDir _path, _system;
    quint16 _remFilter = -1;
    quint32 _rem1 = 0, _rem2 = 0, _rem3 = 0, _rem4 = 0, _pos;

    openCtree _sym;
    bool _localize = false;
    QStringList _synomSL, _referSL, _masterSL;

    void rendering();
private slots:
    void showTextInformation(QListWidgetItem*);
public:
    explicit Label(const cache &, const QDir &, const QDir &, const quint32, const quint16 , QWidget *parent = nullptr);
    ~Label();
};

#endif // LABEL_H
