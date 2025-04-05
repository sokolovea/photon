#include <QApplication>
#include <QIcon>
#include "windowbmp24.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/resourses/icon.ico"));
    app.setApplicationName(QString("Учебная программа для целочисленного масштабирования 24-битных изображений BMP"));

    WindowBmp24 windowBmp24 = WindowBmp24();
    windowBmp24.show();
    return app.exec();
}
