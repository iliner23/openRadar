/********************************************************************************
** Form generated from reading UI file 'repertory.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REPERTORY_H
#define UI_REPERTORY_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Repertory
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QTextBrowser *textBrowser;
    QMenuBar *menubar;
    QMenu *menu;

    void setupUi(QMainWindow *Repertory)
    {
        if (Repertory->objectName().isEmpty())
            Repertory->setObjectName(QString::fromUtf8("Repertory"));
        Repertory->resize(800, 600);
        centralwidget = new QWidget(Repertory);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        textBrowser = new QTextBrowser(centralwidget);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        textBrowser->setReadOnly(true);
        textBrowser->setOpenLinks(false);

        horizontalLayout->addWidget(textBrowser);

        Repertory->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Repertory);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 25));
        menu = new QMenu(menubar);
        menu->setObjectName(QString::fromUtf8("menu"));
        Repertory->setMenuBar(menubar);

        menubar->addAction(menu->menuAction());

        retranslateUi(Repertory);

        QMetaObject::connectSlotsByName(Repertory);
    } // setupUi

    void retranslateUi(QMainWindow *Repertory)
    {
        Repertory->setWindowTitle(QCoreApplication::translate("Repertory", "MainWindow", nullptr));
        menu->setTitle(QCoreApplication::translate("Repertory", "\320\243\321\200\320\276\320\262\320\275\320\270", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Repertory: public Ui_Repertory {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REPERTORY_H
