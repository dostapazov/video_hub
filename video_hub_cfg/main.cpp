#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPointer<MainWindow> w(MainWindow::get_instance());
    w->show();
    return  a.exec();
}
