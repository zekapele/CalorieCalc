# Use Case

Ключові взаємодії користувача з застосунком.

```mermaid
graph TB
    User[Користувач] --> SearchFood[Пошук продукту]
    User --> AddFood[Додати у щоденник]
    User --> ViewDiary[Перегляд щоденника]
    User --> ViewStats[Статистика дня]
    User --> DeleteMeal[Видалити прийом]
    User --> SetGoal[Ціль калорій]
    User --> SaveDiary[Зберегти]
    User --> LoadDiary[Завантажити]
    User --> Water[Вода]
    User --> Weight[Вага]
    User --> Templates[Шаблони]
    User --> Calendar[Календар/Профіль]

    SearchFood --> FoodDatabase[База продуктів]
    AddFood --> Diary[Щоденник]
    ViewDiary --> Diary
    ViewStats --> Diary
    DeleteMeal --> Diary
    SetGoal --> Diary
    SaveDiary --> SaveStrategy[Стратегія збереження]
    LoadDiary --> SaveStrategy
```

## Сценарії (стисло)
- Пошук продукту: введення назви → результати з бази.
- Додавання: вибір продукту, кількість, секція → запис у щоденник.
- Статистика: сума калорій/макро, прогрес до цілі, вода.
- Календар/Профіль: перемикає день/сховище даних.
- Збереження/Завантаження: JSON по даті/профілю.

