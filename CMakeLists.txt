cmake_minimum_required(VERSION 3.10)
project(CalorieCalc)

set(CMAKE_CXX_STANDARD 20)

# SQLite (used by targets and tests)
find_package(SQLite3 QUIET)
if(SQLite3_FOUND)
    add_definitions(-DHAVE_SQLITE3)
    message(STATUS "SQLite3 found: ${SQLite3_VERSION}")
else()
    message(STATUS "SQLite3 not found - SQLite features disabled")
endif()

# Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen to build/html"
        VERBATIM
    )
    message(STATUS "Doxygen: run 'cmake --build <build-dir> --target docs' then open build/html/index.html")
endif()

# Tests and benchmarks (optional)
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)

if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_executable(CalorieCalcTests
        tests/main_test.cpp
        tests/test_food.cpp
        tests/test_diary.cpp
        tests/test_fooddatabase.cpp
        tests/test_savedmeal.cpp
        tests/test_statistics.cpp
        tests/test_savestrategies.cpp
        tests/test_training_plan.cpp
        tests/test_csv_import.cpp
        tests/test_parallel.cpp
        tests/test_training_json.cpp
        tests/test_error_handling.cpp
        tests/test_auth.cpp
        gui/AuthStore.cpp
        Food.cpp
        Diary.cpp
        SavedMeal.cpp
        FoodDatabase.cpp
        JsonSaveStrategy.cpp
        TextSaveStrategy.cpp
        StatisticsCalculator.cpp
        CsvImportExport.cpp
        TrainingSession.cpp
        TrainingPlanGenerator.cpp
        TrainingSession.cpp
        TrainingDiary.cpp
        TrainingJsonSaveStrategy.cpp
        Parallel.cpp
        BatchDiaryLoader.cpp
    )
    find_package(Qt6 COMPONENTS Core QUIET)
    if(Qt6_FOUND)
        target_link_libraries(CalorieCalcTests PRIVATE Qt6::Core)
    else()
        message(WARNING "Qt6 Core not found — test_training_json.cpp will not link; install Qt6 for full tests")
    endif()
    if(SQLite3_FOUND)
        target_sources(CalorieCalcTests PRIVATE SqliteSaveStrategy.cpp)
        target_link_libraries(CalorieCalcTests PRIVATE SQLite::SQLite3)
    endif()
    target_link_libraries(CalorieCalcTests PRIVATE gtest_main)
    include(GoogleTest)
    gtest_discover_tests(CalorieCalcTests)
endif()

