#include <QCoreApplication>
#include <QDebug>
#include "rest_server.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    RestServer server;
    if (!server.start(8080)) {
        qCritical() << "Failed to start REST API server";
        return 1;
    }
    
    qInfo() << "REST API server started on http://localhost:8080";
    qInfo() << "Endpoints:";
    qInfo() << "  GET  /api/foods - Get all foods";
    qInfo() << "  GET  /api/diary?date=YYYY-MM-DD - Get diary for date";
    qInfo() << "  POST /api/meals - Add meal (JSON body)";
    qInfo() << "  DELETE /api/meals?id=ID - Delete meal";
    
    return app.exec();
}
