#include "CsvImportExport.h"
#include "SavedMeal.h"
#include <fstream>
#include <sstream>
#include <algorithm>

bool CsvImportExport::exportToCSV(const std::map<std::string, std::map<std::string, Diary>>& diaries,
                                  const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // Header
    file << "Дата,Профіль,Секція,Продукт,Кількість (г),Калорії,Білки (г),Жири (г),Вуглеводи (г),Вода (мл),Вага (кг)\n";

    for (const auto& profilePair : diaries) {
        const std::string& profile = profilePair.first;
        for (const auto& datePair : profilePair.second) {
            const std::string& date = datePair.first;
            const Diary& diary = datePair.second;
            
            auto meals = diary.getAllMeals();
            for (const auto& meal : meals) {
                const auto& food = meal.getFood();
                file << date << ","
                     << profile << ","
                     << meal.getMealName() << ","
                     << food.getName() << ","
                     << food.getAmount() << ","
                     << meal.getTotalCalories() << ","
                     << meal.getTotalProtein() << ","
                     << meal.getTotalFat() << ","
                     << meal.getTotalCarbs() << ","
                     << diary.getWaterMl() << ","
                     << diary.getWeightKg() << "\n";
            }
        }
    }

    file.close();
    return true;
}

bool CsvImportExport::importFromCSV(const std::string& filename,
                                   std::map<std::string, std::map<std::string, Diary>>& diaries) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    bool firstLine = true;
    
    while (std::getline(file, line)) {
        if (firstLine) {
            firstLine = false;
            continue; // Skip header
        }

        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(iss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() < 11) continue;

        std::string date = tokens[0];
        std::string profile = tokens[1];
        std::string mealName = tokens[2];
        std::string foodName = tokens[3];
        double amount = std::stod(tokens[4]);
        double calories = std::stod(tokens[5]);
        double protein = std::stod(tokens[6]);
        double fat = std::stod(tokens[7]);
        double carbs = std::stod(tokens[8]);
        int water = std::stoi(tokens[9]);
        double weight = std::stod(tokens[10]);

        Food food(foodName, calories, carbs, protein, fat);
        SavedMeal meal(mealName, food, amount);
        
        Diary& diary = diaries[profile][date];
        diary.addMeal(meal);
        diary.addWater(water);
        diary.setWeightKg(weight);
    }

    file.close();
    return true;
}

bool CsvImportExport::exportToTSV(const std::map<std::string, std::map<std::string, Diary>>& diaries,
                                   const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // TSV header (tab-separated)
    file << "Дата\tПрофіль\tСекція\tПродукт\tКількість (г)\tКалорії\tБілки (г)\tЖири (г)\tВуглеводи (г)\tВода (мл)\tВага (кг)\n";

    for (const auto& profilePair : diaries) {
        const std::string& profile = profilePair.first;
        for (const auto& datePair : profilePair.second) {
            const std::string& date = datePair.first;
            const Diary& diary = datePair.second;
            
            auto meals = diary.getAllMeals();
            for (const auto& meal : meals) {
                const auto& food = meal.getFood();
                file << date << "\t"
                     << profile << "\t"
                     << meal.getMealName() << "\t"
                     << food.getName() << "\t"
                     << food.getAmount() << "\t"
                     << meal.getTotalCalories() << "\t"
                     << meal.getTotalProtein() << "\t"
                     << meal.getTotalFat() << "\t"
                     << meal.getTotalCarbs() << "\t"
                     << diary.getWaterMl() << "\t"
                     << diary.getWeightKg() << "\n";
            }
        }
    }

    file.close();
    return true;
}

