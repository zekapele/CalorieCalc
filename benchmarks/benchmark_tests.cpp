#include <gtest/gtest.h>
#include <benchmark/benchmark.h>
#include <chrono>
#include "../FoodDatabase.h"
#include "../Diary.h"
#include "../SavedMeal.h"
#include "../Food.h"

// Validation tests for benchmarks
TEST(BenchmarkValidation, FoodDatabaseSearchPerformance) {
    FoodDatabase db;
    auto start = std::chrono::high_resolution_clock::now();
    auto results = db.searchFoods("яблуко");
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    EXPECT_GT(results.size(), 0);
    EXPECT_LT(duration.count(), 10000); // Should be fast (< 10ms)
}

TEST(BenchmarkValidation, DiaryAddMealPerformance) {
    Diary diary;
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    SavedMeal meal("Сніданок", apple, 100.0);
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        diary.addMeal(meal);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    EXPECT_EQ(diary.getMealsCount(), 100);
    EXPECT_LT(duration.count(), 1000); // Should be very fast (< 1ms for 100)
}

TEST(BenchmarkValidation, DiaryTotalCaloriesAccuracy) {
    Diary diary;
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    Food chicken("Куряче філе", 165, 0, 31, 3.6);
    
    diary.addMeal(SavedMeal("Сніданок", apple, 100.0));
    diary.addMeal(SavedMeal("Обід", chicken, 150.0));
    
    double total = diary.getTotalCalories();
    EXPECT_NEAR(total, 52.0 + 247.5, 0.1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
