#include "TextSaveStrategy.h"
#include "Diary.h"
#include "SavedMeal.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

bool TextSaveStrategy::save(const Diary& diary, const std::string& filename) const {
    std::ofstream file(filename + getExtension());
    if (!file.is_open()) {
        return false;
    }
    
    file << std::fixed << std::setprecision(2);
    file << "CALORIE_GOAL: " << diary.getCalorieGoal() << "\n";
    file << "MEALS_COUNT: " << diary.getAllMeals().size() << "\n";
    file << "---\n";
    
    auto meals = diary.getAllMeals();
    for (const auto& meal : meals) {
        const auto& food = meal.getFood();
        file << meal.getMealName() << "\n";
        file << food.getName() << "\n";
        file << food.getAmount() << "\n";
        file << food.getCalories() << "\n";
        file << food.getCarbs() << "\n";
        file << food.getProtein() << "\n";
        file << food.getFat() << "\n";
        file << "---\n";
    }
    
    file.close();
    return true;
}

bool TextSaveStrategy::load(Diary& diary, const std::string& filename) const {
    std::ifstream file(filename + getExtension());
    if (!file.is_open()) {
        return false;
    }
    
    diary.clear();
    
    std::string line;
    double goal = 2000.0;
    
    // Читаємо ціль
    if (std::getline(file, line)) {
        if (line.find("CALORIE_GOAL:") != std::string::npos) {
            goal = std::stod(line.substr(line.find(":") + 1));
        }
    }
    
    // Пропускаємо кількість та розділювач
    std::getline(file, line);
    std::getline(file, line);
    
    // Читаємо прийоми їжі
    while (std::getline(file, line)) {
        if (line == "---") {
            continue;
        }
        
        std::string mealName = line;
        std::getline(file, line);
        std::string foodName = line;
        std::getline(file, line);
        double amount = std::stod(line);
        std::getline(file, line);
        double calories = std::stod(line);
        std::getline(file, line);
        double carbs = std::stod(line);
        std::getline(file, line);
        double protein = std::stod(line);
        std::getline(file, line);
        double fat = std::stod(line);
        
        Food food(foodName, calories, carbs, protein, fat);
        SavedMeal meal(mealName, food, amount);
        diary.addMeal(meal);
        
        std::getline(file, line); // Пропускаємо розділювач
    }
    
    diary.setCalorieGoal(goal);
    file.close();
    return true;
}

