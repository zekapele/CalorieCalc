#ifndef CALORIECALC_STATISTICSCALCULATOR_H
#define CALORIECALC_STATISTICSCALCULATOR_H

#include "Diary.h"
#include <vector>
#include <map>
#include <string>

// Клас для розрахунку статистики
// Демонстрація композиції та aggregation
class StatisticsCalculator {
public:
    StatisticsCalculator();
    
    // Розрахунок середніх значень
    struct DailyStats {
        double avgCalories;
        double avgCarbs;
        double avgProtein;
        double avgFat;
        int daysCount;
    };
    
    // Додати щоденник для статистики (поки один, але можна розширити)
    void addDiary(const Diary& diary);
    
    // Отримати статистику
    DailyStats calculateAverage() const;
    
    // Статистика по прийомах їжі
    std::map<std::string, double> getMealTypeStatistics() const;
    
    // Найпопулярніші продукти
    std::map<std::string, int> getMostUsedFoods(int topN = 5) const;

private:
    Diary currentDiary_;
};

#endif //CALORIECALC_STATISTICSCALCULATOR_H

