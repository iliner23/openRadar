#ifndef REPERTORY_H
#define REPERTORY_H

#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>
#include <unordered_map>
#include <memory>
#include "openctree.h"
#include "customscene.h"
#include "label.h"
#include "remed_author.h"
#include "cache.h"
#include "author.h"
#include "repertoryengine.h"
#include "vocabulary.h"

class repertory : public QWidget
{
    Q_OBJECT
public:
    explicit repertory(const QDir, const QDir,
                       std::shared_ptr<cache> &, QTextCodec *,
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
    repertoryEngine * _engine;
    vocabulary * _vocabulary = nullptr;

    QTextCodec * _codec = nullptr;
    QLocale::Language _lang;
    std::shared_ptr<cache> _cache;
    QDir _filename, _system;

    inline void repaintView();
    inline void notShowLabel() const;

    void resizeEvent(QResizeEvent*) override;
    void redrawPointer(QGraphicsItem *, const QVector<QGraphicsItem*> &);
    void keyPressEvent(QKeyEvent *event) override;
    void rendering();
    void redrawing();
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
