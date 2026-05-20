#include "rest_server.h"

#include "../FoodDatabase.h"
#include "../Diary.h"
#include "../SavedMeal.h"
#include "../Food.h"
#include "../TrainingDiary.h"
#include "../TrainingSession.h"

#include <QDate>
#include <QHostAddress>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace {

QString dateKey(const QHttpServerRequest& request) {
    QUrlQuery query(request.url().query());
    QString dateStr = query.queryItemValue(QStringLiteral("date"));
    if (dateStr.isEmpty()) {
        dateStr = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    }
    return dateStr;
}

QJsonObject sessionToJson(const TrainingSession& s) {
    QJsonObject o;
    o[QStringLiteral("type")] = QString::fromStdString(s.getType());
    o[QStringLiteral("durationMin")] = s.getDurationMin();
    o[QStringLiteral("time")] = QString::fromStdString(s.getTimeHHmm());
    o[QStringLiteral("status")] = QString::fromStdString(TrainingSession::statusToString(s.getStatus()));
    o[QStringLiteral("notes")] = QString::fromStdString(s.getNotes());
    return o;
}

} // namespace

RestServer::RestServer(QObject* parent) : QObject(parent), foodDb_(new FoodDatabase()) {}

RestServer::~RestServer() {
    stop();
    delete foodDb_;
}

bool RestServer::start(quint16 port) {
    if (server_) {
        return false;
    }

    port_ = port;
    server_ = new QHttpServer(this);

    server_->route("/api/health", QHttpServerRequest::Method::Get,
                    [this](const QHttpServerRequest& req) { return handleHealth(req); });

    server_->route("/api/foods", QHttpServerRequest::Method::Get,
                    [this](const QHttpServerRequest& req) { return handleGetFoods(req); });

    server_->route("/api/foods/search", QHttpServerRequest::Method::Get,
                    [this](const QHttpServerRequest& req) { return handleSearchFoods(req); });

    server_->route("/api/diary", QHttpServerRequest::Method::Get,
                    [this](const QHttpServerRequest& req) { return handleGetDiary(req); });

    server_->route("/api/meals", QHttpServerRequest::Method::Post,
                    [this](const QHttpServerRequest& req) { return handlePostMeal(req); });

    server_->route("/api/meals", QHttpServerRequest::Method::Delete,
                    [this](const QHttpServerRequest& req) { return handleDeleteMeal(req); });

    server_->route("/api/training", QHttpServerRequest::Method::Get,
                    [this](const QHttpServerRequest& req) { return handleGetTraining(req); });

    server_->route("/api/training/session", QHttpServerRequest::Method::Post,
                    [this](const QHttpServerRequest& req) { return handlePostTrainingSession(req); });

    server_->route("/api/stats/week", QHttpServerRequest::Method::Get,
                    [this](const QHttpServerRequest& req) { return handleWeekStats(req); });

    return server_->listen(QHostAddress::Any, port_);
}

void RestServer::stop() {
    if (server_) {
        server_->deleteLater();
        server_ = nullptr;
    }
}

QHttpServerResponse RestServer::jsonResponse(const QJsonDocument& doc, int status) {
    return QHttpServerResponse(doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode(status));
}

QHttpServerResponse RestServer::errorResponse(const QString& message, int status) {
    QJsonObject o;
    o[QStringLiteral("error")] = message;
    return jsonResponse(QJsonDocument(o), status);
}

QHttpServerResponse RestServer::handleHealth(const QHttpServerRequest&) {
    QJsonObject o;
    o[QStringLiteral("status")] = QStringLiteral("ok");
    o[QStringLiteral("service")] = QStringLiteral("CalorieCalc API");
    o[QStringLiteral("version")] = 1;
    return jsonResponse(QJsonDocument(o));
}

