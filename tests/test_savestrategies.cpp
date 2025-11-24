#include <gtest/gtest.h>
#include "../JsonSaveStrategy.h"
#include "../TextSaveStrategy.h"
#include "../Diary.h"
#include "../SavedMeal.h"
#include "../Food.h"
#include <fstream>
#include <filesystem>

TEST(SaveStrategyTest, JsonSaveLoad) {
    Diary diary;
    diary.setCalorieGoal(2500.0);
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    diary.addMeal(SavedMeal("Сніданок", apple, 100.0));
    diary.addWater(500);
    diary.setWaterGoal(3000);
    diary.setWeightKg(75.5);

    JsonSaveStrategy saver;
    std::string testFile = "test_diary";
    
    ASSERT_TRUE(saver.save(diary, testFile));
    
    Diary loaded;
    ASSERT_TRUE(saver.load(loaded, testFile));
    
    EXPECT_NEAR(loaded.getCalorieGoal(), 2500.0, 0.1);
    EXPECT_EQ(loaded.getWaterMl(), 500);
    EXPECT_EQ(loaded.getWaterGoalMl(), 3000);
    EXPECT_DOUBLE_EQ(loaded.getWeightKg(), 75.5);
    EXPECT_EQ(loaded.getMealsCount(), 1);
    
    // Cleanup
    std::remove((testFile + ".json").c_str());
}

TEST(SaveStrategyTest, TextSaveLoad) {
    Diary diary;
    diary.setCalorieGoal(2000.0);
    Food chicken("Куряче філе", 165, 0, 31, 3.6);
    diary.addMeal(SavedMeal("Обід", chicken, 150.0));

    TextSaveStrategy saver;
    std::string testFile = "test_diary_txt";
    
    ASSERT_TRUE(saver.save(diary, testFile));
    
    Diary loaded;
    ASSERT_TRUE(saver.load(loaded, testFile));
    
    EXPECT_NEAR(loaded.getCalorieGoal(), 2000.0, 0.1);
    EXPECT_EQ(loaded.getMealsCount(), 1);
    
    // Cleanup
    std::remove((testFile + ".txt").c_str());
}

#ifdef HAVE_SQLITE3
#include "../SqliteSaveStrategy.h"

TEST(SaveStrategyTest, SqliteSaveLoad) {
    Diary diary;
    diary.setCalorieGoal(2200.0);
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    diary.addMeal(SavedMeal("Сніданок", apple, 100.0));
    diary.addWater(750);
    diary.setWeightKg(70.0);

    SqliteSaveStrategy saver;
    std::string testFile = "test_diary_sqlite";
    
    ASSERT_TRUE(saver.save(diary, testFile));
    
    Diary loaded;
    ASSERT_TRUE(saver.load(loaded, testFile));
    
    EXPECT_NEAR(loaded.getCalorieGoal(), 2200.0, 0.1);
    EXPECT_EQ(loaded.getWaterMl(), 750);
    EXPECT_DOUBLE_EQ(loaded.getWeightKg(), 70.0);
    EXPECT_EQ(loaded.getMealsCount(), 1);
    
    // Cleanup
    std::remove((testFile + ".db").c_str());
}
#endif
