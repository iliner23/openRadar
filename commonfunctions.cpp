#include "commonfunctions.h"

std::pair<QStringList, QVector<QByteArray>> functions::linksParser::operator() (openCtree symptom, openCtree word, const QString expression, QTextCodec * codec){
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
QVector<QByteArray> functions::linksParser::keysParser(const std::string & key, QSet<QByteArray> & set){
    auto data = QByteArray::fromStdString(_word.at(key));
    const auto lastPos = data.indexOf('\0', _word.serviceDataLenght()) + 1;
    data = data.mid(lastPos);

    auto iterSec = data.cbegin();
    char oldSubRublic = 127;
    int tp = 0;
    QVector<QByteArray> keys;

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

            set.insert(ptr);
            keys.push_back(ptr);
        }

        oldSubRublic = subRublic;
    }

    return keys;
}
std::pair<QStringList, QVector<QByteArray>> functions::linksParser::logicalParser(QVector<std::pair<QString, operation>> & expr){
    QVector<std::pair<QVector<QByteArray>, operation>> multiKeys;

    for(auto exprIt = 0; exprIt != expr.size(); ++exprIt){
        std::string key;
        const operation oper = (exprIt == 0) ? operation::none : expr.at(exprIt - 1).second;
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
        QVector<QByteArray> tempPar;

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

    QVector<QByteArray> keysList;

    for(auto mult = 0; mult != multiKeys.size(); ++mult){
        QVector<QByteArray> tempList;

        if(multiKeys.at(mult).second == operation::none)
            keysList.append(multiKeys.at(mult).first);

        else if(multiKeys.at(mult).second == operation::AND){
            logicalANDparser(keysList, multiKeys.at(mult).first, tempList);
            keysList = tempList;
        }
        else{
            for(const auto & it : multiKeys.at(mult).first){
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

    if(keysList.size() > 500){
        const auto del = keysList.size() / 4;
        auto thread1 = QtConcurrent::run(thread, _symptom, keysList.constBegin(), keysList.constBegin() + del);
        auto thread2 = QtConcurrent::run(thread, _symptom, keysList.constBegin() + del, keysList.constBegin() + del * 2);
        auto thread3 = QtConcurrent::run(thread, _symptom, keysList.constBegin() + del * 2, keysList.constBegin() + del * 3);
        auto thread4 = QtConcurrent::run(thread, _symptom, keysList.constBegin() + del * 3, keysList.constEnd());

        list.append(thread1.result());
        list.append(thread2.result());
        list.append(thread3.result());
        list.append(thread4.result());
    }
    else
        list = thread(_symptom, keysList.constBegin(), keysList.constEnd());

    return std::make_pair(list, keysList);
}
void functions::linksParser::logicalANDparser(const QVector<QByteArray> firstList, const QVector<QByteArray> secondList, QVector<QByteArray> & tempList){
    const QVector<QByteArray> * list1, * list2;

    if(firstList.size() < secondList.size()){
        list2 = &secondList;
        list1 = &firstList;
    }
    else{
        list1 = &secondList;
        list2 = &firstList;
    }

    QVector<QVector<QByteArray>> fList, sList;

    auto listFiller = [](openCtree symptom,
                            const auto & sourceList, auto begin, auto end){
        QVector<QVector<QByteArray>> flist;

        for(auto i = begin; i != end; ++i){
            symptom.at(sourceList.at(i).toStdString(), false);
            flist += getRootPath(symptom);//TODO : add exception handler
        }

        return flist;
    };

    auto thread1 = QtConcurrent::run([&](){
        return threadsParent(*list1, listFiller); });
    auto thread2 = QtConcurrent::run([&](){
        return threadsParent(*list2, listFiller); });

    fList = thread1.result();
    sList = thread2.result();

    if(firstList == secondList){
        tempList = firstList;
        return;
    }

    auto compareThread = QtConcurrent::run([&](){ return firstList == secondList; });

    auto threadFor = [&](auto begin, auto end){
        QVector<QByteArray> tpList;

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

    if(list1->size() > 50){
        const auto del = list1->size() / 4;
        auto thread1 = QtConcurrent::run(threadFor, list1->constBegin(), list1->constBegin() + del);
        auto thread2 = QtConcurrent::run(threadFor, list1->constBegin() + del , list1->constBegin() + del * 2);
        auto thread3 = QtConcurrent::run(threadFor, list1->constBegin() + del * 2, list1->constBegin() + del * 3);
        auto thread4 = QtConcurrent::run(threadFor, list1->constBegin() + del * 3, list1->constEnd());

        tempList.append(thread1.result());
        tempList.append(thread2.result());
        tempList.append(thread3.result());
        tempList.append(thread4.result());
    }
    else
        tempList.append(threadFor(list1->constBegin(), list1->constEnd()));
}
QVector<QVector<QByteArray>> functions::linksParser::threadsParent(const QVector<QByteArray> & sourceList,
                                 std::function<QVector<QVector<QByteArray>>(openCtree symptom, const QVector<QByteArray> & , quint32 , quint32 )> threadFunc){
    QVector<QVector<QByteArray>> fillerList;

    if(sourceList.size() > 500){
        const auto del = sourceList.size() / 4;

        auto thread1 = QtConcurrent::run(threadFunc, _symptom, sourceList, 0, del);
        auto thread2 = QtConcurrent::run(threadFunc, _symptom, sourceList, del, del * 2);
        auto thread3 = QtConcurrent::run(threadFunc, _symptom, sourceList, del * 2, del * 3);
        auto thread4 = QtConcurrent::run(threadFunc, _symptom, sourceList, del * 3,  sourceList.size());

        fillerList.append(thread1.result());
        fillerList.append(thread2.result());
        fillerList.append(thread3.result());
        fillerList.append(thread4.result());
    }
    else
        fillerList = threadFunc(_symptom, sourceList, 0, sourceList.size());

    return fillerList;
}


QString functions::renderingLabel(openCtree & symptom, bool pass, QTextCodec * codec){
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
QVector<QByteArray> functions::getRootPath(openCtree & symptom, quint16 deep){
    QVector<QByteArray> array;
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
