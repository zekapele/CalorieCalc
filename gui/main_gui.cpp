#include <QApplication>
#include <QDialog>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include "LoginDialog.h"
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // i18n: завантаження перекладів
    QTranslator translator;
    QString locale = QLocale::system().name();
    if (translator.load("caloriecounter_" + locale, ":/translations")) {
        app.installTranslator(&translator);
    }
    
    LoginDialog loginDlg;
    if (loginDlg.exec() != QDialog::Accepted) {
        return 0;
    }
    const QString userLogin = loginDlg.login();
    if (userLogin.isEmpty()) {
        return 0;
    }

    MainWindow w(userLogin);
    w.setWindowTitle(QApplication::translate("MainWindow", "calorie counter") + QStringLiteral(" — ") + userLogin);
    w.resize(980, 640);
    w.showMaximized();
    return app.exec();
}