QHttpServerResponse RestServer::handleGetFoods(const QHttpServerRequest&) {
    QJsonArray foodsArray;
    for (const auto& food : foodDb_->getAllFoods()) {
        QJsonObject foodObj;
        foodObj[QStringLiteral("name")] = QString::fromStdString(food.getName());
        foodObj[QStringLiteral("calories")] = food.getCalories();
        foodObj[QStringLiteral("carbs")] = food.getCarbs();
        foodObj[QStringLiteral("protein")] = food.getProtein();
        foodObj[QStringLiteral("fat")] = food.getFat();
        foodsArray.append(foodObj);
    }

    QJsonObject response;
    response[QStringLiteral("foods")] = foodsArray;
    response[QStringLiteral("count")] = foodsArray.size();
    return jsonResponse(QJsonDocument(response));
}

QHttpServerResponse RestServer::handleSearchFoods(const QHttpServerRequest& request) {
    QUrlQuery query(request.url().query());
    const QString q = query.queryItemValue(QStringLiteral("q"));
    if (q.trimmed().isEmpty()) {
        return errorResponse(QStringLiteral("Missing query parameter q"), 400);
    }

    const auto results = foodDb_->searchFoodsParallel(q.toStdString(), 0);
    QJsonArray arr;
    for (const auto& food : results) {
        QJsonObject foodObj;
        foodObj[QStringLiteral("name")] = QString::fromStdString(food.getName());
        foodObj[QStringLiteral("calories")] = food.getCalories();
        arr.append(foodObj);
    }

    QJsonObject response;
    response[QStringLiteral("query")] = q;
    response[QStringLiteral("results")] = arr;
    response[QStringLiteral("count")] = arr.size();
    return jsonResponse(QJsonDocument(response));
}

QHttpServerResponse RestServer::handleGetDiary(const QHttpServerRequest& request) {
    const QString dateStr = dateKey(request);
    Diary& diary = diaries_[dateStr];

    QJsonObject response;
    response[QStringLiteral("date")] = dateStr;
    response[QStringLiteral("calorieGoal")] = diary.getCalorieGoal();
    response[QStringLiteral("totalCalories")] = diary.getTotalCalories();
    response[QStringLiteral("waterMl")] = diary.getWaterMl();
    response[QStringLiteral("waterGoalMl")] = diary.getWaterGoalMl();
    response[QStringLiteral("weightKg")] = diary.getWeightKg();

    QJsonArray mealsArray;
    for (const auto& meal : diary.getAllMeals()) {
        QJsonObject mealObj;
        mealObj[QStringLiteral("mealName")] = QString::fromStdString(meal.getMealName());
        mealObj[QStringLiteral("foodName")] = QString::fromStdString(meal.getFood().getName());
        mealObj[QStringLiteral("amount")] = meal.getFood().getAmount();
        mealObj[QStringLiteral("calories")] = meal.getTotalCalories();
        mealsArray.append(mealObj);
    }
    response[QStringLiteral("meals")] = mealsArray;

    return jsonResponse(QJsonDocument(response));
}

QHttpServerResponse RestServer::handlePostMeal(const QHttpServerRequest& request) {
    const QJsonDocument doc = QJsonDocument::fromJson(request.body());
    if (doc.isNull() || !doc.isObject()) {
        return errorResponse(QStringLiteral("Invalid JSON"), 400);
    }

    const QJsonObject obj = doc.object();
    QString dateStr = obj.value(QStringLiteral("date")).toString();
    if (dateStr.isEmpty()) {
        dateStr = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    }

    const QString mealName = obj.value(QStringLiteral("mealName")).toString();
    const QString foodName = obj.value(QStringLiteral("foodName")).toString();
    const double amount = obj.value(QStringLiteral("amount")).toDouble(100.0);

    const auto foods = foodDb_->searchFoods(foodName.toStdString());
    if (foods.empty()) {
        return errorResponse(QStringLiteral("Food not found"), 404);
    }

    Food food = foods[0];
    food.setAmount(amount);
    SavedMeal meal(mealName.toStdString(), food, amount);
    diaries_[dateStr].addMeal(meal);

    QJsonObject response;
    response[QStringLiteral("status")] = QStringLiteral("success");
    response[QStringLiteral("date")] = dateStr;
    return jsonResponse(QJsonDocument(response), 201);
}

