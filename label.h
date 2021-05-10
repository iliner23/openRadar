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
#include "openctree.h"
#include "abstractengine.h"
#include "cache.h"

namespace Ui {
class Label;
}

class Label : public QDialog, public abstractEngine
{
    Q_OBJECT
private:
    Ui::Label *ui;
    QDir _path, _system;
    quint32 _remedSize[4] = {0, 0, 0, 0};
    bool _localize = false;
    QStringList _linksNames[3];//_synomSL, _masterSL, _referSL

    void renderingView(const int heightView, const int widthView);
private slots:
    void showTextInformation(QListWidgetItem*);
public:
    explicit Label(const cache &, const QDir &, const QDir &, const QByteArray &, const quint16 , QWidget *parent = nullptr);
    ~Label();
};

#endif // LABEL_H
