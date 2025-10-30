#include "Diary.h"
#include <iostream>
#include <iomanip>

Diary::Diary() : calorieGoal_(2000.0), waterMl_(0), waterGoalMl_(2000), weightKg_(0.0) {
}

void Diary::addMeal(const SavedMeal& meal) {
    meals_.push_back(meal);
}

void Diary::removeMeal(int index) {
    if (index >= 0 && index < static_cast<int>(meals_.size())) {
        meals_.erase(meals_.begin() + index);
    }
}

std::vector<SavedMeal> Diary::getAllMeals() const {
    return meals_;
}

double Diary::getTotalCalories() const {
    double total = 0.0;
    for (const auto& meal : meals_) {
        total += meal.getTotalCalories();
    }
    return total;
}

double Diary::getTotalCarbs() const {
    double total = 0.0;
    for (const auto& meal : meals_) {
        total += meal.getTotalCarbs();
    }
    return total;
}

double Diary::getTotalProtein() const {
    double total = 0.0;
    for (const auto& meal : meals_) {
        total += meal.getTotalProtein();
    }
    return total;
}

double Diary::getTotalFat() const {
    double total = 0.0;
    for (const auto& meal : meals_) {
        total += meal.getTotalFat();
    }
    return total;
}

void Diary::setCalorieGoal(double goal) {
    calorieGoal_ = goal;
}

double Diary::getCalorieGoal() const {
    return calorieGoal_;
}

double Diary::getRemainingCalories() const {
    return calorieGoal_ - getTotalCalories();
}

void Diary::printDailySummary() const {
    std::cout << "\n╔════════════════════════════════════╗" << std::endl;
    std::cout << "║      ЩОДЕННИЙ ЗВІТ            ║" << std::endl;
    std::cout << "╠════════════════════════════════════╣" << std::endl;
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "║ Ціль: " << std::setw(25) << calorieGoal_ << " ккал ║" << std::endl;
    std::cout << "║ З'їдено: " << std::setw(23) << getTotalCalories() << " ккал ║" << std::endl;
    std::cout << "║ Залишок: " << std::setw(23) << getRemainingCalories() << " ккал ║" << std::endl;
    std::cout << "╠════════════════════════════════════╣" << std::endl;
    std::cout << "║ Макроелементи:                    ║" << std::endl;
    std::cout << "║ Вуглеводи: " << std::setw(21) << getTotalCarbs() << " г ║" << std::endl;
    std::cout << "║ Білки: " << std::setw(26) << getTotalProtein() << " г ║" << std::endl;
    std::cout << "║ Жири: " << std::setw(27) << getTotalFat() << " г ║" << std::endl;
    std::cout << "╠════════════════════════════════════╣" << std::endl;
    std::cout << "║ Вода: " << std::setw(26) << waterMl_ << " / " << waterGoalMl_ << " мл ║" << std::endl;
    if (weightKg_ > 0.0) {
        std::cout << "║ Вага: " << std::setw(26) << weightKg_ << " кг ║" << std::endl;
    }
    std::cout << "╚════════════════════════════════════╝" << std::endl;
}

void Diary::printAllMeals() const {
    if (meals_.empty()) {
        std::cout << "\nЩоденник порожній.\n" << std::endl;
        return;
    }
    
    std::cout << "\n╔════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║            ВСІ ПРИЙОМИ ЇЖІ              ║" << std::endl;
    std::cout << "╠════════════════════════════════════════════════╣" << std::endl;
    
    for (size_t i = 0; i < meals_.size(); ++i) {
        std::cout << "\n[" << (i + 1) << "] ";
        meals_[i].print();
    }
    
    std::cout << "\n╚════════════════════════════════════════════════╝\n" << std::endl;
}

void Diary::clear() {
    meals_.clear();
    waterMl_ = 0;
}

bool Diary::isEmpty() const {
    return meals_.empty();
}

size_t Diary::getMealsCount() const {
    return meals_.size();
}

void Diary::addWater(int ml) {
    if (ml > 0) waterMl_ += ml;
}

void Diary::setWaterGoal(int ml) {
    if (ml >= 0) waterGoalMl_ = ml;
}

int Diary::getWaterMl() const {
    return waterMl_;
}

int Diary::getWaterGoalMl() const {
    return waterGoalMl_;
}

void Diary::setWeightKg(double kg) {
    if (kg >= 0.0) weightKg_ = kg;
}

double Diary::getWeightKg() const {
    return weightKg_;
}

