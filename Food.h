#ifndef CALORIECALC_FOOD_H
#define CALORIECALC_FOOD_H

#include <string>

class Food {
public:
    Food(const std::string& name, double calories, double carbs, double protein, double fat);
    
    // Гетери
    std::string getName() const;
    double getCalories() const;
    double getCarbs() const;
    double getProtein() const;
    double getFat() const;
    
    // Встановлення кількості (грами або порції)
    void setAmount(double amount);
    double getAmount() const;
    
    // Розрахунок макроелементів з урахуванням кількості
    double getTotalCalories() const;
    double getTotalCarbs() const;
    double getTotalProtein() const;
    double getTotalFat() const;
    
    // Виведення інформації
    void print() const;

private:
    std::string name_;
    double caloriesPer100g_;  // калорії на 100г
    double carbsPer100g_;     // вуглеводи на 100г
    double proteinPer100g_;   // білки на 100г
    double fatPer100g_;       // жири на 100г
    double amount_;           // кількість в грамах
};

#endif //CALORIECALC_FOOD_H

