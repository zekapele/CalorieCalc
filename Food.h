#ifndef CALORIECALC_FOOD_H
#define CALORIECALC_FOOD_H

#include <string>

/**
 * @brief Клас для представлення продукту харчування
 * 
 * Зберігає інформацію про калорійність та макроелементи на 100г,
 * дозволяє встановлювати кількість та розраховувати загальні значення.
 */
class Food {
public:
    /**
     * @brief Конструктор продукту
     * @param name Назва продукту
     * @param calories Калорії на 100г
     * @param carbs Вуглеводи на 100г (г)
     * @param protein Білки на 100г (г)
     * @param fat Жири на 100г (г)
     * @param category Категорія продукту (за замовчуванням "Інше")
     */
    Food(const std::string& name, double calories, double carbs, double protein, double fat, 
         const std::string& category = "Інше");
    
    /**
     * @brief Отримати назву продукту
     * @return Назва продукту
     */
    std::string getName() const;
    
    /**
     * @brief Отримати калорії на 100г
     * @return Калорії на 100г
     */
    double getCalories() const;
    
    /**
     * @brief Отримати вуглеводи на 100г
     * @return Вуглеводи (г)
     */
    double getCarbs() const;
    
    /**
     * @brief Отримати білки на 100г
     * @return Білки (г)
     */
    double getProtein() const;
    
    /**
     * @brief Отримати жири на 100г
     * @return Жири (г)
     */
    double getFat() const;
    
    /**
     * @brief Встановити кількість продукту
     * @param amount Кількість в грамах
     */
    void setAmount(double amount);
    
    /**
     * @brief Отримати поточну кількість
     * @return Кількість в грамах
     */
    double getAmount() const;
    
    /**
     * @brief Розрахувати загальні калорії з урахуванням кількості
     * @return Калорії для поточної кількості
     */
    double getTotalCalories() const;
    
    /**
     * @brief Розрахувати загальні вуглеводи
     * @return Вуглеводи (г) для поточної кількості
     */
    double getTotalCarbs() const;
    
    /**
     * @brief Розрахувати загальні білки
     * @return Білки (г) для поточної кількості
     */
    double getTotalProtein() const;
    
    /**
     * @brief Розрахувати загальні жири
     * @return Жири (г) для поточної кількості
     */
    double getTotalFat() const;
    
    /**
     * @brief Отримати категорію продукту
     * @return Категорія продукту
     */
    std::string getCategory() const;
    
    /**
     * @brief Вивести інформацію про продукт у консоль
     */
    void print() const;

private:
    std::string name_;
    double caloriesPer100g_;  // калорії на 100г
    double carbsPer100g_;     // вуглеводи на 100г
    double proteinPer100g_;   // білки на 100г
    double fatPer100g_;       // жири на 100г
    double amount_;           // кількість в грамах
    std::string category_;    // категорія продукту
};

#endif //CALORIECALC_FOOD_H

