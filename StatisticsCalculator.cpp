#include "StatisticsCalculator.h"
#include <algorithm>
#include <map>
#include <vector>

StatisticsCalculator::StatisticsCalculator() {
}

void StatisticsCalculator::addDiary(const Diary& diary) {
    currentDiary_ = diary;
}

StatisticsCalculator::DailyStats StatisticsCalculator::calculateAverage() const {
    DailyStats stats;
    stats.daysCount = 1; // Поки один день
    stats.avgCalories = currentDiary_.getTotalCalories();
    stats.avgCarbs = currentDiary_.getTotalCarbs();
    stats.avgProtein = currentDiary_.getTotalProtein();
    stats.avgFat = currentDiary_.getTotalFat();
    return stats;
}

std::map<std::string, double> StatisticsCalculator::getMealTypeStatistics() const {
    std::map<std::string, double> stats;
    auto meals = currentDiary_.getAllMeals();
    
    for (const auto& meal : meals) {
        stats[meal.getMealName()] += meal.getTotalCalories();
    }
    
    return stats;
}

std::map<std::string, int> StatisticsCalculator::getMostUsedFoods(int topN) const {
    std::map<std::string, int> foodCount;
    auto meals = currentDiary_.getAllMeals();
    
    for (const auto& meal : meals) {
        foodCount[meal.getFood().getName()]++;
    }
    
    return foodCount;
}


