#include "launcher.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Launcher w;
    
    w.initialSignals();
    w.resize(640, 480);
    w.show();
    
    return a.exec();
}
