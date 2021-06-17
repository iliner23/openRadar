#include "searchresult.h"
#include "ui_searchresult.h"

searchResult::searchResult(QWidget * parent):
    QDialog(parent),
    ui(new Ui::searchResult)
{
    ui->setupUi(this);
    connect(ui->pushButton, &QPushButton::clicked, this, &searchResult::accept);
}
searchResult::searchResult(const QFileInfo word, const QFileInfo symptom, const QString expression, QTextCodec * codec, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::searchResult)
{
    ui->setupUi(this);
    setWindowFlag(Qt::Window, true);
    setData(word, symptom, expression, codec);
}
void searchResult::setData(const QFileInfo word, const QFileInfo symptom, const QString expression, QTextCodec * codec){
    _codec = codec;

    _word.open(word.filePath().toStdString());
    _symptom.open(symptom.filePath().toStdString());

    expressionParser(expression);
    logicalParser();
}
void searchResult::expressionParser(const QString expr){
    ui->label->setText(expr);

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

        _expr.push_back(pair);
    }
}
QVector<QByteArray> searchResult::keysParser(const std::string & key, QSet<QByteArray> & set){
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
void searchResult::logicalParser(){
    QVector<std::pair<QVector<QByteArray>, operation>> multiKeys;

    for(auto exprIt = 0; exprIt != _expr.size(); ++exprIt){
        std::string key;
        const operation oper = (exprIt == 0) ? operation::none : _expr.at(exprIt - 1).second;
        auto string = _expr.at(exprIt).first;

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

    _expr.clear();
    ui->listWidget->setUpdatesEnabled(false);

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
                lst.push_back(repertoryEngine::renderingLabel(symptom, false, _codec));
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
        list.append(thread(_symptom, keysList.constBegin(), keysList.constEnd()));

    ui->listWidget->addItems(list);
    ui->label_2->setText(QString::number(list.size()));
    ui->listWidget->setUpdatesEnabled(true);
}
void searchResult::logicalANDparser(const QVector<QByteArray> firstList, const QVector<QByteArray> secondList, QVector<QByteArray> & tempList){
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
            flist += repertoryEngine::getRootPath(symptom);
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
                    qDebug() << "middle";
                }
                else if(*it < *ir && tempList.indexOf(*ir) == -1 &&
                                        tpList.indexOf(*ir) == -1){
                    const auto iter = sList.at(ir - list2->constBegin()).indexOf(*it);

                    if(iter != -1){
                        tpList.push_back(*ir);
                        qDebug() << "up";
                    }
                }
                else if(*it > *ir && tempList.indexOf(*it) == -1 &&
                                        tpList.indexOf(*it) == -1){
                    const auto iter = fList.at(it - list1->constBegin()).indexOf(*ir);

                    if(iter != -1){
                        tpList.push_back(*it);
                        qDebug() << "down";
                    }
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
QVector<QVector<QByteArray>> searchResult::threadsParent(const QVector<QByteArray> & sourceList,
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
searchResult::~searchResult()
{
    delete ui;
}
void searchResult::clearValues(){
    ui->listWidget->clear();
}
void searchResult::reject(){
    clearValues();
    QDialog::reject();
}
void searchResult::accept(){
    clearValues();
    QDialog::accept();
}
