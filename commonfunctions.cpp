#include "commonfunctions.h"

std::pair<QStringList, QByteArrayList> functions::linksParser::operator() (openCtree symptom, openCtree word, const QString expression, QTextCodec * codec){
    QVector<std::pair<QString, operation>> expr;

    _codec = codec;
    _symptom = std::move(symptom);
    _word = std::move(word);
    expressionParser(expression, expr);
    return logicalParser(expr);
}
void functions::linksParser::expressionParser(const QString expr, QVector<std::pair<QString, operation>> & exAr){
    QRegularExpression re(R"(\[word:(\S+)\]\s*(AND|OR|\s*))",
                          QRegularExpression::CaseInsensitiveOption);
    auto globMatch = re.globalMatch(expr);

    while(globMatch.hasNext()){
        auto match = globMatch.next();

        std::pair<QString, operation> pair;
        pair.first = match.captured(1);

        if(match.lastCapturedIndex() > 1){
            if(match.captured(2) == "AND")
                pair.second = operation::AND;
            else if(match.captured(2) == "OR")
                pair.second = operation::OR;
            else
                pair.second = operation::none;
        }
        else
            pair.second = operation::none;

        exAr.push_back(pair);
    }
}
QByteArrayList functions::linksParser::keysParser(const std::string & key, QSet<QByteArray> & set){
    auto data = QByteArray::fromStdString(_word.at(key));
    const auto lastPos = data.indexOf('\0', _word.serviceDataLenght()) + 1;
    data = data.mid(lastPos);

    auto iterSec = data.cbegin();
    char oldSubRublic = 127;
    int tp = 0;
    QByteArrayList keys;

    while(iterSec != data.cend()){
        QByteArray ptr(6, '\0');
        const auto subRublic = *(iterSec++);

        std::copy(iterSec, iterSec + 4, ptr.begin() + 2);
        iterSec += 4;

        if(subRublic != '\x80' && subRublic != '\0'){
            const auto subTemp = subRublic & ~'\x80';
            ptr[0] = '\0';
            ptr[1] = subTemp;
        }

        if(oldSubRublic != subRublic || (subRublic & 0x80) == 0x80)
            tp = 1;
        else
            ++tp;

        if(tp % 2 != 0){
            if(set.constFind(ptr) != set.cend())
                continue;

            if(_symptom.haveKey(ptr.toStdString())){
                set.insert(ptr);
                keys.push_back(ptr);
            }
        }

        oldSubRublic = subRublic;
    }

    return keys;
}
std::pair<QStringList, QByteArrayList> functions::linksParser::logicalParser(QVector<std::pair<QString, operation>> & expr){
    QVector<std::pair<QByteArrayList, operation>> multiKeys;

    for(auto exprIt = 0; exprIt != expr.size(); ++exprIt){
        std::string key;
        const operation oper = expr.at(exprIt).second;
        auto string = expr.at(exprIt).first;

        for(auto & it : string){
            if(QRegularExpression("[^\\x{0000}-\\x{007F}]").
                    match(it).captured().isEmpty())
                it = it.toUpper();
        }

        key = _word.encodeKey(_codec->fromUnicode(string).toStdString());
        key.resize(_word.keyLenght(), _word.paddingChar());
        key.back() = '\0';

        QSet<QByteArray> set;
        QByteArrayList tempPar;

        while(_word.haveKey(key)){
            tempPar.append(keysParser(key, set));

            if(key.back() == (char) 254)
                break;

            key.back() = key.back() + 1;
        }

        if(tempPar.isEmpty())
            continue;

        multiKeys.push_back(std::make_pair(tempPar, oper));
    }

    QByteArrayList keysList;

    for(auto mult = multiKeys.crbegin(); mult != multiKeys.crend(); ++mult){
        QByteArrayList tempList;

        if(mult == multiKeys.crbegin())
            keysList.append(mult->first);

        else if(mult->second == operation::AND){
            logicalANDparser(keysList, mult->first, tempList);
            keysList = tempList;
        }
        else{
            for(const auto & it : mult->first){
                const auto kel = keysList.indexOf(it);

                if(kel == -1)
                   keysList.push_back(it);
            }
        }
    }

    QStringList list;

    auto thread = [&](openCtree symptom, auto begin, auto end){
        QStringList lst;

        for(auto it = begin; it != end; ++it){
            try{
                symptom.at(it->toStdString(), false);
                lst.push_back(renderingLabel(symptom, false, _codec));
            }
            catch(std::exception){}
        }

        return lst;
    };

    QVector<QFuture<QStringList>> threads;

    const auto maxThreads = (keysList.size() > QThread::idealThreadCount()) ?
                QThread::idealThreadCount() : keysList.size();
    const auto del = keysList.size() / maxThreads;

    for(auto i = 0; i != maxThreads; ++i){
        threads.append(QtConcurrent::run(thread, _symptom,
            keysList.constBegin() + del * i ,
            (i == maxThreads - 1) ? keysList.constEnd()
                                  : keysList.constBegin() + del * (i + 1)));
    }

    for(auto & it : threads)
        list.append(it.result());

    return std::make_pair(list, keysList);
}
void functions::linksParser::logicalANDparser(const QByteArrayList firstList, const QByteArrayList secondList, QByteArrayList & tempList){
    const QByteArrayList * list1, * list2;

    if(firstList.size() < secondList.size()){
        list2 = &secondList;
        list1 = &firstList;
    }
    else{
        list1 = &secondList;
        list2 = &firstList;
    }

    QVector<QByteArrayList> fList, sList;

    auto listFiller = [](openCtree symptom,
                            const auto & sourceList, auto begin, auto end){
        QVector<QByteArrayList> flist;

        for(auto i = begin; i != end; ++i){
            try{
                symptom.at(sourceList.at(i).toStdString(), false);
                flist += getRootPath(symptom);
            }
            catch(std::exception) {}
        }

        return flist;
    };

    fList = threadsParent(*list1, listFiller);
    sList = threadsParent(*list2, listFiller);

    if(firstList == secondList){
        tempList = firstList;
        return;
    }

    auto threadFor = [&](auto begin, auto end){
        QByteArrayList tpList;

        for(auto it = begin; it != end; ++it){
            for(auto ir = list2->constBegin(); ir != list2->constEnd(); ++ir){
                if(*it == *ir && tempList.indexOf(*ir) == -1 &&
                                    tpList.indexOf(*ir) == -1){
                    tpList.push_back(*it);
                }
                else if(*it < *ir && tempList.indexOf(*ir) == -1 &&
                                        tpList.indexOf(*ir) == -1){
                    const auto iter = sList.at(ir - list2->constBegin()).indexOf(*it);

                    if(iter != -1)
                        tpList.push_back(*ir);
                }
                else if(*it > *ir && tempList.indexOf(*it) == -1 &&
                                        tpList.indexOf(*it) == -1){
                    const auto iter = fList.at(it - list1->constBegin()).indexOf(*ir);

                    if(iter != -1)
                        tpList.push_back(*it);
                }
            }
        }

        return tpList;
    };

    QVector<QFuture<QByteArrayList>> threads;

    const auto maxThreads = (list1->size() > QThread::idealThreadCount()) ?
                QThread::idealThreadCount() : list1->size();
    const auto del = list1->size() / maxThreads;

    for(auto i = 0; i != maxThreads; ++i){
        threads.append(QtConcurrent::run(threadFor,
            list1->constBegin() + del * i ,
            (i == maxThreads - 1) ? list1->constEnd() :
                                    list1->constBegin() + del * (i + 1)));
    }

    for(auto & it : threads)
        tempList.append(it.result());
}
QVector<QByteArrayList> functions::linksParser::threadsParent(const QByteArrayList & sourceList,
                                 std::function<QVector<QByteArrayList>(openCtree symptom, const QByteArrayList & , quint32 , quint32 )> threadFunc){
    QVector<QByteArrayList> fillerList;

    QVector<QFuture<QVector<QByteArrayList>>> threads;

    const auto maxThreads = (sourceList.size() > QThread::idealThreadCount()) ?
                QThread::idealThreadCount() : sourceList.size();
    const auto del = sourceList.size() / maxThreads;

    for(auto i = 0; i != maxThreads; ++i){
        threads.append(QtConcurrent::run(threadFunc, _symptom, sourceList, del * i ,
            (i == maxThreads - 1) ? sourceList.size() : del * (i + 1)));
    }

    for(auto & it : threads)
        fillerList.append(it.result());

    return fillerList;
}


