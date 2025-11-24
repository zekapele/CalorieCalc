#include "FoodDatabase.h"
#include <algorithm>
#include <cctype>

FoodDatabase::FoodDatabase() {
    initializeDefaultFoods();
}

std::vector<Food> FoodDatabase::searchFoods(const std::string& query) const {
    std::vector<Food> results;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (const auto& food : foods_) {
        std::string foodName = food.getName();
        std::transform(foodName.begin(), foodName.end(), foodName.begin(), ::tolower);
        
        if (foodName.find(lowerQuery) != std::string::npos) {
            results.push_back(food);
        }
    }
    
    return results;
}

std::vector<Food> FoodDatabase::getAllFoods() const {
    return foods_;
}

void FoodDatabase::addFood(const Food& food) {
    foods_.push_back(food);
}

Food FoodDatabase::getFood(int index) const {
    if (index >= 0 && index < static_cast<int>(foods_.size())) {
        return foods_[index];
    }
    return Food("", 0, 0, 0, 0);
}

size_t FoodDatabase::size() const {
    return foods_.size();
}

std::vector<Food> FoodDatabase::searchByCategory(const std::string& category) const {
    std::vector<Food> results;
    for (const auto& food : foods_) {
        if (food.getCategory() == category) {
            results.push_back(food);
        }
    }
    return results;
}

std::vector<std::string> FoodDatabase::getAllCategories() const {
    std::vector<std::string> categories;
    for (const auto& food : foods_) {
        const std::string& cat = food.getCategory();
        if (std::find(categories.begin(), categories.end(), cat) == categories.end()) {
            categories.push_back(cat);
        }
    }
    return categories;
}

