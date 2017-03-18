#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("qpcopl");
    a.setApplicationVersion("0.0.1");
    a.setApplicationDisplayName("Open PlayStation 2 Loader PC Tools");
    a.setOrganizationName("brainstream");
    MainWindow w;
    w.show();

    return a.exec();
}
