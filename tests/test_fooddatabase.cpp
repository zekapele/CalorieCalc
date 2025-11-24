#include <gtest/gtest.h>
#include "../FoodDatabase.h"

TEST(FoodDatabaseTest, Constructor) {
    FoodDatabase db;
    EXPECT_GT(db.size(), 0);
}

TEST(FoodDatabaseTest, SearchFoods) {
    FoodDatabase db;
    auto results = db.searchFoods("яблуко");
    EXPECT_GT(results.size(), 0);
    EXPECT_EQ(results[0].getName(), "Яблуко");
}

TEST(FoodDatabaseTest, SearchCaseInsensitive) {
    FoodDatabase db;
    auto results1 = db.searchFoods("ЯБЛУКО");
    auto results2 = db.searchFoods("яблуко");
    EXPECT_EQ(results1.size(), results2.size());
}

TEST(FoodDatabaseTest, AddFood) {
    FoodDatabase db;
    size_t initialSize = db.size();
    Food custom("Кастомний продукт", 100, 20, 5, 2);
    db.addFood(custom);
    EXPECT_EQ(db.size(), initialSize + 1);
    auto results = db.searchFoods("Кастомний");
    EXPECT_EQ(results[0].getName(), "Кастомний продукт");
}

TEST(FoodDatabaseTest, GetFood) {
    FoodDatabase db;
    Food food = db.getFood(0);
    EXPECT_FALSE(food.getName().empty());
    Food invalid = db.getFood(99999);
    EXPECT_TRUE(invalid.getName().empty());
}

