#include "JsonSaveStrategy.h"
#include "Diary.h"
#include "SavedMeal.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

// Проста реалізація JSON збереження (без зовнішніх бібліотек для простоти)
bool JsonSaveStrategy::save(const Diary& diary, const std::string& filename) const {
    std::ofstream file(filename + getExtension());
    if (!file.is_open()) {
        return false;
    }
    
    file << std::fixed << std::setprecision(2);
    file << "{\n";
    file << "  \"calorie_goal\": " << diary.getCalorieGoal() << ",\n";
    file << "  \"water_ml\": " << diary.getWaterMl() << ",\n";
    file << "  \"water_goal_ml\": " << diary.getWaterGoalMl() << ",\n";
    file << "  \"weight_kg\": " << diary.getWeightKg() << ",\n";
    file << "  \"meals\": [\n";
    
    auto meals = diary.getAllMeals();
    for (size_t i = 0; i < meals.size(); ++i) {
        const auto& meal = meals[i];
        const auto& food = meal.getFood();
        
        file << "    {\n";
        file << "      \"meal_name\": \"" << meal.getMealName() << "\",\n";
        file << "      \"food_name\": \"" << food.getName() << "\",\n";
        file << "      \"amount\": " << food.getAmount() << ",\n";
        file << "      \"calories_per_100g\": " << food.getCalories() << ",\n";
        file << "      \"carbs_per_100g\": " << food.getCarbs() << ",\n";
        file << "      \"protein_per_100g\": " << food.getProtein() << ",\n";
        file << "      \"fat_per_100g\": " << food.getFat() << "\n";
        file << "    }";
        
        if (i < meals.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    return true;
}

bool JsonSaveStrategy::load(Diary& diary, const std::string& filename) const {
    std::ifstream file(filename + getExtension());
    if (!file.is_open()) {
        return false;
    }
    
    // Простий парсер (для повної реалізації потрібна бібліотека JSON)
    // Формат, який генерується у save():
    // - calorie_goal, water_ml, water_goal_ml, weight_kg
    // - meals: [ { meal_name, food_name, amount, calories_per_100g, carbs_per_100g, protein_per_100g, fat_per_100g }, ... ]
    auto trim = [](std::string s) {
        // trim spaces
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(s.begin());
        while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == ',' || s.back() == '\r' || s.back() == '\n')) s.pop_back();
        return s;
    };

    auto trimQuotes = [&](std::string s) {
        s = trim(std::move(s));
        if (!s.empty() && s.front() == '"') s.erase(s.begin());
        if (!s.empty() && s.back() == '"') s.pop_back();
        return s;
    };

    auto extractAfterColon = [](const std::string& line) -> std::string {
        const auto pos = line.find(':');
        if (pos == std::string::npos) return {};
        return line.substr(pos + 1);
    };

    std::string line;
    double goal = 2000.0;
    int water = 0;
    int waterGoal = 2000;
    double weight = 0.0;

    std::string mealName;
    std::string foodName;
    double amount = 0.0;
    double kcal = 0.0;
    double carbs = 0.0;
    double protein = 0.0;
    double fat = 0.0;

    bool haveMealName = false;
    bool haveFoodName = false;
    bool haveAmount = false;
    bool haveKcal = false;
    bool haveCarbs = false;
    bool haveProtein = false;
    bool haveFat = false;

    auto commitMealIfReady = [&]() {
        if (!(haveMealName && haveFoodName && haveAmount && haveKcal && haveCarbs && haveProtein && haveFat)) return;
        Food food(foodName, kcal, carbs, protein, fat);
        diary.addMeal(SavedMeal(mealName, food, amount));
        mealName.clear();
        foodName.clear();
        amount = 0.0;
        kcal = carbs = protein = fat = 0.0;
        haveMealName = haveFoodName = haveAmount = false;
        haveKcal = haveCarbs = haveProtein = haveFat = false;
    };

    diary.clear();

    while (std::getline(file, line)) {
        if (line.find("calorie_goal") != std::string::npos) {
            goal = std::stod(trim(extractAfterColon(line)));
        } else if (line.find("water_ml") != std::string::npos) {
            water = static_cast<int>(std::stod(trim(extractAfterColon(line))));
        } else if (line.find("water_goal_ml") != std::string::npos) {
            waterGoal = static_cast<int>(std::stod(trim(extractAfterColon(line))));
        } else if (line.find("weight_kg") != std::string::npos) {
            weight = std::stod(trim(extractAfterColon(line)));
        } else if (line.find("\"meal_name\"") != std::string::npos) {
            mealName = trimQuotes(extractAfterColon(line));
            haveMealName = true;
        } else if (line.find("\"food_name\"") != std::string::npos) {
            foodName = trimQuotes(extractAfterColon(line));
            haveFoodName = true;
        } else if (line.find("\"amount\"") != std::string::npos) {
            amount = std::stod(trim(extractAfterColon(line)));
            haveAmount = true;
        } else if (line.find("\"calories_per_100g\"") != std::string::npos) {
            kcal = std::stod(trim(extractAfterColon(line)));
            haveKcal = true;
        } else if (line.find("\"carbs_per_100g\"") != std::string::npos) {
            carbs = std::stod(trim(extractAfterColon(line)));
            haveCarbs = true;
        } else if (line.find("\"protein_per_100g\"") != std::string::npos) {
            protein = std::stod(trim(extractAfterColon(line)));
            haveProtein = true;
        } else if (line.find("\"fat_per_100g\"") != std::string::npos) {
            fat = std::stod(trim(extractAfterColon(line)));
            haveFat = true;
            commitMealIfReady();
        }
    }

    diary.setCalorieGoal(goal);
    diary.setWaterGoal(waterGoal);
    diary.setWeightKg(weight);
    if (water > 0) diary.addWater(water);

    file.close();
    return true;
}

