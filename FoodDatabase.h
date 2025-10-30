#ifndef CALORIECALC_FOODDATABASE_H
#define CALORIECALC_FOODDATABASE_H

#include <vector>
#include <string>
#include "Food.h"

class FoodDatabase {
public:
    FoodDatabase();
    
    // Пошук продуктів за назвою
    std::vector<Food> searchFoods(const std::string& query) const;
    
    // Отримати всі продукти
    std::vector<Food> getAllFoods() const;
    
    // Додати новий продукт до бази
    void addFood(const Food& food);
    
    // Отримати продукт за індексом
    Food getFood(int index) const;
    
    // Кількість продуктів у базі
    size_t size() const;
    
    // Ініціалізація базових продуктів
    void initializeDefaultFoods();

private:
    std::vector<Food> foods_;
};

#endif //CALORIECALC_FOODDATABASE_H



