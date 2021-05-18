#ifndef LABEL_H
#define LABEL_H

#include <QtWidgets>
#include <memory>
#include <unordered_map>
#include "customscene.h"
#include "openctree.h"
#include "abstractengine.h"
#include "cache.h"
#include "remed_author.h"
#include "author.h"

namespace Ui {
class Label;
}

class Label : public QDialog, public abstractEngine
{
    Q_OBJECT
private:
    Ui::Label *ui;
    quint32 _remedSize[4] = {0, 0, 0, 0};
    bool _localize = false;
    QStringList _linksNames[3];//_synomSL, _masterSL, _referSL

    void renderingView(const int heightView, const int widthView);
private slots:
    void showTextInformation(QListWidgetItem*);
    void clickedAction(const QGraphicsSimpleTextItem * item);
public:
    explicit Label(std::shared_ptr<cache> &, const QDir &, const QDir &, const QByteArray &, const quint16 , QWidget *parent = nullptr);
    ~Label();
};

#endif // LABEL_H
