#include "windowchapters.h"
#include "ui_windowchapters.h"

windowChapters::windowChapters(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::windowChapters)
{
    ui->setupUi(this);
    _codec = QTextCodec::codecForName("system");
    ui->groupBox->setLayout(ui->horizontalLayout_3);
    setLayout(ui->verticalLayout);
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::activated), this, &windowChapters::windowChanged);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(130);
    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(80);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setItemDelegate(new delegate);
    ui->tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
}

windowChapters::~windowChapters()
{
    delete ui;
}

void windowChapters::getWindows(QList<QMdiSubWindow*> win, QMdiSubWindow * mdiSub){
    ui->comboBox->clear();
    _dirPaths.clear();

    for(auto it = 0; it != win.size(); ++it){
        ui->comboBox->addItem(win.at(it)->windowTitle());

        if(win.at(it) == mdiSub){
            ui->comboBox->setCurrentIndex(it);
        }

        _dirPaths.push_back(qobject_cast<repertory*>(win.at(it)->widget())->getRepDir());
    }
}
void windowChapters::windowChanged(int index){
    const int mx = 8;
    std::vector<QTableWidgetItem*> items;
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(mx);
    openCtree read(_dirPaths.at(index).filePath("symptom").toStdString());
    read.setMember(4);
    const std::string compr(6, '\0');
    quint16 x = 0, y = 0;

    while(true){
        auto temp = read.next();

        if(x > mx){
            ++y;
            x = 0;
        }

        if(temp.substr(12, 6) == compr){
            auto iter = std::find(temp.cbegin() + read.getReclen(), temp.cend(), '\0');
            std::string textOriginal(temp.cbegin() + read.getReclen(), iter);
            ++iter;
            std::string textLocalize(iter, std::find(iter, temp.cend(), '\0'));

            items.push_back(new QTableWidgetItem(QIcon(QPixmap(48, 48)),
                        QString::fromStdString(textOriginal + "\n") + _codec->toUnicode(textLocalize.c_str()) ));
            (*(--items.end()))->setTextAlignment(Qt::AlignHCenter);
            //TODO : picture add for debug only
            ++x;
        }
        else
            break;
    }

    if(y == 0 && !items.empty())
        ui->tableWidget->setRowCount(1);
    else
        ui->tableWidget->setRowCount(y);

    x = 0; y = 0;

    for(auto & it : items){
        if(x > mx){
            ++y;
            x = 0;
        }

        ui->tableWidget->setItem(y, x, it);
        ++x;
    }
}
void windowChapters::show(){
    if(ui->comboBox->count() != 0)
        windowChanged(ui->comboBox->currentIndex());

    QWidget::show();
}
