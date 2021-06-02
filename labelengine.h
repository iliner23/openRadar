#ifndef LABELENGINE_H
#define LABELENGINE_H

#include "repertoryengine.h"

class labelEngine : public repertoryEngine{
public:
    explicit labelEngine(QGraphicsScene *parent = nullptr) : repertoryEngine(parent) {}
    explicit labelEngine(const QDir & filename, const std::shared_ptr<cache> & cache
                    ,QGraphicsScene * scene = nullptr
                    ,QTextCodec * codec = QTextCodec::codecForName("system"))
                : repertoryEngine(filename, cache, scene, codec) {}

    void render(const int heightView, const int widthView, const bool oneChapter) override;

    int fullRemedsCount() const;
    int remeds1TypeCount() const { return (IsSortingRemeds()) ? _remedsSize[0] : 0; }
    int remeds2TypeCount() const { return _remedsSize[1]; }
    int remeds3TypeCount() const { return _remedsSize[2]; }
    int remeds4TypeCount() const { return _remedsSize[3]; }

    void setGetLinksStrings(bool strings) noexcept { _getLinksStr = strings; }
    bool IsGetLinksStrings() const noexcept { return _getLinksStr; }
    QStringList synomyList() const { return _linksNames[0]; }
    QStringList masterReferensesList() const { return _linksNames[1]; }
    QStringList crossReferensesList() const { return _linksNames[2]; }

    bool IsLocalize() const noexcept { return repertoryEngine::IsLocalize(); }
private:
    inline void linksStrings(const quint8, const QString &);
    void linksItems(const quint8, const QString &, QVector<QGraphicsItem *> &) override;
    void sortRemeds(QVector<QVector<QGraphicsItemGroup*>> &) override;

    bool _getLinksStr = false;
    QStringList _linksNames[3];
    std::array<int, 4> _remedsSize;
};

#endif // LABELENGINE_H
