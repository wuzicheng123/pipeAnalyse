#include "mainwindow.h"
#include <QApplication>
#include "dataservice.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.hideForm();
    dataService::getInstance()->setMainWindow(&w);

    return a.exec();
}
