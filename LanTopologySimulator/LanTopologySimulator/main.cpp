#include "LanTopologySimulator.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    LanTopologySimulator window;
    window.show();
    return app.exec();
}
