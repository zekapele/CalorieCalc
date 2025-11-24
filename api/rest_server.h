#ifndef CALORIECALC_REST_SERVER_H
#define CALORIECALC_REST_SERVER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QDate>

class QHttpServer;
class QHttpServerRequest;
class QHttpServerResponse;
class FoodDatabase;
class Diary;

/**
 * @brief Простий REST API сервер для доступу до даних
 * 
 * Надає HTTP API для інтеграції з іншими додатками
 */
class RestServer : public QObject {
    Q_OBJECT
public:
    explicit RestServer(QObject* parent = nullptr);
    ~RestServer();
    
    bool start(quint16 port = 8080);
    void stop();

private:
    QHttpServer* server_;
    FoodDatabase* foodDb_;
    QMap<QString, Diary> diaries_;
    
    QHttpServerResponse handleGetFoods(const QHttpServerRequest& request);
    QHttpServerResponse handleGetDiary(const QHttpServerRequest& request);
    QHttpServerResponse handlePostMeal(const QHttpServerRequest& request);
    QHttpServerResponse handleDeleteMeal(const QHttpServerRequest& request);
};

#endif //CALORIECALC_REST_SERVER_H
