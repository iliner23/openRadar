#include "keyreader.h"
#include "ui_keyreader.h"

keyReader::keyReader(QByteArray text, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::keyReader)
{
    ui->setupUi(this);
    _text = text;
    _codec = QTextCodec::codecForName(languages::systemCodec());
    ui->plainTextEdit->setPlainText(_codec->toUnicode(text));
    connect(ui->pushButton_3, &QPushButton::clicked, this, &keyReader::setCodec);
}
void keyReader::setCodec(){
    using namespace languages;
    QStringList languages;

    for(auto & it : languages::radarLang)
        languages += QLocale(it).nativeLanguageName();

    bool ok;
    const auto codec =
            QInputDialog::getItem(this, "Выберите язык",
                    "Выберите язык отображения репертория", languages, 0, false, &ok);

    const auto pos = languages.indexOf(codec);

    if(ok){
        _codec = QTextCodec::codecForName(
                    languageToName(radarLang.at(pos)));
        ui->plainTextEdit->setPlainText(_codec->toUnicode(_text));
    }
}
keyReader::~keyReader(){
    delete ui;
}
