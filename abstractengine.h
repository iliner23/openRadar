#ifndef ABSTRACTENGINE_H
#define ABSTRACTENGINE_H

#include <QtWidgets>
#include <QTextCodec>
#include <memory>
#include "openctree.h"
#include "cache.h"
#include "customscene.h"

class abstractEngine {
public:
    static QString renderingLabel(QByteArray, openCtree &symptom,
                bool pass = true, QTextCodec * codec = QTextCodec::codecForName("system"));
protected:
    void renderingView(const int heightView, const int widthView);
    void addRemeds(QGraphicsItem * temp, const int labelWidth, const int widthView);
    inline void authorsSym(const QString & autr, const quint16 author, QGraphicsItemGroup * allrm, const bool next = false);
    inline void addLabel(QGraphicsItem *);
    QString renderingLabel(const bool pass = false);
    inline void remedRender(QVector<QVector<QGraphicsItemGroup*>> &,
                     bool sorting = false, quint64 * remedSize = nullptr);
    void renderingChapter();
    abstractEngine(QTextCodec *);

    std::shared_ptr<cache> _cache;
    QTextCodec * _codec = nullptr;
    openCtree _symptom;
    customScene * _scene = nullptr;
    QDir _filename, _system;

    struct{
        quint16 remFilter = -1;//repertory filter in widget menu
        QByteArray index, endIndex;
        quint8 attach = 0;
        quint16 maxDrug = 0, filter = 0;
        int labelWidth = 0;

        const quint8 attachRatio = 3;
        const quint8 spaceHeight = 5;

        int heightView = 0, widthView = 0;
        int labelsEnd = 0;
        QByteArray fullStr;

        QRectF size;
        QPointF pos;

        bool localize = false;
        bool hideLabel = false;

        QVector<QGraphicsSimpleTextItem*> labelsVec;
    } _render;

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
