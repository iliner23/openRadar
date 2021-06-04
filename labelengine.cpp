#include "labelengine.h"

void labelEngine::render(const int heightView, const int widthView, const bool oneChapter){
    for(auto & it : _linksNames)
        it.clear();

    repertoryEngine::render(heightView, widthView, oneChapter);
}
int labelEngine::fullRemedsCount() const{
    int size = 0;

    for(const auto & it : _remedsSize)
        size += it;

    return size;
}
void labelEngine::processingLinks(const quint8 type, const QString & synLinkText){
    if(!_getLinksStr)
        return;

    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    if(synLinkText.size() > 2 && synLinkText.left(3) == " (="){
        const auto sz = _linksNames[0].size() + 1;
        if(type == 3){
            if(sz % 2 == 0)
                _linksNames[0].push_back("");
        }
        else if(type == 2){
            if((IsLocalize() && sz % 2 != 0) ||
              (!IsLocalize() && sz % 2 == 0))
                _linksNames[0].push_back("");
        }
        else
            return;

        _linksNames[0].push_back(synLinkText.mid(1));
    }
    else{
        for(auto it = 0; it != synLinkText.size(); ){
            const auto iterEnd = return_size(synLinkText.indexOf('/', it), synLinkText.size());
            QString linkText;

            if(iterEnd != synLinkText.size()){
                linkText = synLinkText.mid(it + 1, iterEnd - it - 2);
                it = iterEnd + 1;
            }
            else{
                linkText = synLinkText.mid(it + 1, iterEnd - it - 1);
                it = iterEnd;
            }

            const auto lnIndex = (type > 1) ? 1 : 2;
            const auto delFor2 = (type > 1) ? _linksNames[1].size() + 1 : _linksNames[2].size() + 1;

            if(type == 3 || type == 1){
                if(delFor2 % 2 == 0)
                    _linksNames[lnIndex].push_back("");

                _linksNames[lnIndex].push_back(linkText);
            }
            else{
                if((IsLocalize() && delFor2 % 2 != 0) ||
                  (!IsLocalize() && delFor2 % 2 == 0))
                    _linksNames[lnIndex].push_back("");

                _linksNames[lnIndex].push_back(linkText);
            }
        }
    }
}
void labelEngine::sortRemeds(QVector<QVector<QGraphicsItemGroup*>> & remeds){
    std::array<int, 4> sizeRem = {0, 0, 0, 0};

    for(auto it = remeds.rbegin(); it != remeds.rend(); ++it){
        sizeRem.at(it - remeds.rbegin()) = it->size();

        for(auto & iter : *it)
            addRemeds(iter);
    }

    _remedsSize = std::move(sizeRem);
}
