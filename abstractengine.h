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
    abstractEngine() = default;

    quint16 _remFilter = -1;//repertory filter in widget menu
    std::shared_ptr<cache> _cache;
    QTextCodec * _codec = nullptr;
    openCtree _symptom;
    customScene * _scene = nullptr;
    QByteArray _index, _endIndex;
    QDir _filename, _system;
};

#endif // ABSTRACTENGINE_H
