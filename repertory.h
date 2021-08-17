#ifndef REPERTORY_H
#define REPERTORY_H

#include <QtWidgets>
#include <unordered_map>
#include <memory>
#include "customscene.h"
#include "label.h"
#include "remed_author.h"
#include "author.h"
#include "repertoryrender.h"
#include "vocabulary.h"

class repertory : public QWidget
{
    Q_OBJECT
public:
    explicit repertory(const QDir, const QDir,
                       std::shared_ptr<cache> &,
                       const std::pair<QLocale::Language, QLocale::Language> lang,
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

    QTextCodec * _codec = nullptr;
    std::pair<QLocale::Language, QLocale::Language> _lang;
    std::shared_ptr<cache> _cache;
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
