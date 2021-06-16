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
    _expr.clear();
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
    /*auto return_size = [](const auto & value, const auto & _size){
        return (value == -1) ? _size : value;
    };*/

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

    ui->listWidget->setUpdatesEnabled(false);
    QStringList list;
    QVector<QByteArray> keysList;

    auto func = [&](const QVector<QByteArray> & bigList, const QVector<QByteArray> & list, QVector<QByteArray> & tempList){
        QVector<QVector<QByteArray>> fList, sList;

        for(const auto & it : bigList){
            _symptom.at(it.toStdString(), false);
            fList += repertoryEngine::getRootPath(_symptom);
        }

        for(const auto & ir : list){
            _symptom.at(ir.toStdString(), false);
            sList += repertoryEngine::getRootPath(_symptom);
        }

        for(auto it = bigList.constBegin(); it != bigList.constEnd(); ++it){
            for(auto ir = list.constBegin(); ir != list.constEnd(); ++ir){
                if(*it == *ir && tempList.indexOf(*ir) == -1){
                    tempList.push_back(*it);
                    qDebug() << "middle";
                }
                else if(*it < *ir && tempList.indexOf(*ir) == -1){
                    const auto iter = sList.at(ir - list.constBegin()).indexOf(*it);

                    if(iter != -1){
                        tempList.push_back(*ir);
                        qDebug() << "up";
                    }
                }
                else if(*it > *ir && tempList.indexOf(*it) == -1){
                    const auto iter = fList.at(it - bigList.constBegin()).indexOf(*ir);

                    if(iter != -1){
                        tempList.push_back(*it);
                        qDebug() << "down";
                    }
                }
            }
        }
    };

    for(auto mult = 0; mult != multiKeys.size(); ++mult){
        QVector<QByteArray> tempList;

        if(multiKeys.at(mult).second == operation::none)
            keysList.append(multiKeys.at(mult).first);

        else if(multiKeys.at(mult).second == operation::AND){
            if(multiKeys.at(mult).first.size() > keysList.size())
                func(keysList, multiKeys.at(mult).first, tempList);
            else
                func(multiKeys.at(mult).first, keysList, tempList);

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

    for(const auto & it : keysList){
        try{
            _symptom.at(it.toStdString(), false);
            list.push_back(repertoryEngine::renderingLabel(_symptom, false, _codec));
        }
        catch(std::exception){}
    }

    ui->listWidget->addItems(list);
    ui->label_2->setText(QString::number(list.size()));
    ui->listWidget->setUpdatesEnabled(true);
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
