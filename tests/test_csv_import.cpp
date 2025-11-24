#include <gtest/gtest.h>
#include "../CsvImportExport.h"
#include "../Diary.h"
#include "../SavedMeal.h"
#include "../Food.h"
#include <map>
#include <filesystem>

TEST(CsvImportExportTest, ExportImportRoundTrip) {
    std::map<std::string, std::map<std::string, Diary>> originalData;
    
    Diary diary1;
    diary1.setCalorieGoal(2000.0);
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    diary1.addMeal(SavedMeal("Сніданок", apple, 100.0));
    diary1.addWater(500);
    diary1.setWeightKg(70.0);
    
    originalData["Профіль1"]["2024-01-01"] = diary1;
    
    std::string testFile = "test_export.csv";
    ASSERT_TRUE(CsvImportExport::exportToCSV(originalData, testFile));
    
    std::map<std::string, std::map<std::string, Diary>> importedData;
    ASSERT_TRUE(CsvImportExport::importFromCSV(testFile, importedData));
    
    EXPECT_EQ(importedData.size(), 1);
    EXPECT_EQ(importedData["Профіль1"].size(), 1);
    
    const Diary& imported = importedData["Профіль1"]["2024-01-01"];
    EXPECT_NEAR(imported.getCalorieGoal(), 2000.0, 0.1);
    EXPECT_EQ(imported.getWaterMl(), 500);
    EXPECT_DOUBLE_EQ(imported.getWeightKg(), 70.0);
    EXPECT_EQ(imported.getMealsCount(), 1);
    
    // Cleanup
    std::remove(testFile.c_str());
}

TEST(CsvImportExportTest, TSVExport) {
    std::map<std::string, std::map<std::string, Diary>> data;
    Diary diary;
    Food chicken("Куряче філе", 165, 0, 31, 3.6);
    diary.addMeal(SavedMeal("Обід", chicken, 150.0));
    data["Профіль1"]["2024-01-02"] = diary;
    
    std::string testFile = "test_export.tsv";
    ASSERT_TRUE(CsvImportExport::exportToTSV(data, testFile));
    
    // Verify file exists and is not empty
    std::ifstream file(testFile);
    ASSERT_TRUE(file.good());
    std::string line;
    std::getline(file, line);
    EXPECT_FALSE(line.empty());
    file.close();
    
    std::remove(testFile.c_str());
}

