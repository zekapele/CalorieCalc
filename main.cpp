#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "FoodDatabase.h"
#include "Diary.h"
#include "SavedMeal.h"
#include "JsonSaveStrategy.h"
#include "TextSaveStrategy.h"
#include "StatisticsCalculator.h"

static void clearCin() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static void printMenu() {
    std::cout << "\n===== Калькулятор калорій (CLI) =====\n";
    std::cout << "1. Пошук продуктів\n";
    std::cout << "2. Додати прийом їжі\n";
    std::cout << "3. Показати всі прийоми їжі\n";
    std::cout << "4. Видалити прийом їжі\n";
    std::cout << "5. Встановити ціль калорій\n";
    std::cout << "6. Показати денну статистику\n";
    std::cout << "7. Зберегти (JSON)\n";
    std::cout << "8. Завантажити (JSON)\n";
    std::cout << "9. Зберегти (TXT)\n";
    std::cout << "10. Завантажити (TXT)\n";
    std::cout << "0. Вихід\n";
    std::cout << "> Оберіть дію: ";
}

int main() {
    FoodDatabase foodDb;
    Diary diary;
    JsonSaveStrategy jsonSaver;
    TextSaveStrategy txtSaver;

    while (true) {
        printMenu();
        int choice = -1;
        if (!(std::cin >> choice)) {
            clearCin();
            std::cout << "Невірне введення. Спробуйте ще раз." << std::endl;
            continue;
        }
        clearCin();

        if (choice == 0) {
            std::cout << "До побачення!" << std::endl;
            break;
        }

        switch (choice) {
            case 1: {
                std::cout << "Введіть запит для пошуку: ";
                std::string query;
                std::getline(std::cin, query);
                auto results = foodDb.searchFoods(query);
                if (results.empty()) {
                    std::cout << "Нічого не знайдено." << std::endl;
                } else {
                    std::cout << "Знайдено (назва — ккал на 100г):\n";
                    for (size_t i = 0; i < results.size(); ++i) {
                        std::cout << i + 1 << ") " << results[i].getName() << " — "
                                  << results[i].getCalories() << " ккал" << std::endl;
                    }
                }
                break;
            }
            case 2: {
                std::cout << "Введіть запит для пошуку продукту: ";
                std::string query;
                std::getline(std::cin, query);
                auto results = foodDb.searchFoods(query);
                if (results.empty()) {
                    std::cout << "Нічого не знайдено." << std::endl;
                    break;
                }
                for (size_t i = 0; i < results.size(); ++i) {
                    std::cout << i + 1 << ") " << results[i].getName() << " — "
                              << results[i].getCalories() << " ккал" << std::endl;
                }
                std::cout << "Оберіть номер продукту: ";
                int idx;
                if (!(std::cin >> idx) || idx < 1 || idx > static_cast<int>(results.size())) {
                    clearCin();
                    std::cout << "Невірний вибір." << std::endl;
                    break;
                }
                clearCin();
                Food chosen = results[static_cast<size_t>(idx - 1)];

                std::cout << "Вкажіть кількість в грамах (напр. 150): ";
                double grams;
                if (!(std::cin >> grams) || grams <= 0) {
                    clearCin();
                    std::cout << "Невірне значення грамів." << std::endl;
                    break;
                }
                clearCin();

                std::cout << "Введіть назву прийому їжі (Сніданок/Обід/Вечеря/Перекус): ";
                std::string mealName;
                std::getline(std::cin, mealName);
                if (mealName.empty()) mealName = "Прийом їжі";

                SavedMeal meal(mealName, chosen, grams);
                diary.addMeal(meal);
                std::cout << "Додано: " << meal.getMealName() << ", " << chosen.getName()
                          << ", " << grams << " г ("
                          << meal.getTotalCalories() << " ккал)" << std::endl;
                break;
            }
            case 3: {
                diary.printAllMeals();
                break;
            }
            case 4: {
                auto meals = diary.getAllMeals();
                if (meals.empty()) {
                    std::cout << "Список порожній." << std::endl;
                    break;
                }
                for (size_t i = 0; i < meals.size(); ++i) {
                    std::cout << i + 1 << ") [" << meals[i].getMealName() << "] "
                              << meals[i].getFood().getName() << " — "
                              << meals[i].getTotalCalories() << " ккал" << std::endl;
                }
                std::cout << "Оберіть номер для видалення: ";
                int idx;
                if (!(std::cin >> idx) || idx < 1 || idx > static_cast<int>(meals.size())) {
                    clearCin();
                    std::cout << "Невірний вибір." << std::endl;
                    break;
                }
                clearCin();
                diary.removeMeal(idx - 1);
                std::cout << "Видалено." << std::endl;
                break;
            }
            case 5: {
                std::cout << "Введіть ціль калорій: ";
                double goal;
                if (!(std::cin >> goal) || goal <= 0) {
                    clearCin();
                    std::cout << "Невірне значення цілі." << std::endl;
                    break;
                }
                clearCin();
                diary.setCalorieGoal(goal);
                std::cout << "Ціль встановлено: " << goal << " ккал" << std::endl;
                break;
            }
            case 6: {
                diary.printDailySummary();
                break;
            }
            case 7: {
                std::cout << "Введіть ім'я файлу без розширення: ";
                std::string name;
                std::getline(std::cin, name);
                if (name.empty()) name = "diary";
                if (jsonSaver.save(diary, name)) {
                    std::cout << "Збережено у файл: " << name << jsonSaver.getExtension() << std::endl;
                } else {
                    std::cout << "Помилка збереження." << std::endl;
                }
                break;
            }
            case 8: {
                std::cout << "Введіть ім'я файлу без розширення: ";
                std::string name;
                std::getline(std::cin, name);
                if (name.empty()) name = "diary";
                if (jsonSaver.load(diary, name)) {
                    std::cout << "Завантажено з: " << name << jsonSaver.getExtension() << std::endl;
                } else {
                    std::cout << "Помилка завантаження." << std::endl;
                }
                break;
            }
            case 9: {
                std::cout << "Введіть ім'я файлу без розширення: ";
                std::string name;
                std::getline(std::cin, name);
                if (name.empty()) name = "diary";
                if (txtSaver.save(diary, name)) {
                    std::cout << "Збережено у файл: " << name << txtSaver.getExtension() << std::endl;
                } else {
                    std::cout << "Помилка збереження." << std::endl;
                }
                break;
            }
            case 10: {
                std::cout << "Введіть ім'я файлу без розширення: ";
                std::string name;
                std::getline(std::cin, name);
                if (name.empty()) name = "diary";
                if (txtSaver.load(diary, name)) {
                    std::cout << "Завантажено з: " << name << txtSaver.getExtension() << std::endl;
                } else {
                    std::cout << "Помилка завантаження." << std::endl;
                }
                break;
            }
            default:
                std::cout << "Невірний вибір. Спробуйте ще раз." << std::endl;
                break;
        }
    }

    return 0;
}
