# Документація проєкту «CalorieCalc» (десктопний клієнт)

Цей репозиторій містить **настільний клієнт (Qt)** **CalorieCalc** для обліку харчування та тренувань: профіль, план на тиждень, щоденники, звіти. Бачення та межі — у [docs/Vision_CalorieCalc.md](docs/Vision_CalorieCalc.md).

**Пріоритизація MVP (MoSCoW + scoring):** [docs/Prioritization_MVP.md](docs/Prioritization_MVP.md).

**Аналіз документів, аудиторії та конкурентів:** [docs/RequirementsAnalysis_Audience_Competitors.md](docs/RequirementsAnalysis_Audience_Competitors.md) (аналоги, порівняльні таблиці, відмінності від комерційних продуктів).

**UR / FR / NFR (курсове ТЗ):** [docs/Requirements_UR_FR_NFR.md](docs/Requirements_UR_FR_NFR.md).

**BPMN (план тренувань):** [docs/BPMN_TrainingPlanGeneration.md](docs/BPMN_TrainingPlanGeneration.md) (діаграма: `docs/images/BPMN_TrainingPlanGeneration.png`).

**UC-01 (Start Workout):** [docs/Flowchart_StartWorkout_UC01.md](docs/Flowchart_StartWorkout_UC01.md) (діаграма: `docs/images/UC01_StartWorkout_Flow.png`).

**User stories (epic Workout Management):** [docs/UserStories_WorkoutManagement.md](docs/UserStories_WorkoutManagement.md) — US-W1…US-W5 (21 SP).

**Traceability (UR/FR → код):** [docs/RequirementsTraceability.md](docs/RequirementsTraceability.md).

**Здача «Основи ООП»:** [docs/SUBMISSION_OOP.md](docs/SUBMISSION_OOP.md) · **Стиль коду:** [docs/CODE_STYLE.md](docs/CODE_STYLE.md).

**У GUI (меню «Довідка»):** зведення UR/FR/NFR, продуктивність (NFR), перевірка швидкості, паралельні обчислення, бачення продукту.

**Інженерія:** unit tests (`BUILD_TESTS=ON`), Doxygen (`cmake --build build --target docs`), лог `data/caloriecalc_gui.log`, i18n `translations/`, CI `.github/workflows/ci.yml`.

## 1. Ідея та межі проєкту

Трекер харчування та фітнесу: калорії, макроелементи, вода, вага, **план тренувань** (детермінований генератор з адаптацією тривалості), офлайн-помічник, історія тренувань. Основний сценарій: знайти продукт → додати в день; на вкладці «Фітнес» — планувати та виконувати тренування.

Ціль курсової частини — продемонструвати чисту ООП‑модель, базові патерни та акуратний користувацький інтерфейс без зовнішніх сервісів.



## 3. Функціональність (фінальна)

- Пошук продуктів у вбудованій базі (+ додавання власних продуктів).
- Чотири секції дня: Сніданок / Обід / Вечеря / Перекус (вкладки).
- Цілі та статистика за день; прогресбар калорій.
- Вода: ціль і фактичне споживання (мл) з прогресбаром.
- Вага: фіксація значення на обраний день.
- Шаблони прийомів: зберегти склад секції і додати його повторно за один клік.
- Профілі (у т.ч. «Без профілю») та календар: окремий щоденник на кожну дату/профіль.
- **Копіювати підсумок дня** у буфер обміну; **Типові БЖВ** (150 / 250 / 70 г); подвійний клік по рядку тренування — редагування сесії.
- Зберігання JSON‑файлів у `./data/<профіль або guest>/<YYYY-MM-DD>.json` (автоматично).
- Консольна версія для демонстрації моделі (CLI).

## 4. Архітектура

- Модель: `Food`, `SavedMeal`, `Diary`, `FoodDatabase`.
- Збереження: патерн Strategy — `ISaveStrategy`, реалізації `JsonSaveStrategy`, `TextSaveStrategy`.
- GUI: Qt Widgets; композиція екранів без важких фреймворків.
- Розширення: додаткові поля `Diary` (вода, ціль води, вага) без ламання існуючих API.



```bash
mkdir build && cd build
cmake -DBUILD_GUI=ON ..
make -j
```

- GUI: `./CalorieCalcGUI` (на macOS — `CalorieCalc.app`;
  для деплойменту — `macdeployqt CalorieCalc.app`).
- CLI: `./CalorieCalc`.

## 7. Структура

```
CalorieCalc/
├─ gui/            # Графічний інтерфейс (Qt)
├─ docs/           # Діаграми та допоміжні матеріали
├─ *.h, *.cpp      # Модель, база продуктів, стратегії збереження
├─ CMakeLists.txt
└─ data/           # JSON-файли за профілями/датами (створюється під час роботи)
```

## 8. Плани розвитку

- Експорт/імпорт раціонів (CSV/JSON) і просте порівняння днів — частково реалізовано (CSV).
- Графіки за тиждень/місяць (калорії, вода, вага) — за наявності Qt Charts.
- Локалізація інтерфейсу, налаштування макрос‑цілей — **фаза 3**: цілі Б/Ж/В у моделі, збереженні та GUI.

## 9. Фаза 3

Детальний опис: `docs/Phase3_Report.md`.

---