if(BUILD_BENCHMARKS)
    include(FetchContent)
    FetchContent_Declare(
      googlebenchmark
      GIT_REPOSITORY https://github.com/google/benchmark.git
      GIT_TAG v1.8.3
    )
    FetchContent_MakeAvailable(googlebenchmark)

    add_executable(CalorieCalcBenchmark
        benchmarks/benchmark_main.cpp
        Food.cpp
        Diary.cpp
        SavedMeal.cpp
        FoodDatabase.cpp
    )
    target_link_libraries(CalorieCalcBenchmark PRIVATE benchmark::benchmark)
    
    # Benchmark validation tests
    add_executable(CalorieCalcBenchmarkTests
        benchmarks/benchmark_tests.cpp
        Food.cpp
        Diary.cpp
        SavedMeal.cpp
        FoodDatabase.cpp
    )
    target_link_libraries(CalorieCalcBenchmarkTests PRIVATE gtest_main benchmark::benchmark)
    target_include_directories(CalorieCalcBenchmarkTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    include(GoogleTest)
    gtest_discover_tests(CalorieCalcBenchmarkTests)
endif()

add_executable(CalorieCalc 
    main.cpp
    Food.cpp
    FoodDatabase.cpp
    SavedMeal.cpp
    Diary.cpp
    JsonSaveStrategy.cpp
    TextSaveStrategy.cpp
    StatisticsCalculator.cpp
    Logger.cpp
    CsvImportExport.cpp
    Parallel.cpp
    BatchDiaryLoader.cpp
)
if(SQLite3_FOUND)
    target_sources(CalorieCalc PRIVATE SqliteSaveStrategy.cpp)
    target_link_libraries(CalorieCalc PRIVATE SQLite::SQLite3)
    target_include_directories(CalorieCalc PRIVATE ${SQLite3_INCLUDE_DIRS})
endif()

# GUI target (Qt Widgets)
option(BUILD_GUI "Build Qt GUI" ON)
option(CALORIECALC_REQUIRE_CHARTS "Require Qt Charts for GUI (fail configure if Charts missing)" OFF)
option(BUILD_API_SERVER "Build REST API server (Qt HttpServer) for NFR-2 load tests" ON)

if(BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    list(APPEND CMAKE_PREFIX_PATH
        /opt/homebrew
        /opt/homebrew/opt/qt
        /opt/homebrew/opt/qt@6
        /opt/homebrew/opt/qtcharts
    )
    set(USE_QT_CHARTS FALSE)
    set(QT_MAJOR 6)

    find_package(Qt6 COMPONENTS Widgets PrintSupport Network QUIET)
    if(Qt6_FOUND)
        find_package(Qt6 COMPONENTS Charts QUIET)
        if(Qt6Charts_FOUND)
            set(QT_LIB Qt6::Widgets Qt6::PrintSupport Qt6::Network Qt6::Charts)
            set(USE_QT_CHARTS TRUE)
            message(STATUS "Using Qt6 with Charts")
        else()
            set(QT_LIB Qt6::Widgets Qt6::PrintSupport Qt6::Network)
            if(CALORIECALC_REQUIRE_CHARTS)
                message(FATAL_ERROR
                    "Qt6 Charts not found. Install Charts (e.g. brew install qtcharts) "
                    "or set -DCALORIECALC_REQUIRE_CHARTS=OFF")
            endif()
            message(WARNING "Qt6 Charts not found — GUI builds without visual charts")
        endif()
    else()
        set(QT_MAJOR 5)
        find_package(Qt5 COMPONENTS Widgets PrintSupport Network QUIET)
        if(Qt5_FOUND)
            find_package(Qt5 COMPONENTS Charts QUIET)
            if(Qt5Charts_FOUND)
                set(QT_LIB Qt5::Widgets Qt5::PrintSupport Qt5::Network Qt5::Charts)
                set(USE_QT_CHARTS TRUE)
                message(STATUS "Using Qt5 with Charts")
            else()
                set(QT_LIB Qt5::Widgets Qt5::PrintSupport Qt5::Network)
                if(CALORIECALC_REQUIRE_CHARTS)
                    message(FATAL_ERROR "Qt5 Charts not found. Install qtcharts or disable CALORIECALC_REQUIRE_CHARTS")
                endif()
            endif()
        else()
            message(FATAL_ERROR "Qt5 or Qt6 not found. Please install Qt.")
        endif()
    endif()

    add_executable(CalorieCalcGUI MACOSX_BUNDLE
        gui/main_gui.cpp
        gui/AppStyle.cpp
        gui/AppStyle.h
        gui/ProfileValidation.cpp
        gui/ProfileValidation.h
        gui/AuthStore.cpp
        gui/AuthStore.h
        gui/LoginDialog.cpp
        gui/LoginDialog.h
        gui/LegalDocuments.cpp
        gui/LegalDocuments.h
        gui/PerfTrace.cpp
        gui/PerfTrace.h
        gui/ProfileDialog.cpp
        gui/ProfileDialog.h
        gui/TrainingPlanWorkflowDialog.cpp
        gui/TrainingPlanWorkflowDialog.h
        gui/CloudAssistant.cpp
        gui/CloudAssistant.h
        gui/AssistantService.cpp
        gui/AssistantService.h
        gui/MainWindow.cpp
        gui/MainWindow.h
        Food.cpp
        FoodDatabase.cpp
        SavedMeal.cpp
        Diary.cpp
        JsonSaveStrategy.cpp
        TextSaveStrategy.cpp
        StatisticsCalculator.cpp
        Logger.cpp
        CsvImportExport.cpp
        TrainingSession.cpp
        TrainingDiary.cpp
        TrainingJsonSaveStrategy.cpp
        TrainingPlanGenerator.cpp
        OfflineAssistant.cpp
        Parallel.cpp
        BatchDiaryLoader.cpp
    )
    
    if(SQLite3_FOUND)
        target_sources(CalorieCalcGUI PRIVATE SqliteSaveStrategy.cpp)
        target_link_libraries(CalorieCalcGUI PRIVATE SQLite::SQLite3)
        target_include_directories(CalorieCalcGUI PRIVATE ${SQLite3_INCLUDE_DIRS})
    endif()
    
    if(BUILD_API_SERVER)
        if(QT_MAJOR EQUAL 6)
            find_package(Qt6 COMPONENTS Core Network HttpServer QUIET)
        else()
            find_package(Qt5 COMPONENTS Core Network QUIET)
            set(Qt6HttpServer_FOUND FALSE)
        endif()

        if(Qt6HttpServer_FOUND)
            add_executable(CalorieCalcAPI
                api/main_api.cpp
                api/rest_server.cpp
                Food.cpp
                FoodDatabase.cpp
                SavedMeal.cpp
                Diary.cpp
                JsonSaveStrategy.cpp
                TrainingSession.cpp
                TrainingDiary.cpp
                TrainingJsonSaveStrategy.cpp
                Parallel.cpp
            )
            target_link_libraries(CalorieCalcAPI PRIVATE
                Qt6::Core Qt6::Network Qt6::HttpServer)
            target_include_directories(CalorieCalcAPI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
            message(STATUS "CalorieCalcAPI enabled (Qt HttpServer)")
        else()
            message(WARNING "BUILD_API_SERVER=ON but Qt6 HttpServer not found — skip CalorieCalcAPI")
        endif()
    endif()
    
    target_link_libraries(CalorieCalcGUI PRIVATE ${QT_LIB})
    target_include_directories(CalorieCalcGUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    if(USE_QT_CHARTS)
        target_compile_definitions(CalorieCalcGUI PRIVATE QT_CHARTS_LIB)
    endif()

    # i18n (українська)
    set(TS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/translations/caloriecounter_uk.ts)
    if(QT_MAJOR EQUAL 6)
        find_package(Qt6 COMPONENTS LinguistTools QUIET)
        if(Qt6LinguistTools_FOUND)
            qt_add_translations(CalorieCalcGUI TS_FILES ${TS_FILE})
        else()
            find_program(LRELEASE_EXECUTABLE lrelease)
            if(LRELEASE_EXECUTABLE)
                set(QM_FILE ${CMAKE_CURRENT_BINARY_DIR}/caloriecounter_uk.qm)
                add_custom_command(OUTPUT ${QM_FILE}
                    COMMAND ${LRELEASE_EXECUTABLE} ${TS_FILE} -qm ${QM_FILE}
                    DEPENDS ${TS_FILE}
                    COMMENT "Generating caloriecounter_uk.qm")
                target_sources(CalorieCalcGUI PRIVATE ${QM_FILE})
            endif()
        endif()
    endif()

    find_program(LUPDATE_EXECUTABLE lupdate)
    if(LUPDATE_EXECUTABLE)
        add_custom_target(update_translations
            COMMAND ${LUPDATE_EXECUTABLE}
                ${CMAKE_SOURCE_DIR}/gui/MainWindow.cpp
                ${CMAKE_SOURCE_DIR}/gui/LoginDialog.cpp
                ${CMAKE_SOURCE_DIR}/gui/ProfileDialog.cpp
                ${CMAKE_SOURCE_DIR}/gui/TrainingPlanWorkflowDialog.cpp
                ${CMAKE_SOURCE_DIR}/gui/ProfileValidation.cpp
                -ts ${TS_FILE}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Update translations/caloriecounter_uk.ts from sources")
    endif()

    # macOS bundle settings
    if(APPLE)
        # Не ставте MACOSX_PACKAGE_LOCATION для цього plist — інакше він потрапляє в Resources/,
        # а macdeployqt шукає Contents/Info.plist і bundle «ламається».
        set_target_properties(CalorieCalcGUI PROPERTIES
            OUTPUT_NAME "CalorieCalc"
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist"
            MACOSX_RPATH ON
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "@executable_path/../Frameworks"
        )
        # macdeployqt викликає install_name_tool на exe: потрібен запас __LINKEDIT і сумісний Mach-O.
        # -no_fixup_chains: Xcode 13+ за замовчуванням «chained fixups» — класичний install_name_tool часто падає
        #   з «link edit information does not fill the __LINKEDIT segment» на arm64.
        # APPEND_STRING LINK_FLAGS: стійко до Ninja/Qt поруч із -Wl,-headerpad_max_install_names.
        set_property(TARGET CalorieCalcGUI APPEND_STRING PROPERTY LINK_FLAGS
            " -Wl,-headerpad,0x10000000 -Wl,-no_fixup_chains")

        # Без macdeployqt CLion запускає .app з Qt з Homebrew → CODESIGNING crash на libqcocoa.
        option(CALORIECALC_MAC_DEPLOY "Run macdeployqt + ad-hoc codesign after GUI build (macOS)" ON)
        if(CALORIECALC_MAC_DEPLOY)
            if(QT_MAJOR EQUAL 6)
                find_program(MACDEPLOYQT_EXECUTABLE macdeployqt6
                    HINTS /opt/homebrew/bin /usr/local/bin)
            else()
                find_program(MACDEPLOYQT_EXECUTABLE macdeployqt
                    HINTS /opt/homebrew/bin /usr/local/bin)
            endif()
            find_program(CODESIGN_EXECUTABLE codesign)

            if(MACDEPLOYQT_EXECUTABLE)
                set(_DEPLOY_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/deploy_macos.sh")
                # Після лінку лише підпис (швидко). Повний macdeployqt — target deploy_gui (надійніше).
                add_custom_command(TARGET CalorieCalcGUI POST_BUILD
                    COMMAND bash "${_DEPLOY_SCRIPT}" --sign-only "$<TARGET_FILE:CalorieCalcGUI>"
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                    COMMENT "macOS: codesign CalorieCalc.app (incremental)"
                )
                add_custom_target(deploy_gui
                    COMMAND bash "${_DEPLOY_SCRIPT}" "$<TARGET_FILE:CalorieCalcGUI>"
                    DEPENDS CalorieCalcGUI
                    COMMENT "Re-run macdeployqt on CalorieCalc.app"
                )
                add_custom_target(reset_gui_dev
                    COMMAND bash "${_DEPLOY_SCRIPT}" --reset-dev "$<TARGET_FILE:CalorieCalcGUI>"
                    COMMENT "Remove bundled Qt; next CalorieCalcGUI build uses Homebrew (CLion dev)"
                )
                message(STATUS "macOS: post-build deploy via ${_DEPLOY_SCRIPT}")
            else()
                message(WARNING "macdeployqt not found — install Qt (brew install qt). "
                    "GUI may crash on launch from CLion with CODESIGNING Invalid Page.")
            endif()
        endif()
    endif()
endif()
