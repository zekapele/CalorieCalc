#include "XmlSaveStrategy.h"
#include "Diary.h"
#include "SavedMeal.h"
#include <fstream>
#include <sstream>
#include <iomanip>

bool XmlSaveStrategy::save(const Diary& diary, const std::string& filename) const {
    std::ofstream file(filename + getExtension());
    if (!file.is_open()) {
        return false;
    }
    
    file << std::fixed << std::setprecision(2);
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    file << "<diary>\n";
    file << "  <calorie_goal>" << diary.getCalorieGoal() << "</calorie_goal>\n";
    file << "  <water_ml>" << diary.getWaterMl() << "</water_ml>\n";
    file << "  <water_goal_ml>" << diary.getWaterGoalMl() << "</water_goal_ml>\n";
    file << "  <weight_kg>" << diary.getWeightKg() << "</weight_kg>\n";
    file << "  <meals>\n";
    
    auto meals = diary.getAllMeals();
    for (const auto& meal : meals) {
        const auto& food = meal.getFood();
        file << "    <meal>\n";
        file << "      <meal_name>" << meal.getMealName() << "</meal_name>\n";
        file << "      <food_name>" << food.getName() << "</food_name>\n";
        file << "      <amount>" << food.getAmount() << "</amount>\n";
        file << "      <calories_per_100g>" << food.getCalories() << "</calories_per_100g>\n";
        file << "      <carbs_per_100g>" << food.getCarbs() << "</carbs_per_100g>\n";
        file << "      <protein_per_100g>" << food.getProtein() << "</protein_per_100g>\n";
        file << "      <fat_per_100g>" << food.getFat() << "</fat_per_100g>\n";
        file << "    </meal>\n";
    }
    
    file << "  </meals>\n";
    file << "</diary>\n";
    
    file.close();
    return true;
}

bool XmlSaveStrategy::load(Diary& diary, const std::string& filename) const {
    std::ifstream file(filename + getExtension());
    if (!file.is_open()) {
        return false;
    }
    
    diary.clear();
    
    std::string line;
    double goal = 2000.0;
    int water = 0, waterGoal = 2000;
    double weight = 0.0;
    bool inMeal = false;
    std::string mealName, foodName;
    double amount = 0, calories = 0, carbs = 0, protein = 0, fat = 0;
    
    while (std::getline(file, line)) {
        if (line.find("<calorie_goal>") != std::string::npos) {
            size_t start = line.find(">") + 1;
            size_t end = line.find("<", start);
            goal = std::stod(line.substr(start, end - start));
        } else if (line.find("<water_ml>") != std::string::npos) {
            size_t start = line.find(">") + 1;
            size_t end = line.find("<", start);
            water = static_cast<int>(std::stod(line.substr(start, end - start)));
        } else if (line.find("<water_goal_ml>") != std::string::npos) {
            size_t start = line.find(">") + 1;
            size_t end = line.find("<", start);
            waterGoal = static_cast<int>(std::stod(line.substr(start, end - start)));
        } else if (line.find("<weight_kg>") != std::string::npos) {
            size_t start = line.find(">") + 1;
            size_t end = line.find("<", start);
            weight = std::stod(line.substr(start, end - start));
        } else if (line.find("<meal>") != std::string::npos) {
            inMeal = true;
        } else if (line.find("</meal>") != std::string::npos) {
            Food food(foodName, calories, carbs, protein, fat);
            SavedMeal meal(mealName, food, amount);
            diary.addMeal(meal);
            inMeal = false;
        } else if (inMeal) {
            if (line.find("<meal_name>") != std::string::npos) {
                size_t start = line.find(">") + 1;
                size_t end = line.find("<", start);
                mealName = line.substr(start, end - start);
            } else if (line.find("<food_name>") != std::string::npos) {
                size_t start = line.find(">") + 1;
                size_t end = line.find("<", start);
                foodName = line.substr(start, end - start);
            } else if (line.find("<amount>") != std::string::npos) {
                size_t start = line.find(">") + 1;
                size_t end = line.find("<", start);
                amount = std::stod(line.substr(start, end - start));
            } else if (line.find("<calories_per_100g>") != std::string::npos) {
                size_t start = line.find(">") + 1;
                size_t end = line.find("<", start);
                calories = std::stod(line.substr(start, end - start));
            } else if (line.find("<carbs_per_100g>") != std::string::npos) {
                size_t start = line.find(">") + 1;
                size_t end = line.find("<", start);
                carbs = std::stod(line.substr(start, end - start));
            } else if (line.find("<protein_per_100g>") != std::string::npos) {
                size_t start = line.find(">") + 1;
                size_t end = line.find("<", start);
                protein = std::stod(line.substr(start, end - start));
            } else if (line.find("<fat_per_100g>") != std::string::npos) {
                size_t start = line.find(">") + 1;
                size_t end = line.find("<", start);
                fat = std::stod(line.substr(start, end - start));
            }
        }
    }
    
    diary.setCalorieGoal(goal);
    diary.addWater(water);
    diary.setWaterGoal(waterGoal);
    diary.setWeightKg(weight);
    
    file.close();
    return true;
}

