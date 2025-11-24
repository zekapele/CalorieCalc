#include <gtest/gtest.h>
#include "../SavedMeal.h"
#include "../Food.h"

TEST(SavedMealTest, Constructor) {
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    SavedMeal meal("Сніданок", apple, 150.0);
    EXPECT_EQ(meal.getMealName(), "Сніданок");
    EXPECT_DOUBLE_EQ(meal.getFood().getAmount(), 150.0);
}

TEST(SavedMealTest, TotalCalories) {
    Food chicken("Куряче філе", 165, 0, 31, 3.6);
    SavedMeal meal("Обід", chicken, 200.0);
    EXPECT_NEAR(meal.getTotalCalories(), 330.0, 0.1);
}

TEST(SavedMealTest, TotalMacros) {
    Food rice("Рис варений", 130, 28, 2.7, 0.3);
    SavedMeal meal("Вечеря", rice, 250.0);
    EXPECT_NEAR(meal.getTotalCarbs(), 70.0, 0.1);
    EXPECT_NEAR(meal.getTotalProtein(), 6.75, 0.1);
}
