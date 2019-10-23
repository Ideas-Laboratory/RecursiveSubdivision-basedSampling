/********************************************************************************
** Form generated from reading UI file 'qt_gui.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QT_GUI_H
#define UI_QT_GUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Qt_GUIClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *Qt_GUIClass)
    {
        if (Qt_GUIClass->objectName().isEmpty())
            Qt_GUIClass->setObjectName(QString::fromUtf8("Qt_GUIClass"));
        Qt_GUIClass->resize(600, 400);
        menuBar = new QMenuBar(Qt_GUIClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        Qt_GUIClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(Qt_GUIClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        Qt_GUIClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(Qt_GUIClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        Qt_GUIClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(Qt_GUIClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        Qt_GUIClass->setStatusBar(statusBar);

        retranslateUi(Qt_GUIClass);

        QMetaObject::connectSlotsByName(Qt_GUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *Qt_GUIClass)
    {
        Qt_GUIClass->setWindowTitle(QApplication::translate("Qt_GUIClass", "Qt_GUI", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Qt_GUIClass: public Ui_Qt_GUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QT_GUI_H
