#ifndef CALORIECALC_SAVEDMEAL_H
#define CALORIECALC_SAVEDMEAL_H

#include "Food.h"
#include <vector>

// Клас для збереження прийому їжі з часом
class SavedMeal {
public:
    SavedMeal(const std::string& mealName, const Food& food, double amount);
    
    // Гетери
    std::string getMealName() const;
    Food getFood() const;
    double getTotalCalories() const;
    double getTotalCarbs() const;
    double getTotalProtein() const;
    double getTotalFat() const;
    
    // Виведення
    void print() const;

private:
    std::string mealName_;  // Назва прийому їжі (сніданок, обід, вечеря, перекус)
    Food food_;
    double amount_;
};

#endif //CALORIECALC_SAVEDMEAL_H



