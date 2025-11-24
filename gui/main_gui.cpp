#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // i18n: завантаження перекладів
    QTranslator translator;
    QString locale = QLocale::system().name();
    if (translator.load("caloriecounter_" + locale, ":/translations")) {
        app.installTranslator(&translator);
    }
    
    MainWindow w;
    w.setWindowTitle(QApplication::translate("MainWindow", "calorie counter"));
    w.resize(980, 640);
    w.show();
    return app.exec();
}

