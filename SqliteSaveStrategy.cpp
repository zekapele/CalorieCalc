#ifdef HAVE_SQLITE3
#include "SqliteSaveStrategy.h"
#include "Diary.h"
#include "SavedMeal.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>

SqliteSaveStrategy::SqliteSaveStrategy() {
}

SqliteSaveStrategy::~SqliteSaveStrategy() {
}

bool SqliteSaveStrategy::createTables(const std::string& dbPath) const {
    sqlite3* db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        return false;
    }

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS diary (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            calorie_goal REAL,
            water_ml INTEGER,
            water_goal_ml INTEGER,
            weight_kg REAL
        );
        CREATE TABLE IF NOT EXISTS meals (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            diary_id INTEGER,
            meal_name TEXT,
            food_name TEXT,
            amount REAL,
            calories_per_100g REAL,
            carbs_per_100g REAL,
            protein_per_100g REAL,
            fat_per_100g REAL,
            FOREIGN KEY (diary_id) REFERENCES diary(id)
        );
        CREATE INDEX IF NOT EXISTS idx_diary_date ON diary(date);
    )";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    return true;
}

bool SqliteSaveStrategy::save(const Diary& diary, const std::string& filename) const {
    std::string dbPath = filename + getExtension();
    
    if (!createTables(dbPath)) {
        return false;
    }

    sqlite3* db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        return false;
    }

    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

    // Delete existing data for this date
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "DELETE FROM meals WHERE diary_id IN (SELECT id FROM diary WHERE date = ?)", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, filename.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    sqlite3_prepare_v2(db, "DELETE FROM diary WHERE date = ?", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, filename.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Insert diary
    sqlite3_prepare_v2(db, 
        "INSERT INTO diary (date, calorie_goal, water_ml, water_goal_ml, weight_kg) VALUES (?, ?, ?, ?, ?)",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, filename.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, diary.getCalorieGoal());
    sqlite3_bind_int(stmt, 3, diary.getWaterMl());
    sqlite3_bind_int(stmt, 4, diary.getWaterGoalMl());
    sqlite3_bind_double(stmt, 5, diary.getWeightKg());
    sqlite3_step(stmt);
    sqlite3_int64 diaryId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);

    // Insert meals
    auto meals = diary.getAllMeals();
    for (const auto& meal : meals) {
        const auto& food = meal.getFood();
        sqlite3_prepare_v2(db,
            "INSERT INTO meals (diary_id, meal_name, food_name, amount, calories_per_100g, carbs_per_100g, protein_per_100g, fat_per_100g) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
            -1, &stmt, nullptr);
        sqlite3_bind_int64(stmt, 1, diaryId);
        sqlite3_bind_text(stmt, 2, meal.getMealName().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, food.getName().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, food.getAmount());
        sqlite3_bind_double(stmt, 5, food.getCalories());
        sqlite3_bind_double(stmt, 6, food.getCarbs());
        sqlite3_bind_double(stmt, 7, food.getProtein());
        sqlite3_bind_double(stmt, 8, food.getFat());
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
    sqlite3_close(db);
    return true;
}

bool SqliteSaveStrategy::load(Diary& diary, const std::string& filename) const {
    std::string dbPath = filename + getExtension();
    
    sqlite3* db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        return false;
    }

    // Load diary
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "SELECT calorie_goal, water_ml, water_goal_ml, weight_kg, id FROM diary WHERE date = ?", -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, filename.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    double goal = sqlite3_column_double(stmt, 0);
    int water = sqlite3_column_int(stmt, 1);
    int waterGoal = sqlite3_column_int(stmt, 2);
    double weight = sqlite3_column_double(stmt, 3);
    sqlite3_int64 diaryId = sqlite3_column_int64(stmt, 4);
    
    diary.clear();
    diary.setCalorieGoal(goal);
    diary.addWater(water);
    diary.setWaterGoal(waterGoal);
    diary.setWeightKg(weight);
    
    sqlite3_finalize(stmt);

    // Load meals
    sqlite3_prepare_v2(db,
        "SELECT meal_name, food_name, amount, calories_per_100g, carbs_per_100g, protein_per_100g, fat_per_100g FROM meals WHERE diary_id = ?",
        -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, diaryId);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string mealName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string foodName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        double amount = sqlite3_column_double(stmt, 2);
        double calories = sqlite3_column_double(stmt, 3);
        double carbs = sqlite3_column_double(stmt, 4);
        double protein = sqlite3_column_double(stmt, 5);
        double fat = sqlite3_column_double(stmt, 6);
        
        Food food(foodName, calories, carbs, protein, fat);
        SavedMeal meal(mealName, food, amount);
        diary.addMeal(meal);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

#endif // HAVE_SQLITE3
