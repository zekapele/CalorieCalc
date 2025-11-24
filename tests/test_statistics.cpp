#include <gtest/gtest.h>
#include "../StatisticsCalculator.h"
#include "../Diary.h"
#include "../SavedMeal.h"
#include "../Food.h"

TEST(StatisticsCalculatorTest, CalculateAverage) {
    Diary diary;
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    Food chicken("Куряче філе", 165, 0, 31, 3.6);
    
    diary.addMeal(SavedMeal("Сніданок", apple, 100.0));
    diary.addMeal(SavedMeal("Обід", chicken, 150.0));
    
    StatisticsCalculator calc;
    calc.addDiary(diary);
    auto stats = calc.calculateAverage();
    
    EXPECT_NEAR(stats.avgCalories, 52.0 + 247.5, 0.1);
    EXPECT_EQ(stats.daysCount, 1);
}

TEST(StatisticsCalculatorTest, MealTypeStatistics) {
    Diary diary;
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    Food chicken("Куряче філе", 165, 0, 31, 3.6);
    
    diary.addMeal(SavedMeal("Сніданок", apple, 100.0));
    diary.addMeal(SavedMeal("Обід", chicken, 150.0));
    
    StatisticsCalculator calc;
    calc.addDiary(diary);
    auto stats = calc.getMealTypeStatistics();
    
    EXPECT_GT(stats["Сніданок"], 0.0);
    EXPECT_GT(stats["Обід"], 0.0);
}

TEST(StatisticsCalculatorTest, MostUsedFoods) {
    Diary diary;
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    
    diary.addMeal(SavedMeal("Сніданок", apple, 100.0));
    diary.addMeal(SavedMeal("Перекус", apple, 50.0));
    
    StatisticsCalculator calc;
    calc.addDiary(diary);
    auto foods = calc.getMostUsedFoods(5);
    
    EXPECT_EQ(foods["Яблуко"], 2);
}
