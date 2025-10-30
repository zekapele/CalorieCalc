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
    std::string line;
    double goal = 2000.0;
    int water = 0;
    int waterGoal = 2000;
    double weight = 0.0;
    
    while (std::getline(file, line)) {
        if (line.find("calorie_goal") != std::string::npos) {
            size_t pos = line.find(":"); if (pos != std::string::npos) goal = std::stod(line.substr(pos + 1));
        } else if (line.find("water_ml") != std::string::npos) {
            size_t pos = line.find(":"); if (pos != std::string::npos) water = static_cast<int>(std::stod(line.substr(pos + 1)));
        } else if (line.find("water_goal_ml") != std::string::npos) {
            size_t pos = line.find(":"); if (pos != std::string::npos) waterGoal = static_cast<int>(std::stod(line.substr(pos + 1)));
        } else if (line.find("weight_kg") != std::string::npos) {
            size_t pos = line.find(":"); if (pos != std::string::npos) weight = std::stod(line.substr(pos + 1));
        }
    }
    
    diary.clear();
    diary.setCalorieGoal(goal);
    diary.addWater(water);
    diary.setWaterGoal(waterGoal);
    diary.setWeightKg(weight);
    
    file.close();
    return true;
}

