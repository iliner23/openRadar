#include "repertorydata.h"

func::repertoryData::repertoryData(const openCtree & repertory, QTextCodec * codec){
    reset(repertory, codec);
}
void func::repertoryData::reset(const openCtree & repertory, QTextCodec * codec){
    clear();
    _codec = codec;
    _data = QByteArray::fromStdString(repertory.currentValue());
    _key = repertory.key();

    quint16 _Fpos = 0;
    quint16 _Spos = 0;
    quint16 _Tpos = 0;

    _Fpos = repertory.serviceDataLenght();
    genLabels(_Fpos, _Spos);
    _Tpos = _Spos;
    genLinks(_Tpos);
    genRemeds(_Tpos);
}
void func::repertoryData::genLabels(const quint16 _Fpos, quint16 & _Spos){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    const auto firstZero = return_size(
                _data.indexOf('\0', _Fpos), _data.size());
    const auto secondZero = return_size(
                _data.indexOf('\0', firstZero + 1), _data.size());

    QString native = _codec->toUnicode(_data.mid(_Fpos, firstZero - _Fpos));
    QString localize = _codec->toUnicode
            (_data.mid(firstZero + 1, secondZero - firstZero - 1));

    _Spos = secondZero;

    _labels = std::make_pair(native, localize);
}
void func::repertoryData::genLinks(quint16 & _Tpos){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };
    quint8 type = 4;

    for(auto i = 0; i != 4; ++i){
        quint8 cnt = 0;

        for(auto it = 0; it != type + 1; ++it){
            if(_Tpos == _data.size()
                    || _data.at(_Tpos) != '\0')
                break;

            ++cnt;
            ++_Tpos;
        }

        if(cnt > type)
            break;

        type = type - cnt;

        const auto tmpLinkIter = return_size
                (_data.indexOf('\0', _Tpos), _data.size());

        auto synLinkText =
                _codec->toUnicode(_data.mid(_Tpos, tmpLinkIter - _Tpos));
        synLinkText.prepend(' ');
        _Tpos = tmpLinkIter;

        linksHandler(type, synLinkText);

        if(type == 0)
            break;
    }
}
void func::repertoryData::linksHandler(const quint8 type, const QString synLinkText){
    auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };

    if(synLinkText.size() > 2 && synLinkText.left(3) == " (="){
        if(type == 2 && !_labels.second.isEmpty())
            _synonyms.second = synLinkText.mid(1);
        else if(type == 3){
            _synonyms.first = synLinkText.mid(1);
        }
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

            switch (type) {
            case 0 : //loc
                _crossLinks.second.append(linkText);
                break;
            case 1 :
                _crossLinks.first.append(linkText);
                break;
            case 2 : //loc
                _links.second.append(linkText);
                break;
            case 3 :
                _links.first.append(linkText);
                break;
            }
        }
    }
}
void func::repertoryData::genRemeds(quint16 _Tpos){
    while(_Tpos < _data.size()){
        quint16 remed = 0, author = 0, tLevel = 0;
        uint8_t rLevel = 0;

        if(_Tpos + 7 >= _data.size())
            break;

        //const auto remedPos = _Tpos;

        remed = qFromLittleEndian<quint16>(_data.constData() + _Tpos);
        _Tpos += 2;
        rLevel = qFromLittleEndian<quint16>(_data.constData() + _Tpos);
        ++_Tpos;
        author = qFromLittleEndian<quint16>(_data.constData() + _Tpos);
        _Tpos += 2;
        tLevel = qFromLittleEndian<quint16>(_data.constData() + _Tpos);
        _Tpos += 2;

        if(memcmp(&remed, "\x5a\x5a", 2) == 0 && rLevel == '\x5a')
            break;
        if(rLevel == 0 || rLevel > 4)
            continue;

        _remeds.push_back(std::make_tuple(remed, rLevel, author, tLevel/*remedPos*/));
    }
}


void func::repertoryData::clear(){
    _data.clear();
    _key.clear();
    _remeds.clear();

    _links.first.clear();
    _links.second.clear();

    _crossLinks.first.clear();
    _crossLinks.second.clear();

    _synonyms = std::make_pair("", "");
    _labels = std::make_pair("", "");
}
std::string func::repertoryData::key() const{
    if(isClear())
        throw repertoryDataException();

    return _key;
}
std::pair<QString, QString> func::repertoryData::repertoryLabel() const{
    if(isClear())
        throw repertoryDataException();

    return _labels;
}
std::pair<QString, QString> func::repertoryData::synonymsList() const{
    if(isClear())
        throw repertoryDataException();

    return _synonyms;
}
std::pair<QStringList, QStringList> func::repertoryData::linksList() const{
    if(isClear())
        throw repertoryDataException();

    return _links;
}
std::pair<QStringList, QStringList> func::repertoryData::crossLinksList() const{
    if(isClear())
        throw repertoryDataException();

    return _crossLinks;
}
QVector<std::tuple<quint16, quint8, quint16, quint16/*, quint16*/>> func::repertoryData::remedsList() const{
    if(isClear())
        throw repertoryDataException();

    return _remeds;
}
QByteArray func::repertoryData::firstCurrentIndex() const{
    if(isClear())
        throw repertoryDataException();

    QByteArray key = _data.left(6);
    std::reverse(key.begin(), key.end());

    return key;
}
QByteArray func::repertoryData::secondCurrentIndex() const{
    if(isClear())
        throw repertoryDataException();

    QByteArray key = _data.mid(6, 6);
    std::reverse(key.begin(), key.end());

    return key;
}
QByteArray func::repertoryData::parentIndex() const{
    if(isClear())
        throw repertoryDataException();

    QByteArray key = _data.mid(12, 6);
    std::reverse(key.begin(), key.end());

    return key;
}
QByteArray func::repertoryData::rawData() const{
    if(isClear())
        throw repertoryDataException();

    return _data;
}
quint16 func::repertoryData::labelIndexFilter() const{
    if(isClear())
        throw repertoryDataException();

    return qFromLittleEndian<quint16>(_data.constData() + 26);
}
quint16 func::repertoryData::maxDrugs() const{
    if(isClear())
        throw repertoryDataException();

    return qFromLittleEndian<quint16>(_data.constData() + 22);
}
quint16 func::repertoryData::childrenLabels() const{
    if(isClear())
        throw repertoryDataException();

    return qFromLittleEndian<quint16>(_data.constData() + 24);
}
quint8 func::repertoryData::level() const{
    if(isClear())
        throw repertoryDataException();

    return qFromLittleEndian<quint8>(_data.constData() + 21);
}
quint8 func::repertoryData::chapterNumber() const{
    if(isClear())
        throw repertoryDataException();

    return qFromLittleEndian<quint8>(_data.constData() + 20);
}
