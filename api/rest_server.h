#ifndef CALORIECALC_REST_SERVER_H
#define CALORIECALC_REST_SERVER_H

#include <QObject>
#include <QString>
#include <QMap>

class QHttpServer;
class QHttpServerRequest;
class QHttpServerResponse;
class FoodDatabase;
class Diary;
class TrainingDiary;

/**
 * @brief REST API для CalorieCalc (харчування, тренування, health).
 * Призначений для інтеграцій і навантажувального тестування (NFR-2).
 */
class RestServer : public QObject {
    Q_OBJECT
public:
    explicit RestServer(QObject* parent = nullptr);
    ~RestServer();

    bool start(quint16 port = 8080);
    void stop();
    quint16 port() const { return port_; }

private:
    QHttpServer* server_{nullptr};
    quint16 port_{8080};
    FoodDatabase* foodDb_{nullptr};
    QMap<QString, Diary> diaries_;
    QMap<QString, TrainingDiary> trainings_;

    static QHttpServerResponse jsonResponse(const QJsonDocument& doc, int status = 200);
    static QHttpServerResponse errorResponse(const QString& message, int status);

    QHttpServerResponse handleHealth(const QHttpServerRequest& request);
    QHttpServerResponse handleGetFoods(const QHttpServerRequest& request);
    QHttpServerResponse handleSearchFoods(const QHttpServerRequest& request);
    QHttpServerResponse handleGetDiary(const QHttpServerRequest& request);
    QHttpServerResponse handlePostMeal(const QHttpServerRequest& request);
    QHttpServerResponse handleDeleteMeal(const QHttpServerRequest& request);
    QHttpServerResponse handleGetTraining(const QHttpServerRequest& request);
    QHttpServerResponse handlePostTrainingSession(const QHttpServerRequest& request);
    QHttpServerResponse handleWeekStats(const QHttpServerRequest& request);
};

#endif // CALORIECALC_REST_SERVER_H
