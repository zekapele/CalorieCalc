#ifndef CALORIECALC_DIARY_H
#define CALORIECALC_DIARY_H

#include "SavedMeal.h"
#include <vector>
#include <string>

/**
 * @brief Щоденник харчування
 * 
 * Зберігає прийоми їжі за день, обчислює статистику,
 * відстежує воду та вагу.
 */
class Diary {
public:
    /**
     * @brief Конструктор щоденника
     * Ініціалізує ціль калорій (2000), ціль води (2000 мл)
     */
    Diary();
    ~Diary() = default;
    
    /**
     * @brief Додати прийом їжі
     * @param meal Прийом їжі для додавання
     */
    void addMeal(const SavedMeal& meal);
    
    /**
     * @brief Видалити прийом їжі за індексом
     * @param index Індекс прийому їжі (0-based)
     */
    void removeMeal(int index);
    
    /**
     * @brief Отримати всі прийоми їжі
     * @return Вектор прийомів їжі
     */
    std::vector<SavedMeal> getAllMeals() const;
    
    /**
     * @brief Розрахувати загальні калорії за день
     * @return Сума калорій з усіх прийомів їжі
     */
    double getTotalCalories() const;
    
    /**
     * @brief Розрахувати загальні вуглеводи
     * @return Сума вуглеводів (г)
     */
    double getTotalCarbs() const;
    
    /**
     * @brief Розрахувати загальні білки
     * @return Сума білків (г)
     */
    double getTotalProtein() const;
    
    /**
     * @brief Розрахувати загальні жири
     * @return Сума жирів (г)
     */
    double getTotalFat() const;
    
    /**
     * @brief Встановити ціль калорій на день
     * @param goal Ціль калорій
     */
    void setCalorieGoal(double goal);
    
    /**
     * @brief Отримати ціль калорій
     * @return Ціль калорій
     */
    double getCalorieGoal() const;
    
    /**
     * @brief Розрахувати залишок калорій
     * @return Різниця між ціллю та з'їденими калоріями
     */
    double getRemainingCalories() const;
    
    /**
     * @brief Вивести статистику дня у консоль
     */
    void printDailySummary() const;
    
    /**
     * @brief Вивести всі прийоми їжі у консоль
     */
    void printAllMeals() const;
    
    /**
     * @brief Очистити щоденник (видалити всі прийоми, скинути воду)
     */
    void clear();
    
    /**
     * @brief Перевірити чи порожній щоденник
     * @return true якщо немає прийомів їжі
     */
    bool isEmpty() const;
    
    /**
     * @brief Отримати кількість прийомів їжі
     * @return Кількість прийомів
     */
    size_t getMealsCount() const;

    /**
     * @brief Додати випиту воду
     * @param ml Кількість води в мілілітрах
     */
    void addWater(int ml);
    
    /**
     * @brief Встановити ціль води на день
     * @param ml Ціль води в мілілітрах
     */
    void setWaterGoal(int ml);
    
    /**
     * @brief Отримати випиту воду
     * @return Вода в мілілітрах
     */
    int getWaterMl() const;
    
    /**
     * @brief Отримати ціль води
     * @return Ціль води в мілілітрах
     */
    int getWaterGoalMl() const;

    /**
     * @brief Встановити вагу
     * @param kg Вага в кілограмах
     */
    void setWeightKg(double kg);
    
    /**
     * @brief Отримати вагу
     * @return Вага в кілограмах
     */
    double getWeightKg() const;

private:
    std::vector<SavedMeal> meals_;
    double calorieGoal_;
    int waterMl_;
    int waterGoalMl_;
    double weightKg_;
};

#endif //CALORIECALC_DIARY_H
