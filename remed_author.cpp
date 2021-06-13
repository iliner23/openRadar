#include "remed_author.h"
#include "ui_remed_author.h"

remed_author::remed_author(const QDir path, std::shared_ptr<cache> ch,
                           const QByteArray pos,
                           const quint16 remFilter, const quint32 localPos, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::remed_author)
{
    ui->setupUi(this);
    _path = path;
    _pos = pos;
    _remFilter = remFilter;
    _localPos = localPos;
    _cache = ch;

    _sym.open(path.filePath("symptom").toStdString());

    ui->listWidget_2->addItem("Авторы");
    ui->listWidget_2->addItem("Семейства");
    ui->label->setFont(QFont("defalut", 10));
    ui->label_2->setFont(QFont("defalut", 10));
    rendering();
    connect(ui->listWidget_2, &QListWidget::itemClicked, this, &remed_author::showTextInformation);

    ui->listWidget_2->setCurrentRow(0);
    emit ui->listWidget_2->itemClicked(ui->listWidget_2->item(0));
}

void remed_author::showTextInformation(QListWidgetItem * item){
    ui->listWidget->clear();
    switch (ui->listWidget_2->row(item)){
    case 0:{
        for(const auto & it : qAsConst(_authorsText))
            ui->listWidget->addItem(it);
        break;
    }
    case 1:
        break;
    }
}
void remed_author::rendering(){
    auto * codec = QTextCodec::codecForName("system");
    //remed

    auto authorTxt = [&](const auto & text){
        QString tmp;
        auto itA = std::find(text.cbegin() + _cache->_lenAuthor, text.cend(), '\0');
        tmp += "(" + QString::fromStdString(std::string(text.cbegin() + _cache->_lenAuthor, itA)) + ")";

        if(itA != text.cend()){
            ++itA;
            auto itB = std::find(itA, text.cend(), '\0');
            tmp += " " + codec->toUnicode(std::string(itA, itB).c_str());
            itA = itB;
        }
        if(itA != text.cend()){
            ++itA;
            auto itB = std::find(itA, text.cend(), '\0');
            tmp += "\n" + codec->toUnicode(std::string(itA, itB).c_str());
            itA = itB;
        }

        _authorsText.push_back(std::move(tmp));
    };

    auto remedTxt = [&](const auto & text){
        QString tmp;
        auto itA = std::find(text.cbegin() + _cache->_lenRem, text.cend(), '\0');
        tmp += QString::fromStdString(std::string(text.cbegin() + _cache->_lenRem, itA));

        if(itA != text.cend()){
            ++itA;
            auto itB = std::find(itA, text.cend(), '\0');
            tmp += " - " + QString::fromStdString(std::string(itA, itB).c_str());
        }

        return tmp;
    };

    quint16 prevRem = 0;
    const auto symptom = QByteArray::fromStdString(_sym.at(_pos.toStdString()));

    while(_localPos < symptom.size()){
        quint16 remed = 0, author = 0, tLevel = 0;
        uint8_t rLevel;

        if(symptom.size() - _localPos <= 3)
            break;

        remed = qFromLittleEndian<quint16>(symptom.constData() + _localPos);
        _localPos += 2;
        rLevel = qFromLittleEndian<quint16>(symptom.constData() + _localPos);
        ++_localPos;
        author = qFromLittleEndian<quint16>(symptom.constData() + _localPos);
        _localPos += 2;
        tLevel = qFromLittleEndian<quint16>(symptom.constData() + _localPos);
        _localPos += 2;

        if(memcmp(&remed, "\x5a\x5a", 2) == 0 && rLevel == '\x5a')
            break;

        if(prevRem != 0 && remed != prevRem)
            break;

        if(_remFilter == (quint16)-1 || (tLevel & _remFilter) != 0){
            if((((char *)&author)[1] & (char)128) != 0){
                ((char *)&author)[1] ^= (char)128;//remed have * in the end
            }

            const auto & rmStr = _cache->_cacheRemed.at(remed);
            const auto & auStr = _cache->_cacheAuthor.at(author);

            if(prevRem != remed)
                ui->label_2->setText(remedTxt(rmStr));

            authorTxt(auStr);
            prevRem = remed;
        }
    }

    ui->label->setText(repertoryEngine::renderingLabel(_sym, false));
}
remed_author::~remed_author()
{
    delete ui;
}
