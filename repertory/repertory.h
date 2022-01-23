#ifndef REPERTORY_H
#define REPERTORY_H

#include <QtWidgets>
#include <unordered_map>
#include <memory>
#include "commonfunction/customscene.h"
#include "symptom_info/label.h"
#include "remed_info/remed_author.h"
#include "author_info/author.h"
#include "commonfunction/repertoryrender.h"
#include "symptom_search/vocabulary.h"

class repertory : public QWidget
{
    Q_OBJECT
public:
    explicit repertory(const QDir, const QDir,
                       std::shared_ptr<func::cache> &,
                       const std::pair<QLocale, QLocale> lang, keysRemedList * remedList,
                       const quint16 = -1, QWidget * = nullptr);
    QDir catalog() const noexcept { return _filename; }
    QTextCodec * textCodec() const noexcept { return _codec; }
    QByteArray currentPosition() const noexcept { return _pointer; }
private:
    QGraphicsView * _viewLeft;
    QGraphicsView * _viewRight;
    customScene * _scene;
    QScrollBar * _bar;
    QMenu * _menu;
    QLabel * _label;
    QByteArray _pointer;
    repertoryRender _engine;
    vocabulary * _vocabulary = nullptr;
    keysRemedList * _remedList = nullptr;

    QTextCodec * _codec = nullptr;
    std::pair<QLocale, QLocale> _lang;
    std::shared_ptr<func::cache> _cache;
    QDir _filename, _system;

    inline void repaintView();
    inline void notShowLabel() const;

    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent *event) override;
    void rendering();
private slots:
    void changeFilter(QAction *);
    void changedPos(const int);
    void doubleClickedAction(QGraphicsSimpleTextItem *item);
    void clickedAction(QGraphicsSimpleTextItem *item);
public slots:
    void setPosition(const QByteArray);
    void openVocabulary();
};

#endif // REPERTORY_H
