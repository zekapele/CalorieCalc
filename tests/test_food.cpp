#include <gtest/gtest.h>
#include "../Food.h"

TEST(FoodTest, Constructor) {
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    EXPECT_EQ(apple.getName(), "Яблуко");
    EXPECT_DOUBLE_EQ(apple.getCalories(), 52.0);
    EXPECT_DOUBLE_EQ(apple.getCarbs(), 14.0);
    EXPECT_DOUBLE_EQ(apple.getProtein(), 0.3);
    EXPECT_DOUBLE_EQ(apple.getFat(), 0.2);
    EXPECT_DOUBLE_EQ(apple.getAmount(), 100.0);
}

TEST(FoodTest, SetAmount) {
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    apple.setAmount(150.0);
    EXPECT_DOUBLE_EQ(apple.getAmount(), 150.0);
}

TEST(FoodTest, TotalCalories) {
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    apple.setAmount(200.0);
    EXPECT_DOUBLE_EQ(apple.getTotalCalories(), 104.0);
}

TEST(FoodTest, TotalMacros) {
    Food chicken("Куряче філе", 165, 0, 31, 3.6);
    chicken.setAmount(150.0);
    EXPECT_NEAR(chicken.getTotalCalories(), 247.5, 0.1);
    EXPECT_NEAR(chicken.getTotalProtein(), 46.5, 0.1);
    EXPECT_NEAR(chicken.getTotalFat(), 5.4, 0.1);
}

