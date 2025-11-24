#include <gtest/gtest.h>
#include "../Diary.h"
#include "../SavedMeal.h"
#include "../Food.h"

TEST(DiaryTest, Constructor) {
    Diary diary;
    EXPECT_TRUE(diary.isEmpty());
    EXPECT_EQ(diary.getMealsCount(), 0);
    EXPECT_DOUBLE_EQ(diary.getCalorieGoal(), 2000.0);
    EXPECT_EQ(diary.getWaterMl(), 0);
    EXPECT_EQ(diary.getWaterGoalMl(), 2000);
}

TEST(DiaryTest, AddMeal) {
    Diary diary;
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    SavedMeal meal("Сніданок", apple, 100.0);
    diary.addMeal(meal);
    EXPECT_FALSE(diary.isEmpty());
    EXPECT_EQ(diary.getMealsCount(), 1);
}

TEST(DiaryTest, TotalCalories) {
    Diary diary;
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    Food chicken("Куряче філе", 165, 0, 31, 3.6);
    diary.addMeal(SavedMeal("Сніданок", apple, 100.0));
    diary.addMeal(SavedMeal("Обід", chicken, 150.0));
    EXPECT_NEAR(diary.getTotalCalories(), 52.0 + 247.5, 0.1);
}

TEST(DiaryTest, RemoveMeal) {
    Diary diary;
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    diary.addMeal(SavedMeal("Сніданок", apple, 100.0));
    diary.removeMeal(0);
    EXPECT_TRUE(diary.isEmpty());
}

TEST(DiaryTest, WaterTracking) {
    Diary diary;
    diary.addWater(250);
    diary.addWater(500);
    EXPECT_EQ(diary.getWaterMl(), 750);
    diary.setWaterGoal(3000);
    EXPECT_EQ(diary.getWaterGoalMl(), 3000);
}

TEST(DiaryTest, WeightTracking) {
    Diary diary;
    diary.setWeightKg(75.5);
    EXPECT_DOUBLE_EQ(diary.getWeightKg(), 75.5);
}

TEST(DiaryTest, RemainingCalories) {
    Diary diary;
    diary.setCalorieGoal(2000.0);
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    diary.addMeal(SavedMeal("Сніданок", apple, 100.0));
    EXPECT_NEAR(diary.getRemainingCalories(), 2000.0 - 52.0, 0.1);
}

