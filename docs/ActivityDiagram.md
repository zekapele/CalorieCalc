# Діаграми активності (Activity Diagrams)

## 1. Додавання продукту до щоденника

```mermaid
flowchart TD
    Start([Початок]) --> Search[Введення назви продукту]
    Search --> CheckResults{Знайдено продукти?}
    CheckResults -->|Ні| NotFound[Повідомлення: не знайдено]
    NotFound --> End([Кінець])
    CheckResults -->|Так| DisplayResults[Відображення результатів]
    DisplayResults --> UserChoice{Користувач обирає продукт?}
    UserChoice -->|Ні| End
    UserChoice -->|Так| EnterAmount[Введення кількості]
    EnterAmount --> SelectMealType[Обрання прийому їжі]
    SelectMealType --> CreateMeal[Створення SavedMeal]
    CreateMeal --> AddToDiary[Додавання до Diary]
    AddToDiary --> Success[Повідомлення про успіх]
    Success --> End
```

## 2. Розрахунок статистики дня

```mermaid
flowchart TD
    Start([Запит статистики]) --> GetMeals[Отримання всіх прийомів їжі]
    GetMeals --> CheckEmpty{Щоденник порожній?}
    CheckEmpty -->|]}EmptyMessage[Відображення: щоденник порожній]
    EmptyMessage --> End([Кінець])
    CheckEmpty -->|Ні| CalculateCalories[Розрахунок калорій]
    CalculateCalories --> CalculateCarbs[Розрахунок вуглеводів]
    CalculateCarbs --> CalculateProtein[Розрахунок білків]
    CalculateProtein --> CalculateFat[Розрахунок жирів]
    CalculateFat --> GetGoal[Отримання цілі калорій]
    GetGoal --> CalculateRemaining[Розрахунок залишку]
    CalculateRemaining --> DisplayStats[Відображення статистики]
    DisplayStats --> End
```

## 3. Збереження щоденника

```mermaid
flowchart TD
    Start([Запит збереження]) --> SelectFormat[Обрання формату]
    SelectFormat --> CheckFormat{Формат обрано?}
    CheckFormat -->|Ні| ErrorFormat[Помилка: невірний формат]
    ErrorFormat --> End([Кінець])
    CheckFormat -->|JSON| CreateJsonStrategy[Створення JsonSaveStrategy]
    CheckFormat -->|Текст| CreateTextStrategy[Створення TextSaveStrategy]
    CreateJsonStrategy --> EnterFilename[Введення імені файлу]
    CreateTextStrategy --> EnterFilename
    EnterFilename --> CallSave[Виклик методу save]
    CallSave --> SaveSuccess{Збережено успішно?}
    SaveSuccess -->|Ні| ErrorSave[Помилка збереження]
    SaveSuccess -->|Так| SuccessMessage[Повідомлення про успіх]
    ErrorSave --> End
    SuccessMessage --> End
```

## 4. Розширена статистика

```mermaid
flowchart TD
    Start([Запит розширеної статистики]) --> CreateCalculator[Створення StatisticsCalculator]
    CreateCalculator --> AddDiary[Додавання Diary до калькулятора]
    AddDiary --> CalcAverage[Розрахунок середніх значень]
    CalcAverage --> CalcMealStats[Розрахунок статистики по прийомах їжі]
    CalcMealStats --> GetPopularFoods[Отримання найпопулярніших продуктів]
    GetPopularFoods --> GroupByMealType[Групування по типу прийому їжі]
    GroupByMealType --> CountFoods[Підрахунок використань продуктів]
    CountFoods --> DisplayExtended[Відображення розширеної статистики]
    DisplayExtended --> End([Кінець])
```



