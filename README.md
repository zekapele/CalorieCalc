# CalorieCalc

**CalorieCalc** — настільний застосунок для обліку харчування та тренувань (курс «Основи ООП», 2025–2026). Дані зберігаються локально в `data/`.

## Можливості

- Щоденник харчування, макроси, вода, вага, ІМТ
- План тренувань на 7 днів, UC-01 «Почати тренування»
- Графіки (Qt Charts), експорт CSV/PDF
- Помічник (офлайн; опційно хмара за API-ключем)
- REST API та load test (NFR-2, опційно)

## Стиль коду

[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) + `.clang-format` — деталі в [docs/CODE_STYLE.md](docs/CODE_STYLE.md).

## Збірка

Потрібен **окремий каталог збірки** і шлях до **кореня проєкту** (там, де `CMakeLists.txt`), а не до батьківської папки `CLionProjects`.

```bash
mkdir -p build && cd build
cmake -DBUILD_GUI=ON -DBUILD_TESTS=ON -DCALORIECALC_REQUIRE_CHARTS=OFF ..
cmake --build . -j
ctest --output-on-failure
```

Якщо ви вже всередині `CalorieCalc/`, то `..` вказує на `CLionProjects` без `CMakeLists.txt` — використайте наприклад `cmake -S . -B ../caloriecalc-build` або зайдіть у `cmake-build-debug` і виконайте `cmake -S .. -B .` (як у CLion).

macOS (графіки): `brew install qt qtcharts`, потім `-DCALORIECALC_REQUIRE_CHARTS=ON`.

Опційно: microbenchmarks — `-DBUILD_BENCHMARKS=ON`.

## Запуск

```bash
open CalorieCalc.app          # macOS, після збірки GUI
./CalorieCalc                 # консоль
```

Лог GUI: `data/caloriecalc_gui.log`.

### macOS: іконка стрибає і вікно не з’являється

1. Зупиніть усі копії: `killall CalorieCalc 2>/dev/null`
2. Перезберіть і deploy:
   ```bash
   cmake --build cmake-build-debug --target CalorieCalcGUI
   bash scripts/deploy_macos.sh cmake-build-debug/CalorieCalc.app
   ```
3. Дані зберігаються в `~/Library/Application Support/CalorieCalc/data/` (не в `/`).

Для розробки з даними в папці збірки:  
`export CALORIECALC_DATA_DIR=/Users/you/CLionProjects/CalorieCalc/cmake-build-debug`

### macOS: падіння `CODESIGNING / Invalid Page` (CLion)

Причина: Qt не в bundle, або `macdeployqt` ще копіює Frameworks під час запуску, або дані/логи потрапили в `Contents/MacOS/data`. Часто ще: **exe з Homebrew**, а в `.app` лишилися **`Contents/PlugIns`** від невдалого deploy — тоді `dyld` падає при `dlopen(libqcocoa)` (у звіті видно `QLibraryPrivate::load_sys`).

Після збірки скрипт `--sign-only` прибирає такі залишки; вручну можна: `rm -rf cmake-build-debug/CalorieCalc.app/Contents/PlugIns`. Якщо після цього з’являється **«Could not find the Qt platform plugin cocoa»**, перезберіть GUI: у `main_gui.cpp` перед `QApplication` підставляється `QT_PLUGIN_PATH` з Homebrew, якщо в bundle немає `libqcocoa`.

1. Після **чистої** збірки один раз:
   ```bash
   cmake --build cmake-build-debug --target deploy_gui
   ```
2. У CLion → Run/Debug Configuration:
   - **Executable:** `cmake-build-debug/CalorieCalc.app/Contents/MacOS/CalorieCalc`
   - **Working directory:** `$ProjectFileDir$/cmake-build-debug` (не `MacOS` всередині `.app`)
3. Не натискайте Run, поки збірка не завершилась повністю.
4. Запуск з термінала: `./scripts/run_macos.sh`

Потрібні `macdeployqt6` і `codesign` (Xcode Command Line Tools).

**Не видаляйте цілком** `CalorieCalc.app` (`rm -rf …/CalorieCalc.app`): CMake очікує bundle у каталозі збірки; без нього не з’явиться `Contents/Info.plist`, і `macdeployqt` / `install_name_tool` часто падають. Для «чистого» Qt-bundle достатньо `cmake --build … --target deploy_gui` або видалити лише `Contents/Frameworks`, `Contents/PlugIns` і `_CodeSignature`, потім знову зібрати `CalorieCalcGUI`.

Якщо `deploy_gui` падає з **`install_name_tool` … `__LINKEDIT`**: після `git pull` / зміни `CMakeLists` виконайте `cmake -S .. -B .`, видаліть `CalorieCalc.app/Contents/MacOS/CalorieCalc`, знову `CalorieCalcGUI`, потім `deploy_gui` (лінкер додає `-no_fixup_chains` і великий `-headerpad` саме для `macdeployqt`).

**Повернути режим розробки** (як коли «зараз працює» з CLion), якщо після `deploy_gui` застосунок не стартує:

```bash
cd cmake-build-debug   # ви вже в цьому каталозі — не робіть cd cmake-build-debug ще раз
cmake --build . --target reset_gui_dev
cmake --build . --target CalorieCalcGUI
```

`deploy_gui` опційний; для щоденної роботи в CLion його можна не запускати.

## Doxygen (обов’язково 2-й семестр)

Потрібен [Doxygen](https://www.doxygen.org/):

```bash
cmake -DBUILD_GUI=OFF -DBUILD_TESTS=OFF ..
cmake --build . --target docs
open html/index.html
```

CI також генерує HTML (артефакт `doxygen-html`).

## i18n

```bash
cmake --build build --target update_translations   # оновити .ts
# lrelease / qt_add_translations створює .qm при збірці GUI
```

Файл: `translations/caloriecounter_uk.ts`.

## Здача курсу

Чеклист вимог: [docs/SUBMISSION_OOP.md](docs/SUBMISSION_OOP.md)

## Бізнес-аналіз і UML

| Артефакт | Файл |
|----------|------|
| Здача ООП | [docs/SUBMISSION_OOP.md](docs/SUBMISSION_OOP.md) |
| Бачення | [docs/Vision_CalorieCalc.md](docs/Vision_CalorieCalc.md) |
| UR / FR / NFR | [docs/Requirements_UR_FR_NFR.md](docs/Requirements_UR_FR_NFR.md) |
| Traceability | [docs/RequirementsTraceability.md](docs/RequirementsTraceability.md) |
| User stories | [docs/UserStories_WorkoutManagement.md](docs/UserStories_WorkoutManagement.md) |
| MoSCoW | [docs/Prioritization_MVP.md](docs/Prioritization_MVP.md) |
| Аудиторія / конкуренти | [docs/RequirementsAnalysis_Audience_Competitors.md](docs/RequirementsAnalysis_Audience_Competitors.md) |
| Фаза 3 | [docs/Phase3_Report.md](docs/Phase3_Report.md) |

У GUI (**Довідка**): UR/FR/NFR, NFR-1, паралельність, бачення продукту.

## Помічник (хмара)

`CALORIECALC_API_KEY` або `OPENAI_API_KEY` у середовищі.

## CI

GitHub Actions: `.github/workflows/ci.yml` — збірка, `ctest`, Doxygen.
