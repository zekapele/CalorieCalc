#ifndef CALORIECALC_DIARY_H
#define CALORIECALC_DIARY_H

#include "SavedMeal.h"
#include <vector>
#include <string>

class Diary {
public:
    Diary();
    ~Diary() = default;
    
    // Додати прийом їжі
    void addMeal(const SavedMeal& meal);
    
    // Видалити прийом їжі
    void removeMeal(int index);
    
    // Отримати всі прийоми їжі
    std::vector<SavedMeal> getAllMeals() const;
    
    // Розрахунок загальних макроелементів
    double getTotalCalories() const;
    double getTotalCarbs() const;
    double getTotalProtein() const;
    double getTotalFat() const;
    
    // Встановити ціль калорій
    void setCalorieGoal(double goal);
    double getCalorieGoal() const;
    
    // Залишок калорій
    double getRemainingCalories() const;
    
    // Вивести статистику дня
    void printDailySummary() const;
    
    // Вивести всі прийоми їжі
    void printAllMeals() const;
    
    // Очистити щоденник
    void clear();
    
    // Перевірка чи порожній щоденник
    bool isEmpty() const;
    
    // Кількість прийомів їжі
    size_t getMealsCount() const;

    // Вода (мл)
    void addWater(int ml);
    void setWaterGoal(int ml);
    int getWaterMl() const;
    int getWaterGoalMl() const;

    // Вага (кг)
    void setWeightKg(double kg);
    double getWeightKg() const;

private:
    std::vector<SavedMeal> meals_;
    double calorieGoal_;
    int waterMl_;
    int waterGoalMl_;
    double weightKg_;
};

#endif //CALORIECALC_DIARY_H
