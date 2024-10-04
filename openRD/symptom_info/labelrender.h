#ifndef LABELRENDER_H
#define LABELRENDER_H
#include "repertoryrender.h"

class labelRender : public repertoryRender
{
public:
    labelRender() = default;
    labelRender(const QDir filename, const std::shared_ptr<func::cache> & cache,
                QTextCodec * codec = QTextCodec::codecForLocale());
    void reset(const QDir filename, const std::shared_ptr<func::cache> & cache,
                       QTextCodec * codec = QTextCodec::codecForLocale()) override;
    QVector<QGraphicsItemGroup*> render(const QSize resolution) override;
    bool isLocalize() { return _localize; };
    bool isHidden() const noexcept { return _hideLabel; }
    auto remedsCount() const { return _remedsCount; }
    auto synonymList() const { return _synonyms; }
    auto masterReferensesList() const { return _links; }
    auto crossReferensesList() const { return _crossLinks; }
private:
    std::array<int , 4> _remedsCount;
    bool _localize;
    bool _hideLabel;

    std::pair<QString, QString> _synonyms;
    std::pair<QStringList, QStringList> _links;
    std::pair<QStringList, QStringList> _crossLinks;
};

#endif // LABELRENDER_H