void FoodDatabase::initializeDefaultFoods() {
    // Овочі та фрукти
    foods_.push_back(Food("Яблуко", 52, 14, 0.3, 0.2, "Фрукти"));
    foods_.push_back(Food("Банан", 89, 23, 1.1, 0.3, "Фрукти"));
    foods_.push_back(Food("Броколі", 34, 7, 2.8, 0.4, "Овочі"));
    foods_.push_back(Food("Морква", 41, 10, 0.9, 0.2, "Овочі"));
    foods_.push_back(Food("Помідор", 18, 4, 0.9, 0.2, "Овочі"));
    foods_.push_back(Food("Огірок", 16, 4, 0.7, 0.1, "Овочі"));
    foods_.push_back(Food("Цибуля", 40, 9, 1.1, 0.1, "Овочі"));
    foods_.push_back(Food("Часник", 149, 33, 6.4, 0.5, "Овочі"));
    foods_.push_back(Food("Перець болгарський", 31, 6, 1, 0.3, "Овочі"));
    foods_.push_back(Food("Шпинат", 23, 3.6, 2.9, 0.4, "Овочі"));
    foods_.push_back(Food("Капуста", 25, 6, 1.3, 0.1, "Овочі"));
    foods_.push_back(Food("Буряк", 43, 10, 1.6, 0.2, "Овочі"));
    foods_.push_back(Food("Апельсин", 47, 12, 0.9, 0.1, "Фрукти"));
    foods_.push_back(Food("Груша", 57, 15, 0.4, 0.1, "Фрукти"));
    foods_.push_back(Food("Ківі", 61, 15, 1.1, 0.5, "Фрукти"));
    foods_.push_back(Food("Полуниця", 33, 8, 0.7, 0.3, "Фрукти"));
    foods_.push_back(Food("Черниця", 57, 14, 0.7, 0.3, "Фрукти"));
    foods_.push_back(Food("Виноград", 69, 18, 0.7, 0.2, "Фрукти"));

    // М'ясо та риба
    foods_.push_back(Food("Куряче філе", 165, 0, 31, 3.6, "М'ясо"));
    foods_.push_back(Food("Яловичина", 250, 0, 26, 15, "М'ясо"));
    foods_.push_back(Food("Свинина", 242, 0, 27, 14, "М'ясо"));
    foods_.push_back(Food("Лосось", 208, 0, 20, 13, "Риба"));
    foods_.push_back(Food("Тунець", 144, 0, 30, 1, "Риба"));
    foods_.push_back(Food("Яйце куряче", 155, 1.1, 13, 11, "М'ясо"));
    foods_.push_back(Food("Індичка філе", 135, 0, 29, 1, "М'ясо"));
    foods_.push_back(Food("Куряче стегно без шкіри", 209, 0, 26, 10.9, "М'ясо"));
    foods_.push_back(Food("Скумбрія", 205, 0, 19, 13.9, "Риба"));
    foods_.push_back(Food("Хек", 82, 0, 18, 0.7, "Риба"));

    // Молочні продукти
    foods_.push_back(Food("Молоко 3.2%", 60, 5, 3, 3.2, "Молочні"));
    foods_.push_back(Food("Сир білий", 60, 3, 11, 1, "Молочні"));
    foods_.push_back(Food("Йогурт натуральний", 59, 4, 5, 2.5, "Молочні"));
    foods_.push_back(Food("Сир твердий", 363, 2, 25, 27, "Молочні"));
    foods_.push_back(Food("Сметана 20%", 206, 3, 2.5, 20, "Молочні"));
    foods_.push_back(Food("Кефір 2.5%", 53, 4.7, 3, 2.5, "Молочні"));
    foods_.push_back(Food("Ряжанка 4%", 67, 4, 3, 4, "Молочні"));

    // Крупи та злаки
    foods_.push_back(Food("Рис варений", 130, 28, 2.7, 0.3, "Крупи"));
    foods_.push_back(Food("Гречка варена", 101, 20, 4, 1, "Крупи"));
    foods_.push_back(Food("Вівсянка", 389, 66, 17, 7, "Крупи"));
    foods_.push_back(Food("Макарони варені", 131, 25, 5, 1, "Крупи"));
    foods_.push_back(Food("Хліб білий", 265, 49, 8, 3.2, "Крупи"));
    foods_.push_back(Food("Хліб житній", 259, 48, 13, 3, "Крупи"));
    foods_.push_back(Food("Кіноа варена", 120, 21, 4.4, 1.9, "Крупи"));
    foods_.push_back(Food("Булгур варений", 83, 19, 3.1, 0.2, "Крупи"));
    foods_.push_back(Food("Кус-кус варений", 112, 23, 3.8, 0.2, "Крупи"));
    foods_.push_back(Food("Перловка варена", 123, 28, 2.3, 0.4, "Крупи"));

    // Бобові
    foods_.push_back(Food("Квасоля варена", 127, 23, 8.7, 0.5, "Бобові"));
    foods_.push_back(Food("Нут варений", 164, 27, 8.9, 2.6, "Бобові"));
    foods_.push_back(Food("Сочевиця варена", 116, 20, 9, 0.4, "Бобові"));
    foods_.push_back(Food("Горох варений", 118, 21, 7.9, 0.4, "Бобові"));

    // Горіхи та насіння
    foods_.push_back(Food("Миндаль", 579, 22, 21, 50, "Горіхи"));
    foods_.push_back(Food("Грецький горіх", 654, 14, 15, 65, "Горіхи"));
    foods_.push_back(Food("Арахіс", 567, 16, 26, 49, "Горіхи"));
    foods_.push_back(Food("Насіння соняшнику", 584, 20, 21, 51, "Горіхи"));
    foods_.push_back(Food("Насіння гарбуза", 559, 11, 30, 49, "Горіхи"));

    // Інше
    foods_.push_back(Food("Оливкова олія", 884, 0, 0, 100, "Інше"));
    foods_.push_back(Food("Мед", 304, 82, 0.3, 0, "Інше"));
    foods_.push_back(Food("Шоколад чорний", 546, 45, 5, 31, "Інше"));
    foods_.push_back(Food("Картопля варена", 82, 18, 2, 0.1, "Овочі"));
    foods_.push_back(Food("Авокадо", 160, 9, 2, 15, "Фрукти"));
    foods_.push_back(Food("Тост хліб", 267, 49, 9, 3.2, "Крупи"));
    foods_.push_back(Food("Сільські вершки 10%", 118, 3, 3, 10, "Молочні"));
    foods_.push_back(Food("Сир моцарела", 280, 3, 28, 17, "Молочні"));
    foods_.push_back(Food("Сир рікота", 174, 3, 11, 13, "Молочні"));
}



