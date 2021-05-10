#include "remed_author.h"
#include "ui_remed_author.h"

remed_author::remed_author(const QDir & path, const QDir & system, std::shared_ptr<cache> & ch,
                           const QByteArray & pos,
                           const quint16 remFilter, const quint32 localPos, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::remed_author)
{
    ui->setupUi(this);
    setLayout(ui->gridLayout);
    setFixedSize(700, 700);
    _path = path;
    _system = system;
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
        for(const auto & it : _authorsText)
            ui->listWidget->addItem(it);
        break;
    }
    case 1:
        break;
    }
}
void remed_author::rendering(){
    auto * codec = QTextCodec::codecForName("system");

    {//remed
        auto pred = [](auto it){
            if(it != '\0')
                return true;
            return false;
        };

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
        qint8 type = 0;
        const auto symptom = _sym.at(_pos.toStdString());
        auto secondIt = symptom.cbegin() + _localPos;

        for(auto it = std::find_if(secondIt, symptom.cend(), pred); it != symptom.cend(); ++secondIt){
            if(*it == 0)
                break;

            quint16 remed = 0, author = 0, tLevel = 0;
            uint8_t rLevel;

            if(symptom.cend() - secondIt <= 3)
                break;

            ((char *)&remed)[0] = *secondIt;
            ((char *)&remed)[1] = *(++secondIt);

            rLevel = *(++secondIt);

            ((char *)&author)[0] = *(++secondIt);
            ((char *)&author)[1] = *(++secondIt);

            ((char *)&tLevel)[0] = *(++secondIt);
            ((char *)&tLevel)[1] = *(++secondIt);

            if(((char *)&remed)[0] == '\x5a' && ((char *)&remed)[1] == '\x5a' && rLevel == '\x5a')
                break;

            if(prevRem != 0 && remed != prevRem)
                break;

            if(_remFilter == (quint16)-1 || (tLevel & _remFilter) != 0){
                if((((char *)&author)[1] & (char)128) != 0){
                    ((char *)&author)[1] ^= (char)128;//remed have * in the end
                }

                const auto & rmStr = _cache->_cacheRemed.at(remed);
                const auto & auStr = _cache->_cacheAuthor.at(author);

                if(prevRem != remed){
                    type = rLevel;
                    switch (rLevel) {
                    case 1 :
                        //ui->label_2->setText(QString::fromStdString(rmStr.substr(_cache->_lenRem, itN - _cache->_lenRem)));
                        ui->label_2->setText(remedTxt(rmStr));
                        //rem->setDefaultTextColor(Qt::darkGreen);
                        //rem->setFont(QFont(fontName, 10));
                        break;
                    case 2 :
                        ui->label_2->setText(remedTxt(rmStr));
                        //rem->setDefaultTextColor(Qt::blue);
                        //rem->setFont(QFont(fontName, 10, -1, true));
                        break;
                    case 3 :
                        ui->label_2->setText(remedTxt(rmStr));
                        //rem->setDefaultTextColor(Qt::red);
                        //rem->setFont(QFont(fontName, 10, QFont::Bold));
                        break;
                    case 4 :
                        ui->label_2->setText(remedTxt(rmStr));
                        //rem->setDefaultTextColor(Qt::darkRed);
                        //rem->setFont(QFont(fontName, 10, QFont::Bold));
                        break;
                    default:
                        continue;
                    }
                    authorTxt(auStr);
                }
                else{
                    authorTxt(auStr);
                }
                prevRem = remed;
            }
        }
    }
    {
        //Chapters shower
        std::string text;
        QString endlab;
        QStringList orig, loz;
        std::string ind(6, '\0');
        bool fis = true;

        while(true){
            quint8 caption = 0;

            if(fis)
                text = _sym.at(_pos.toStdString());
            else
                text = _sym.at(ind);

            auto first = std::find(text.cbegin() + _sym.serviceDataLenght(), text.cend(), '\0');
            auto localize = std::string(first + 1, std::find(first + 1, text.cend(), '\0'));
            orig.push_back(codec->toUnicode(std::string(text.cbegin() + _sym.serviceDataLenght(), first).c_str()));

            if(!localize.empty())
                loz.push_back(codec->toUnicode(localize.c_str()));

            caption = text.at(21);

            for(auto it = 0; it != 6; ++it){
                ind[0 + it] = text.at(17 - it);
            }

            if(caption <= 1 || ind == "\0\0\0\0\0\0")
                break;

            fis = false;
        }

        if(!orig.isEmpty()){
            QString org, lz;

            for(auto it = orig.size() - 1; it != -1; --it){
                if(it == orig.size() - 1){
                    org += orig[it];
                }
                else{
                    org += " - " + orig[it];
                }
            }
            if(!loz.isEmpty()){
                for(auto it = loz.size() - 1; it != -1; --it){
                    if(it == loz.size() - 1){
                        lz += loz[it];
                    }
                    else{
                        lz += " - " + loz[it];
                    }
                }
            }

            endlab = org + ((loz.isEmpty()) ? "" : '\n' + lz);
        }

        ui->label->setText(endlab);
    }
}
remed_author::~remed_author()
{
    delete ui;
}
