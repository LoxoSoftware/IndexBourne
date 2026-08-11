#include "mainwindow.h"

#include <QApplication>

MainWindow* main_window;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    a.setApplicationDisplayName("IndexBourne");
    main_window= &w;
    w.show();
    return QApplication::exec();
}
