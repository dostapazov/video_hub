/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QLabel *FrameNo;
    QLabel *label_3;
    QLabel *LostFrames;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_5;
    QLabel *LabelVersion;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(649, 389);
        QFont font;
        font.setPointSize(32);
        font.setBold(true);
        font.setItalic(false);
        MainWindow->setFont(font);
        MainWindow->setFocusPolicy(Qt::StrongFocus);
        MainWindow->setStyleSheet(QLatin1String("background-color: rgb(0, 0, 0);\n"
"color: rgb(255, 255, 255);"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setFont(font);
        label->setStyleSheet(QStringLiteral(""));
        label->setTextFormat(Qt::PlainText);
        label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);
        QFont font1;
        font1.setPointSize(18);
        font1.setBold(true);
        font1.setItalic(false);
        label_2->setFont(font1);

        horizontalLayout->addWidget(label_2);

        FrameNo = new QLabel(centralWidget);
        FrameNo->setObjectName(QStringLiteral("FrameNo"));
        FrameNo->setFont(font1);
        FrameNo->setStyleSheet(QStringLiteral(""));
        FrameNo->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(FrameNo);

        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QStringLiteral("label_3"));
        QFont font2;
        font2.setPointSize(18);
        label_3->setFont(font2);

        horizontalLayout->addWidget(label_3);

        LostFrames = new QLabel(centralWidget);
        LostFrames->setObjectName(QStringLiteral("LostFrames"));
        LostFrames->setFont(font2);

        horizontalLayout->addWidget(LostFrames);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        label_5 = new QLabel(centralWidget);
        label_5->setObjectName(QStringLiteral("label_5"));
        QFont font3;
        font3.setFamily(QStringLiteral("Liberation Serif"));
        font3.setPointSize(14);
        font3.setItalic(true);
        label_5->setFont(font3);

        horizontalLayout->addWidget(label_5);

        LabelVersion = new QLabel(centralWidget);
        LabelVersion->setObjectName(QStringLiteral("LabelVersion"));
        LabelVersion->setFont(font3);

        horizontalLayout->addWidget(LabelVersion);


        verticalLayout->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", Q_NULLPTR));
        label->setText(QApplication::translate("MainWindow", "Startup system", Q_NULLPTR));
        label_2->setText(QApplication::translate("MainWindow", "Frames :", Q_NULLPTR));
        FrameNo->setText(QApplication::translate("MainWindow", "-", Q_NULLPTR));
        label_3->setText(QApplication::translate("MainWindow", "Lost: ", Q_NULLPTR));
        LostFrames->setText(QApplication::translate("MainWindow", "-", Q_NULLPTR));
        label_5->setText(QApplication::translate("MainWindow", "Version:", Q_NULLPTR));
        LabelVersion->setText(QApplication::translate("MainWindow", "2.0", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
