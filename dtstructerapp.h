#ifndef DTSTRUCTERAPP_H
#define DTSTRUCTERAPP_H

#include <QApplication>
#include "mainwindow.h"

class DtStructerApp : public QApplication
{

public:
    DtStructerApp(int argc, char *argv[]);

private:
    MainWindow window;
};

#endif // DTSTRUCTERAPP_H
