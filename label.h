#ifndef LABEL_H
#define LABEL_H

#include <QtWidgets>
#include <memory>
#include <unordered_map>
#include "customscene.h"
#include "openctree.h"
#include "cache.h"
#include "remed_author.h"
#include "author.h"
#include "labelengine.h"

namespace Ui {
class Label;
}

class Label : public QDialog
{
    Q_OBJECT
private:
    Ui::Label *ui;
    quint32 _remedSize[4] = {0, 0, 0, 0};
    bool _localize = false;
    customScene * _scene;
    std::shared_ptr<cache> _cache;
    QDir _filename;
    QStringList _linksNames[3];//_synomSL, _masterSL, _referSL
    labelEngine * _engine;
    QTextCodec * _codec;

    void renderingChapter();
private slots:
    void showTextInformation(QListWidgetItem*);
    void clickedAction(const QGraphicsSimpleTextItem * item);
public:
    explicit Label(std::shared_ptr<cache> &, const QDir &, const QByteArray &,
                   const quint16 , QTextCodec *, QWidget *parent = nullptr);

    bool isHiddenLabels() const noexcept { return _engine->IsHidden(); }
    ~Label();
};

#endif // LABEL_H
