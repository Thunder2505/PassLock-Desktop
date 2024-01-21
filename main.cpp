#include "passlock.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    PassLock w;
    w.show();
    w.ResizeTabs();
    w.ShowPasswords();
    w.QuickView();
    return a.exec();
}
