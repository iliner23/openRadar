#ifndef ABSTRACTENGINE_H
#define ABSTRACTENGINE_H

#include <QString>
#include <QFont>
#include <QGraphicsView>
#include <QGraphicsItemGroup>
#include <QtEndian>
#include <QTextCodec>
#include "openctree.h"
#include "cache.h"
#include "customscene.h"

class abstractEngine
{
protected:
    void renderingView(const int heightView, const int widthView);
    QString renderingLabel(QByteArray);
    abstractEngine();

    quint16 _remFilter = -1;//repertory filter in widget menu
    const cache * _cache = nullptr;
    QTextCodec * _codec = nullptr;
    openCtree _symptom;
    customScene * _scene = nullptr;
    QByteArray _index, _endIndex;
};

#endif // ABSTRACTENGINE_H
