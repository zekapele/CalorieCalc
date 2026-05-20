# Traceability: UR/FR/NFR → реалізація (актуально після фази 3)

Матриця відповідності вимог бізнес-аналізу поточному коду та UI **CalorieCalc**.

**Позначення:** ✅ повністю · ◐ частково · ⏳ roadmap

| ID | Вимога | Статус | Доказ у проєкті |
|----|--------|--------|-----------------|
| **UR-1** | Персональний профіль | ✅ | `AuthStore`, `LoginDialog`, `ProfileDialog`, вік/зріст/вага в `MainWindow` |
| FR-1.1 | Реєстрація email / Google / Apple | ◐ | Email + пароль; локальний профіль Google/Apple |
| FR-1.2 | Вік, вага, зріст, ціль | ✅ | `ageSpin_`, `heightSpin_`, `weightSpin_`, `trainingGoalCombo_`, `profile.json` |
| **UR-2** | План тренувань | ✅ | |
| FR-2.1 | Генерація плану | ✅ | `TrainingPlanGenerator`, `TrainingPlanWorkflowDialog`, `onGenerateTrainingPlan7Days` |
| FR-2.2 | Редагування плану | ✅ | «Редагувати план», додати/видалити сесію, майстер, `onStartWorkout` |
| **UR-3** | Калорії | ✅ | |
| FR-3.1 | Журнал продуктів | ✅ | `onAddFood`, `onAddCustomFood`, секції дня |
| FR-3.2 | Підрахунок за день | ✅ | `Diary::getTotalCalories`, `refreshStats` |
| **UR-4** | Помічник | ✅ | |
| FR-4.1 | Питання | ✅ | `onAskOfflineAssistant`, `CloudAssistant` (опційно) |
| FR-4.2 | Рекомендації за даними | ✅ | `OfflineAssistant`, фідбек у діалозі |
| **UR-5** | Прогрес | ✅ | |
| FR-5.1 | Статистика | ✅ | Звіти 7 днів, KPI, «Зведення прогресу» |
| FR-5.2 | Графіки ваги та активності | ✅ | `onShowCharts` (вага, калорії, вода, хвилини тренувань) |
| **NFR-1** | Відгук ≤2 с | ✅ | `PerfTrace`, `statusBar`, меню **«Перевірка швидкості…»** |
| **NFR-2** | Масштаб (100k users) | ⏳ | `api/rest_server`, `tools/loadtest/load_api.py` |
| **NFR-4** | Захист ПД | ✅ | SHA-256, згода, legal, видалення акаунта; тести `tests/test_auth.cpp` |
| **NFR-5** | iOS / Android | ⏳ | Roadmap; поточна збірка — Qt Desktop |

**User stories:** [UserStories_WorkoutManagement.md](UserStories_WorkoutManagement.md) (US-W1…US-W5).

**У застосунку:** меню **Довідка → Вимоги UR/FR/NFR (зведення)…** — короткий огляд для захисту.

Повний перелік: [Requirements_UR_FR_NFR.md](Requirements_UR_FR_NFR.md).
