#include "mainwindow.h"
#include <QApplication>
#include <QtGlobal>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qSetMessagePattern("%{type}\t%{qthreadptr}\t%{message}");
    // a.setStyleSheet()
    MainWindow w;

    w.show();

    return a.exec();
}
