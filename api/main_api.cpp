#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "rest_server.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("CalorieCalcAPI"));
    app.setApplicationVersion(QStringLiteral("1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("CalorieCalc REST API (NFR-2 load testing)"));
    parser.addHelpOption();
    QCommandLineOption portOpt(QStringList{QStringLiteral("p"), QStringLiteral("port")},
                               QStringLiteral("Listen port (default 8080)"),
                               QStringLiteral("port"),
                               QStringLiteral("8080"));
    parser.addOption(portOpt);
    parser.process(app);

    const quint16 port = static_cast<quint16>(parser.value(portOpt).toUShort());

    RestServer server;
    if (!server.start(port)) {
        qCritical() << "Failed to start REST API on port" << port;
        return 1;
    }

    qInfo() << "CalorieCalc API listening on http://127.0.0.1:" << server.port();
    qInfo() << "  GET  /api/health";
    qInfo() << "  GET  /api/foods";
    qInfo() << "  GET  /api/foods/search?q=...";
    qInfo() << "  GET  /api/diary?date=YYYY-MM-DD";
    qInfo() << "  POST /api/meals";
    qInfo() << "  DELETE /api/meals?date=...&index=0";
    qInfo() << "  GET  /api/training?date=YYYY-MM-DD";
    qInfo() << "  POST /api/training/session";
    qInfo() << "  GET  /api/stats/week?end=YYYY-MM-DD";

    return app.exec();
}
