#include <QtWidgets/QApplication>
#include "etriggermain.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    eTriggerMain w;
    w.show();
    return a.exec();
}
