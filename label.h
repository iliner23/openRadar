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
#include "labelrender.h"
#include "languages.h"

namespace Ui {
class Label;
}

class Label : public QDialog
{
    Q_OBJECT
private:
    Ui::Label *ui;
    quint32 _remedSize[4] = {0, 0, 0, 0};
    customScene * _scene;
    std::shared_ptr<cache> _cache;
    QDir _filename;

    std::pair<QString, QString> _synonyms;
    std::pair<QStringList, QStringList> _links;
    std::pair<QStringList, QStringList> _crossLinks;

    labelRender * _engine;
    QTextCodec * _codec;

    void renderingChapter();
private slots:
    void showTextInformation(QListWidgetItem*);
    void clickedAction(const QGraphicsSimpleTextItem * item);
public:
    explicit Label(std::shared_ptr<cache>, const QDir, const QByteArray,
                   const quint16 , std::pair<QLocale::Language, QLocale::Language>,
                   QWidget *parent = nullptr);

    bool isHiddenLabels() const noexcept { return _engine->isHidden(); }
    ~Label();
};

#endif // LABEL_H
