#include "SavedMeal.h"
#include <iostream>
#include <iomanip>

SavedMeal::SavedMeal(const std::string& mealName, const Food& food, double amount)
    : mealName_(mealName), food_(food), amount_(amount) {
    food_.setAmount(amount);
}

std::string SavedMeal::getMealName() const {
    return mealName_;
}

Food SavedMeal::getFood() const {
    return food_;
}

double SavedMeal::getTotalCalories() const {
    return food_.getTotalCalories();
}

double SavedMeal::getTotalCarbs() const {
    return food_.getTotalCarbs();
}

double SavedMeal::getTotalProtein() const {
    return food_.getTotalProtein();
}

double SavedMeal::getTotalFat() const {
    return food_.getTotalFat();
}

void SavedMeal::print() const {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "[" << mealName_ << "] ";
    food_.print();
}



