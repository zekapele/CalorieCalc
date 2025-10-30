#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.setWindowTitle("calorie counter");
    w.resize(980, 640);
    w.show();
    return app.exec();
}

