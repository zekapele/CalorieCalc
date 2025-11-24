#include "rest_server.h"
#include "../FoodDatabase.h"
#include "../Diary.h"
#include "../SavedMeal.h"
#include "../Food.h"
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QHostAddress>
#include <QDate>

RestServer::RestServer(QObject* parent) : QObject(parent), server_(nullptr), foodDb_(new FoodDatabase()) {
}

RestServer::~RestServer() {
    stop();
    delete foodDb_;
}

bool RestServer::start(quint16 port) {
    if (server_) {
        return false;
    }
    
    server_ = new QHttpServer(this);
    
    // Routes
    server_->route("/api/foods", QHttpServerRequest::Method::Get, [this](const QHttpServerRequest& request) {
        return handleGetFoods(request);
    });
    
    server_->route("/api/diary", QHttpServerRequest::Method::Get, [this](const QHttpServerRequest& request) {
        return handleGetDiary(request);
    });
    
    server_->route("/api/meals", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest& request) {
        return handlePostMeal(request);
    });
    
    server_->route("/api/meals", QHttpServerRequest::Method::Delete, [this](const QHttpServerRequest& request) {
        return handleDeleteMeal(request);
    });
    
    return server_->listen(QHostAddress::Any, port);
}

void RestServer::stop() {
    if (server_) {
        server_->deleteLater();
        server_ = nullptr;
    }
}

QHttpServerResponse RestServer::handleGetFoods(const QHttpServerRequest& request) {
    QJsonArray foodsArray;
    auto allFoods = foodDb_->getAllFoods();
    
    for (const auto& food : allFoods) {
        QJsonObject foodObj;
        foodObj["name"] = QString::fromStdString(food.getName());
        foodObj["calories"] = food.getCalories();
        foodObj["carbs"] = food.getCarbs();
        foodObj["protein"] = food.getProtein();
        foodObj["fat"] = food.getFat();
        foodsArray.append(foodObj);
    }
    
    QJsonObject response;
    response["foods"] = foodsArray;
    
    return QHttpServerResponse(QJsonDocument(response), "application/json");
}

QHttpServerResponse RestServer::handleGetDiary(const QHttpServerRequest& request) {
    QUrlQuery query(request.url().query());
    QString dateStr = query.queryItemValue("date");
    
    if (dateStr.isEmpty()) {
        dateStr = QDate::currentDate().toString("yyyy-MM-dd");
    }
    
    Diary& diary = diaries_[dateStr];
    QJsonObject response;
    response["date"] = dateStr;
    response["calorieGoal"] = diary.getCalorieGoal();
    response["totalCalories"] = diary.getTotalCalories();
    response["waterMl"] = diary.getWaterMl();
    response["waterGoalMl"] = diary.getWaterGoalMl();
    response["weightKg"] = diary.getWeightKg();
    
    QJsonArray mealsArray;
    auto meals = diary.getAllMeals();
    for (const auto& meal : meals) {
        QJsonObject mealObj;
        mealObj["mealName"] = QString::fromStdString(meal.getMealName());
        mealObj["foodName"] = QString::fromStdString(meal.getFood().getName());
        mealObj["amount"] = meal.getFood().getAmount();
        mealObj["calories"] = meal.getTotalCalories();
        mealsArray.append(mealObj);
    }
    response["meals"] = mealsArray;
    
    return QHttpServerResponse(QJsonDocument(response), "application/json");
}

QHttpServerResponse RestServer::handlePostMeal(const QHttpServerRequest& request) {
    QJsonDocument doc = QJsonDocument::fromJson(request.body());
    if (doc.isNull() || !doc.isObject()) {
        return QHttpServerResponse("Invalid JSON", QHttpServerResponse::StatusCode::BadRequest);
    }
    
    QJsonObject obj = doc.object();
    QString dateStr = obj["date"].toString();
    if (dateStr.isEmpty()) {
        dateStr = QDate::currentDate().toString("yyyy-MM-dd");
    }
    
    QString mealName = obj["mealName"].toString();
    QString foodName = obj["foodName"].toString();
    double amount = obj["amount"].toDouble();
    
    auto foods = foodDb_->searchFoods(foodName.toStdString());
    if (foods.empty()) {
        return QHttpServerResponse("Food not found", QHttpServerResponse::StatusCode::NotFound);
    }
    
    Food food = foods[0];
    SavedMeal meal(mealName.toStdString(), food, amount);
    diaries_[dateStr].addMeal(meal);
    
    QJsonObject response;
    response["status"] = "success";
    response["message"] = "Meal added";
    
    return QHttpServerResponse(QJsonDocument(response), "application/json");
}

QHttpServerResponse RestServer::handleDeleteMeal(const QHttpServerRequest& request) {
    QUrlQuery query(request.url().query());
    QString dateStr = query.queryItemValue("date");
    int index = query.queryItemValue("index").toInt();
    
    if (dateStr.isEmpty() || !diaries_.contains(dateStr)) {
        return QHttpServerResponse("Diary not found", QHttpServerResponse::StatusCode::NotFound);
    }
    
    Diary& diary = diaries_[dateStr];
    if (index < 0 || index >= static_cast<int>(diary.getMealsCount())) {
        return QHttpServerResponse("Invalid index", QHttpServerResponse::StatusCode::BadRequest);
    }
    
    diary.removeMeal(index);
    
    QJsonObject response;
    response["status"] = "success";
    response["message"] = "Meal deleted";
    
    return QHttpServerResponse(QJsonDocument(response), "application/json");
}