QString functions::renderingLabel(openCtree symptom, bool pass, QTextCodec * codec){
    QStringList original, localization;
    auto text = QByteArray::fromStdString(symptom.currentValue());
    QByteArray ind(6, '\0');
    bool fis = true;

    auto return_size = [](const auto & value, const auto & size){
        return (value == -1) ? size : value;
    };

    if(pass == false){
        std::reverse_copy(text.cbegin() + 6, text.cbegin() + 12, ind.begin());
        fis = false;
    }

    while(true){
        quint8 attach = 0;

        if(!fis){
            text = QByteArray::fromStdString(symptom.at(ind.toStdString()));
            const auto first = return_size(text.indexOf('\0', symptom.serviceDataLenght()), text.size());
            const auto second = return_size(text.indexOf('\0', first + 1), text.size()) - first - 1;
            const auto local = codec->toUnicode(text.mid(first + 1, second));

            original.push_back(codec->toUnicode(text.mid(symptom.serviceDataLenght()
                                                          , first - symptom.serviceDataLenght())));
            if(!local.isEmpty())
                localization.push_back(local);
        }

        attach = text.at(21);
        QByteArray midCompare(6, '\0');
        std::reverse_copy(text.cbegin() + 6, text.cbegin() + 12, midCompare.begin());
        std::reverse_copy(text.cbegin() + 12, text.cbegin() + 18, ind.begin());

        if(midCompare.left(4) != ind.left(4)){
            ind.replace(ind.size() - 2, 2, "\0\0", 2);}

        else if(ind.back() != '\0' || ind.at(ind.size() - 1) != '\0'){
            auto decrease = qFromBigEndian<quint16>(ind.right(2)) - 1;
            decrease = qToBigEndian<quint16>(decrease);
            ind.replace(ind.size() - 2, 2, (char *)&decrease, 2);
        }

        if(attach == 0)
            Q_ASSERT("attach == 0");
        else if(attach <= 1 || ind == "\0\0\0\0\0\0")
            break;

        fis = false;
    }

    if(original.isEmpty())
        return QString();

    QString org, lz;

    for(auto it = original.size() - 1; it != -1; --it){
        if(it == original.size() - 1)
            org += original[it];
        else
            org += " - " + original[it];
    }

    if(!localization.isEmpty()){
        for(auto it = localization.size() - 1; it != -1; --it){
            if(it == localization.size() - 1)
                lz += localization[it];
            else
                lz += " - " + localization[it];
        }
    }

    return org + ((localization.isEmpty()) ? "" : '\n' + lz);
}
std::pair<QStringList, QStringList> functions::renderingLabel(openCtree symptom, QTextCodec * codec){
    QStringList original, localization;
    auto text = QByteArray::fromStdString(symptom.currentValue());
    QByteArray ind(6, '\0');
    bool fis = true;

    auto return_size = [](const auto & value, const auto & size){
        return (value == -1) ? size : value;
    };

    std::reverse_copy(text.cbegin() + 6, text.cbegin() + 12, ind.begin());
    fis = false;

    while(true){
        quint8 attach = 0;

        if(!fis){
            text = QByteArray::fromStdString(symptom.at(ind.toStdString()));
            const auto first = return_size(text.indexOf('\0', symptom.serviceDataLenght()), text.size());
            const auto second = return_size(text.indexOf('\0', first + 1), text.size()) - first - 1;
            const auto local = codec->toUnicode(text.mid(first + 1, second));

            original.push_back(codec->toUnicode(text.mid(symptom.serviceDataLenght()
                                                          , first - symptom.serviceDataLenght())));
            if(!local.isEmpty())
                localization.push_back(local);
        }

        attach = text.at(21);
        QByteArray midCompare(6, '\0');
        std::reverse_copy(text.cbegin() + 6, text.cbegin() + 12, midCompare.begin());
        std::reverse_copy(text.cbegin() + 12, text.cbegin() + 18, ind.begin());

        if(midCompare.left(4) != ind.left(4)){
            ind.replace(ind.size() - 2, 2, "\0\0", 2);}

        else if(ind.back() != '\0' || ind.at(ind.size() - 1) != '\0'){
            auto decrease = qFromBigEndian<quint16>(ind.right(2)) - 1;
            decrease = qToBigEndian<quint16>(decrease);
            ind.replace(ind.size() - 2, 2, (char *)&decrease, 2);
        }

        if(attach == 0)
            Q_ASSERT("attach == 0");
        else if(attach <= 1 || ind == "\0\0\0\0\0\0")
            break;

        fis = false;
    }

    return std::make_pair(original, localization);
}
QByteArrayList functions::getRootPath(openCtree symptom, quint16 deep){
    QByteArrayList array;
    auto text = QByteArray::fromStdString(symptom.currentValue());
    QByteArray ind(6, '\0');

    std::reverse_copy(text.cbegin() + 6, text.cbegin() + 12, ind.begin());

    for(quint16 i = 0;; ++i){
        quint8 attach = 0;

        array.push_back(ind);

        if(i == deep)
            break;

        text = QByteArray::fromStdString(symptom.at(ind.toStdString()));

        attach = text.at(21);
        QByteArray midCompare(6, '\0');
        std::reverse_copy(text.cbegin() + 6, text.cbegin() + 12, midCompare.begin());
        std::reverse_copy(text.cbegin() + 12, text.cbegin() + 18, ind.begin());

        if(midCompare.left(4) != ind.left(4)){
            ind.replace(ind.size() - 2, 2, "\0\0", 2);}

        else if(ind.back() != '\0' || ind.at(ind.size() - 1) != '\0'){
            auto decrease = qFromBigEndian<quint16>(ind.right(2)) - 1;
            decrease = qToBigEndian<quint16>(decrease);
            ind.replace(ind.size() - 2, 2, (char *)&decrease, 2);
        }

        if(attach == 0)
            Q_ASSERT("attach == 0");
        else if(attach <= 1 || ind == "\0\0\0\0\0\0")
            break;
    }

    return array;
}
