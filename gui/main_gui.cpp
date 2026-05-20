#include <QApplication>
#include <QCoreApplication>
#include <QDialog>
#include <QFile>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QLocale>
#include <QStandardPaths>
#include <QScreen>
#include <QStyleFactory>
#include <QTranslator>
#include <QWindow>
#include "AppStyle.h"
#include "LoginDialog.h"
#include "MainWindow.h"
#include "../Logger.h"

namespace {

void installUkTranslations(QApplication& app) {
    QLocale::setDefault(QLocale(QLocale::Ukrainian, QLocale::Ukraine));

    QTranslator qtBase;
    if (qtBase.load(QLocale(QLocale::Ukrainian, QLocale::Ukraine),
                    QStringLiteral("qtbase"),
                    QStringLiteral("translations"),
                    QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtBase);
    }

    static QTranslator appTranslator;
    const QStringList qmPaths = {
        QStringLiteral(":/i18n"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../Resources/translations"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../translations"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/translations"),
    };
    for (const QString& path : qmPaths) {
        if (appTranslator.load(QStringLiteral("caloriecounter_uk"), path)) {
            app.installTranslator(&appTranslator);
            return;
        }
    }
}

#ifdef Q_OS_MAC
/**
 * Після видалення часткового Contents/PlugIns (CODESIGNING) exe з Homebrew не знаходить cocoa.
 * Якщо в bundle немає libqcocoa — виставляємо QT_PLUGIN_PATH на каталог плагінів інсталяції Qt.
 */
void ensureQtPluginsPathForMacDev(char *argv0) {
    const QFileInfo fi(QString::fromLocal8Bit(argv0));
    const QString binDir = fi.absolutePath();
    const QString bundledCocoa =
        QDir(binDir).filePath(QStringLiteral("../PlugIns/platforms/libqcocoa.dylib"));
    if (QFile::exists(bundledCocoa)) {
        return;
    }
    if (!qgetenv("QT_PLUGIN_PATH").isEmpty()) {
        return;
    }
    QString plugins = QLibraryInfo::path(QLibraryInfo::PluginsPath);
    if (plugins.isEmpty() || !QFileInfo(plugins).isDir()) {
        const QByteArray brew = qgetenv("HOMEBREW_PREFIX");
        const QString pfx =
            brew.isEmpty() ? QStringLiteral("/opt/homebrew") : QString::fromUtf8(brew);
        const QStringList candidates = {
            pfx + QStringLiteral("/opt/qtbase/share/qt/plugins"),
            pfx + QStringLiteral("/share/qt/plugins"),
            QStringLiteral("/opt/homebrew/opt/qtbase/share/qt/plugins"),
            QStringLiteral("/usr/local/opt/qtbase/share/qt/plugins"),
            QStringLiteral("/usr/local/share/qt/plugins"),
        };
        for (const QString &p : candidates) {
            if (QFileInfo(p).isDir()) {
                plugins = p;
                break;
            }
        }
    }
    if (!plugins.isEmpty()) {
        qputenv("QT_PLUGIN_PATH", QFile::encodeName(plugins));
    }
}
#endif

QString resolveWorkingDirectory() {
    const QByteArray env = qgetenv("CALORIECALC_DATA_DIR");
    if (!env.isEmpty()) {
        return QString::fromUtf8(env);
    }
    const QString cwd = QDir::currentPath();
    const bool cwdInsideBundle = cwd.contains(QStringLiteral(".app/Contents"));
    if (!cwdInsideBundle && cwd != QStringLiteral("/") && QFileInfo(cwd).isWritable()) {
        return cwd;
    }
    QString root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (root.isEmpty()) {
        root = QDir::homePath() + QStringLiteral("/Library/Application Support/CalorieCalc");
    }
    return root;
}

} // namespace

int main(int argc, char* argv[]) {
#ifdef Q_OS_MAC
    ensureQtPluginsPathForMacDev(argv[0]);
#endif
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("com.caloriecalc"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("caloriecalc.app"));
    app.setApplicationName(QStringLiteral("CalorieCalc"));
    app.setApplicationDisplayName(QStringLiteral("CalorieCalc"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

#if !defined(Q_OS_MAC)
    if (QStyle* fusion = QStyleFactory::create(QStringLiteral("Fusion"))) {
        app.setStyle(fusion);
    }
#endif
    app.setQuitOnLastWindowClosed(true);
    AppStyle::applyApplicationFont(&app);
    const bool darkTheme = QSettings().value(QStringLiteral("ui/darkTheme"), true).toBool();
    // На macOS глобальний setStyleSheet до першого вікна може зависати (Dock bounce).
    AppStyle::applyPalette(&app, darkTheme);
    installUkTranslations(app);

    const QString workDir = resolveWorkingDirectory();
    QDir().mkpath(workDir);
    QDir().mkpath(workDir + QStringLiteral("/data"));
    QDir::setCurrent(workDir);

    Logger::getInstance().init("data/caloriecalc_gui.log");
    Logger::getInstance().setMinLevel(Logger::Level::INFO);
    LOG_INFO("CalorieCalc GUI starting");

    LoginDialog loginDlg;
    loginDlg.setStyleSheet(AppStyle::stylesheet(darkTheme));
    if (QScreen* screen = QApplication::primaryScreen()) {
        const QRect avail = screen->availableGeometry();
        loginDlg.adjustSize();
        const QSize sz = loginDlg.size();
        loginDlg.move(avail.center() - QPoint(sz.width() / 2, sz.height() / 2));
    }
    if (loginDlg.exec() != QDialog::Accepted) {
        return 0;
    }
    AppStyle::applyStylesheet(&app, darkTheme);
    const QString userLogin = loginDlg.login();
    if (userLogin.isEmpty()) {
        return 0;
    }

    MainWindow w(userLogin);
    w.setWindowTitle(QStringLiteral("CalorieCalc — %1").arg(userLogin));
    w.setMinimumSize(900, 620);
    w.resize(1100, 720);
    w.show();
    w.raise();
    w.activateWindow();
    if (QWindow* wh = w.windowHandle()) {
        wh->requestActivate();
    }
    return app.exec();
}
