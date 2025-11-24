#ifndef CALORIECALC_FOODDATABASE_H
#define CALORIECALC_FOODDATABASE_H

#include <vector>
#include <string>
#include "Food.h"

/**
 * @brief База даних продуктів
 * 
 * Зберігає колекцію продуктів, надає пошук за назвою
 * та можливість додавання нових продуктів.
 */
class FoodDatabase {
public:
    /**
     * @brief Конструктор бази даних
     * Автоматично ініціалізує базові продукти
     */
    FoodDatabase();
    
    /**
     * @brief Пошук продуктів за назвою (нечутливий до регістру)
     * @param query Пошуковий запит
     * @return Вектор знайдених продуктів
     */
    std::vector<Food> searchFoods(const std::string& query) const;
    
    /**
     * @brief Пошук продуктів за категорією
     * @param category Категорія продукту
     * @return Вектор продуктів з цієї категорії
     */
    std::vector<Food> searchByCategory(const std::string& category) const;
    
    /**
     * @brief Отримати всі доступні категорії
     * @return Вектор назв категорій
     */
    std::vector<std::string> getAllCategories() const;
    
    /**
     * @brief Отримати всі продукти з бази
     * @return Вектор усіх продуктів
     */
    std::vector<Food> getAllFoods() const;
    
    /**
     * @brief Додати новий продукт до бази
     * @param food Продукт для додавання
     */
    void addFood(const Food& food);
    
    /**
     * @brief Отримати продукт за індексом
     * @param index Індекс продукту (0-based)
     * @return Продукт або порожній Food якщо індекс невалідний
     */
    Food getFood(int index) const;
    
    /**
     * @brief Отримати кількість продуктів у базі
     * @return Кількість продуктів
     */
    size_t size() const;
    
    /**
     * @brief Ініціалізувати базові продукти
     * Додає початковий набір продуктів у базу
     */
    void initializeDefaultFoods();

private:
    std::vector<Food> foods_;
};

#endif //CALORIECALC_FOODDATABASE_H