QHttpServerResponse RestServer::handleDeleteMeal(const QHttpServerRequest& request) {
    QUrlQuery query(request.url().query());
    const QString dateStr = query.queryItemValue(QStringLiteral("date"));
    const int index = query.queryItemValue(QStringLiteral("index")).toInt();

    if (dateStr.isEmpty() || !diaries_.contains(dateStr)) {
        return errorResponse(QStringLiteral("Diary not found"), 404);
    }

    Diary& diary = diaries_[dateStr];
    if (index < 0 || index >= static_cast<int>(diary.getMealsCount())) {
        return errorResponse(QStringLiteral("Invalid index"), 400);
    }

    diary.removeMeal(index);

    QJsonObject response;
    response[QStringLiteral("status")] = QStringLiteral("success");
    return jsonResponse(QJsonDocument(response));
}

QHttpServerResponse RestServer::handleGetTraining(const QHttpServerRequest& request) {
    const QString dateStr = dateKey(request);
    const TrainingDiary& td = trainings_[dateStr];

    QJsonArray sessions;
    for (const auto& s : td.getAllSessions()) {
        sessions.append(sessionToJson(s));
    }

    QJsonObject response;
    response[QStringLiteral("date")] = dateStr;
    response[QStringLiteral("totalDurationMin")] = td.getTotalDurationMin();
    response[QStringLiteral("sessions")] = sessions;
    return jsonResponse(QJsonDocument(response));
}

QHttpServerResponse RestServer::handlePostTrainingSession(const QHttpServerRequest& request) {
    const QJsonDocument doc = QJsonDocument::fromJson(request.body());
    if (doc.isNull() || !doc.isObject()) {
        return errorResponse(QStringLiteral("Invalid JSON"), 400);
    }

    const QJsonObject obj = doc.object();
    QString dateStr = obj.value(QStringLiteral("date")).toString();
    if (dateStr.isEmpty()) {
        dateStr = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    }

    const QString type = obj.value(QStringLiteral("type")).toString(QStringLiteral("Cardio"));
    const int duration = obj.value(QStringLiteral("durationMin")).toInt(30);
    const QString time = obj.value(QStringLiteral("time")).toString(QStringLiteral("07:30"));
    const QString statusStr = obj.value(QStringLiteral("status")).toString(QStringLiteral("Planned"));
    const QString notes = obj.value(QStringLiteral("notes")).toString();

    trainings_[dateStr].addSession(TrainingSession(type.toStdString(),
                                                   duration,
                                                   time.toStdString(),
                                                   TrainingSession::statusFromString(statusStr.toStdString()),
                                                   notes.toStdString()));

    QJsonObject response;
    response[QStringLiteral("status")] = QStringLiteral("success");
    response[QStringLiteral("date")] = dateStr;
    response[QStringLiteral("totalDurationMin")] = trainings_[dateStr].getTotalDurationMin();
    return jsonResponse(QJsonDocument(response), 201);
}

QHttpServerResponse RestServer::handleWeekStats(const QHttpServerRequest& request) {
    QUrlQuery query(request.url().query());
    QDate end = QDate::currentDate();
    const QString endStr = query.queryItemValue(QStringLiteral("end"));
    if (!endStr.isEmpty()) {
        end = QDate::fromString(endStr, QStringLiteral("yyyy-MM-dd"));
    }

    QJsonArray days;
    double totalCalories = 0.0;
    int totalTrainingMin = 0;

    for (int i = 6; i >= 0; --i) {
        const QDate day = end.addDays(-i);
        const QString key = day.toString(QStringLiteral("yyyy-MM-dd"));

        QJsonObject dayObj;
        dayObj[QStringLiteral("date")] = key;
        dayObj[QStringLiteral("calories")] = diaries_.value(key).getTotalCalories();
        dayObj[QStringLiteral("trainingMinutes")] = trainings_.value(key).getTotalDurationMin();

        totalCalories += dayObj.value(QStringLiteral("calories")).toDouble();
        totalTrainingMin += dayObj.value(QStringLiteral("trainingMinutes")).toInt();
        days.append(dayObj);
    }

    QJsonObject response;
    response[QStringLiteral("days")] = days;
    response[QStringLiteral("totalCalories")] = totalCalories;
    response[QStringLiteral("totalTrainingMinutes")] = totalTrainingMin;
    return jsonResponse(QJsonDocument(response));
}
