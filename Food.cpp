#include "Food.h"
#include <iostream>
#include <iomanip>

Food::Food(const std::string& name, double calories, double carbs, double protein, double fat, 
           const std::string& category)
    : name_(name), caloriesPer100g_(calories), carbsPer100g_(carbs),
      proteinPer100g_(protein), fatPer100g_(fat), amount_(100.0), category_(category) {
}

std::string Food::getName() const {
    return name_;
}

double Food::getCalories() const {
    return caloriesPer100g_;
}

double Food::getCarbs() const {
    return carbsPer100g_;
}

double Food::getProtein() const {
    return proteinPer100g_;
}

double Food::getFat() const {
    return fatPer100g_;
}

void Food::setAmount(double amount) {
    amount_ = amount;
}

double Food::getAmount() const {
    return amount_;
}

double Food::getTotalCalories() const {
    return (caloriesPer100g_ * amount_) / 100.0;
}

double Food::getTotalCarbs() const {
    return (carbsPer100g_ * amount_) / 100.0;
}

double Food::getTotalProtein() const {
    return (proteinPer100g_ * amount_) / 100.0;
}

double Food::getTotalFat() const {
    return (fatPer100g_ * amount_) / 100.0;
}

std::string Food::getCategory() const {
    return category_;
}

void Food::print() const {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << name_ << " (" << amount_ << "г):" << std::endl;
    std::cout << "  Калорії: " << getTotalCalories() << " ккал" << std::endl;
    std::cout << "  Вуглеводи: " << getTotalCarbs() << " г" << std::endl;
    std::cout << "  Білки: " << getTotalProtein() << " г" << std::endl;
    std::cout << "  Жири: " << getTotalFat() << " г" << std::endl;
}



