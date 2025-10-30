# Діаграми послідовності (Sequence Diagrams)

## 1. Додавання продукту до щоденника

```mermaid
sequenceDiagram
    participant User as Користувач
    participant Main as main()
    participant DB as FoodDatabase
    participant Food as Food
    participant Meal as SavedMeal
    participant Diary as Diary
    
    User->>Main: Вводить назву продукту
    Main->>DB: searchFoods(query)
    DB-->>Main: vector<Food> results
    Main->>User: Відображає результати
    User->>Main: Обирає продукт та кількість
    Main->>Food: getName(), getCalories(), etc.
    Food-->>Main: Дані продукту
    Main->>Meal: new SavedMeal(mealType, food, amount)
    Main->>Diary: addMeal(meal)
    Diary-->>Main: Підтвердження
    Main->>User: Повідомлення про успіх
```

## 2. Розрахунок статистики дня

```mermaid
sequenceDiagram
    participant User as Користувач
    participant Main as main()
    participant Diary as Diary
    participant Meal as SavedMeal
    participant Food as Food
    
    User->>Main: Запит статистики
    Main->>Diary: getAllMeals()
    Diary-->>Main: vector<SavedMeal>
    Main->>Diary: getTotalCalories()
    Diary->>Meal: getTotalCalories() (для кожного)
    Meal->>Food: getTotalCalories()
    Food-->>Meal: Калорії
    Meal-->>Diary: Калорії прийому їжі
    Diary-->>Main: Загальні калорії
    Main->>Diary: getTotalCarbs(), getTotalProtein(), getTotalFat()
    Diary-->>Main: Макроелементи
    Main->>Diary: getCalorieGoal()
    Diary-->>Main: Ціль калорій
    Main->>Diary: getRemainingCalories()
    Diary-->>Main: Залишок
    Main->>User: Відображення статистики
```

## 3. Збереження щоденника

```mermaid
sequenceDiagram
    participant User as Користувач
    participant Main as main()
    participant Strategy as ISaveStrategy
    participant JsonStrategy as JsonSaveStrategy
    participant Diary as Diary
    
    User->>Main: Запит збереження
    Main->>User: Запит формату
    User->>Main: JSON формат
    Main->>JsonStrategy: new JsonSaveStrategy()
    Main->>User: Запит імені файлу
    User->>Main: Ім'я файлу
    Main->>JsonStrategy: save(diary, filename)
    JsonStrategy->>Diary: getAllMeals()
    Diary-->>JsonStrategy: vector<SavedMeal>
    JsonStrategy->>JsonStrategy: Форматування JSON
    JsonStrategy->>JsonStrategy: Запис у файл
    JsonStrategy-->>Main: true/false
    Main->>User: Повідомлення про результат
```

## 4. Завантаження щоденника

```mermaid
sequenceDiagram
    participant User as Користувач
    participant Main as main()
    participant Strategy as ISaveStrategy
    participant TextStrategy as TextSaveStrategy
    participant Diary as Diary
    participant Meal as SavedMeal
    participant Food as Food
    
    User->>Main: Запит завантаження
    Main->>User: Запит формату
    User->>Main: Текстовий формат
    Main->>TextStrategy: new TextSaveStrategy()
    Main->>User: Запит імені файлу
    User->>Main: Ім'я файлу
    Main->>TextStrategy: load(diary, filename)
    TextStrategy->>TextStrategy: Читання файлу
    TextStrategy->>Diary: clear()
    TextStrategy->>Diary: setCalorieGoal(goal)
    loop Для кожного прийому їжі
        TextStrategy->>Food: new Food(name, cal, carbs, prot, fat)
        TextStrategy->>Meal: new SavedMeal(type, food, amount)
        TextStrategy->>Diary: addMeal(meal)
    end
    TextStrategy-->>Main: true/false
    Main->>User: Повідомлення про результат
```

## 5. Розширена статистика

```mermaid
sequenceDiagram
    participant User as Користувач
    participant Main as main()
    participant StatsCalc as StatisticsCalculator
    participant Diary as Diary
    participant Meal as SavedMeal
    
    User->>Main: Запит розширеної статистики
    Main->>StatsCalc: new StatisticsCalculator()
    Main->>StatsCalc: addDiary(diary)
    StatsCalc->>Diary: getAllMeals()
    Diary-->>StatsCalc: vector<SavedMeal>
    Main->>StatsCalc: calculateAverage()
    StatsCalc->>Diary: getTotalCalories(), etc.
    Diary-->>StatsCalc: Значення
    StatsCalc-->>Main: DailyStats
    Main->>StatsCalc: getMealTypeStatistics()
    StatsCalc->>Meal: getMealName(), getTotalCalories()
    loop Для кожного прийому їжі
        Meal-->>StatsCalc: Дані
        StatsCalc->>StatsCalc: Групування по типу
    end
    StatsCalc-->>Main: map<string, double>
    Main->>StatsCalc: getMostUsedFoods(5)
    StatsCalc->>Meal: getFood().getName()
    loop Для кожного прийому їжі
        Meal-->>StatsCalc: Назва продукту
        StatsCalc->>StatsCalc: Підрахунок
    end
    StatsCalc-->>Main: map<string, int>
    Main->>User: Відображення розширеної статистики
```

