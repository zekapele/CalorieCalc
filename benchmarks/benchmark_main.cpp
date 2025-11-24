#include <benchmark/benchmark.h>
#include "../FoodDatabase.h"
#include "../Diary.h"
#include "../SavedMeal.h"
#include "../Food.h"

static void BM_FoodDatabaseSearch(benchmark::State& state) {
    FoodDatabase db;
    for (auto _ : state) {
        auto results = db.searchFoods("яблуко");
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK(BM_FoodDatabaseSearch);

static void BM_DiaryAddMeal(benchmark::State& state) {
    Diary diary;
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    SavedMeal meal("Сніданок", apple, 100.0);
    
    for (auto _ : state) {
        diary.addMeal(meal);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_DiaryAddMeal);

static void BM_DiaryTotalCalories(benchmark::State& state) {
    Diary diary;
    Food apple("Яблуко", 52, 14, 0.3, 0.2);
    Food chicken("Куряче філе", 165, 0, 31, 3.6);
    
    for (int i = 0; i < 10; ++i) {
        diary.addMeal(SavedMeal("Сніданок", apple, 100.0));
        diary.addMeal(SavedMeal("Обід", chicken, 150.0));
    }
    
    for (auto _ : state) {
        double total = diary.getTotalCalories();
        benchmark::DoNotOptimize(total);
    }
}
BENCHMARK(BM_DiaryTotalCalories);

BENCHMARK_MAIN();

