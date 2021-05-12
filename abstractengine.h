#ifndef ABSTRACTENGINE_H
#define ABSTRACTENGINE_H

#include <QString>
#include <QFont>
#include <QGraphicsView>
#include <QGraphicsItemGroup>
#include <QtEndian>
#include <QTextCodec>
#include <memory>
#include "openctree.h"
#include "cache.h"
#include "customscene.h"
#include "remed_author.h"
#include "author.h"

class abstractEngine {
public:
    static QString renderingLabel(QByteArray, openCtree &symptom,
            const bool pass = true, QTextCodec * codec = QTextCodec::codecForName("system"));
protected:
    void renderingView(const int heightView, const int widthView);
    inline void addRemeds(QGraphicsItem * temp, const int labelWidth, const int widthView);
    inline void authorsSym(const QString & autr, const quint16 author, QGraphicsItemGroup * allrm, const bool next = false);
    inline void addLabel(QGraphicsItem *);

    abstractEngine();

    quint16 _remFilter = -1;//repertory filter in widget menu
    std::shared_ptr<cache> _cache;
    QTextCodec * _codec = nullptr;
    openCtree _symptom;
    customScene * _scene = nullptr;
    QByteArray _index, _endIndex;
    QDir _filename, _system;

    const quint8 _attachRatio = 3;
    const quint8 _spaceHeight = 5;

    QRectF _size;
    QPointF _pos;

    struct{
        QString fontName;
        QFont italicFont,
              commonFont,
              defaultFont,
              labelChapterFont,
              smallFont,
              boldFont;
    } _fonts;
};

#endif // ABSTRACTENGINE_H
