cmake_minimum_required(VERSION 3.10)
project(CalorieCalc)

set(CMAKE_CXX_STANDARD 20)

# Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
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
        tests/test_csv_import.cpp
        Food.cpp
        Diary.cpp
        SavedMeal.cpp
        FoodDatabase.cpp
        JsonSaveStrategy.cpp
        TextSaveStrategy.cpp
        StatisticsCalculator.cpp
        CsvImportExport.cpp
    )
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

# SQLite
find_package(SQLite3 QUIET)
if(SQLite3_FOUND)
    add_definitions(-DHAVE_SQLITE3)
    message(STATUS "SQLite3 found: ${SQLite3_VERSION}")
else()
    message(STATUS "SQLite3 not found - SQLite features disabled")
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
)
if(SQLite3_FOUND)
    target_sources(CalorieCalc PRIVATE SqliteSaveStrategy.cpp)
    target_link_libraries(CalorieCalc PRIVATE SQLite::SQLite3)
    target_include_directories(CalorieCalc PRIVATE ${SQLite3_INCLUDE_DIRS})
endif()

# GUI target (Qt Widgets)
option(BUILD_GUI "Build Qt GUI" ON)
if(BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    # Set CMAKE_PREFIX_PATH to help find Qt
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} /opt/homebrew)
    set(USE_QT_CHARTS FALSE)
    
    # Try Qt6 first
    find_package(Qt6 COMPONENTS Widgets PrintSupport Network QUIET)
    if(Qt6_FOUND)
        # Check if Charts is available
        find_package(Qt6 COMPONENTS Charts QUIET)
        if(Qt6Charts_FOUND)
            set(QT_LIB Qt6::Widgets Qt6::PrintSupport Qt6::Network Qt6::Charts)
            set(USE_QT_CHARTS TRUE)
            message(STATUS "Using Qt6 with Charts")
        else()
            set(QT_LIB Qt6::Widgets Qt6::PrintSupport Qt6::Network)
            message(STATUS "Using Qt6 without Charts (install qt-charts for visual graphs)")
        endif()
    else()
        # Fallback to Qt5
        find_package(Qt5 COMPONENTS Widgets PrintSupport Network QUIET)
        if(Qt5_FOUND)
            find_package(Qt5 COMPONENTS Charts QUIET)
            if(Qt5Charts_FOUND)
                set(QT_LIB Qt5::Widgets Qt5::PrintSupport Qt5::Network Qt5::Charts)
                set(USE_QT_CHARTS TRUE)
                message(STATUS "Using Qt5 with Charts")
            else()
                set(QT_LIB Qt5::Widgets Qt5::PrintSupport Qt5::Network)
                message(STATUS "Using Qt5 without Charts")
            endif()
        else()
            message(FATAL_ERROR "Qt5 or Qt6 not found. Please install Qt.")
        endif()
    endif()

    add_executable(CalorieCalcGUI MACOSX_BUNDLE
        gui/main_gui.cpp
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
    )
    
    if(SQLite3_FOUND)
        target_sources(CalorieCalcGUI PRIVATE SqliteSaveStrategy.cpp)
        target_link_libraries(CalorieCalcGUI PRIVATE SQLite::SQLite3)
        target_include_directories(CalorieCalcGUI PRIVATE ${SQLite3_INCLUDE_DIRS})
    endif()
    
    # REST API server (optional)
    option(BUILD_API_SERVER "Build REST API server" OFF)
    if(BUILD_API_SERVER)
        add_executable(CalorieCalcAPI
            api/main_api.cpp
            api/rest_server.cpp
            Food.cpp
            FoodDatabase.cpp
            SavedMeal.cpp
            Diary.cpp
            JsonSaveStrategy.cpp
        )
        target_link_libraries(CalorieCalcAPI PRIVATE ${QT_LIB})
        target_include_directories(CalorieCalcAPI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    
    target_link_libraries(CalorieCalcGUI PRIVATE ${QT_LIB})
    target_include_directories(CalorieCalcGUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    
    # Add QT_CHARTS_LIB definition if Charts is available
    if(USE_QT_CHARTS)
        target_compile_definitions(CalorieCalcGUI PRIVATE QT_CHARTS_LIB)
    endif()
    
    # i18n: Qt translations (optional, requires lrelease)
    if(QT_VERSION_MAJOR VERSION_GREATER_EQUAL 6)
        find_program(LRELEASE_EXECUTABLE lrelease HINTS ${Qt6_DIR}/../../../bin)
        if(LRELEASE_EXECUTABLE)
            qt_add_translations(CalorieCalcGUI TS_FILES translations/caloriecounter_uk.ts)
        endif()
    else()
        find_program(LRELEASE_EXECUTABLE lrelease HINTS ${Qt5_DIR}/../../../bin)
        if(LRELEASE_EXECUTABLE)
            set(TS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/translations/caloriecounter_uk.ts)
            set(QM_FILE ${CMAKE_CURRENT_BINARY_DIR}/caloriecounter_uk.qm)
            add_custom_command(OUTPUT ${QM_FILE}
                COMMAND ${LRELEASE_EXECUTABLE} ${TS_FILE} -qm ${QM_FILE}
                DEPENDS ${TS_FILE}
                COMMENT "Generating translation file")
            add_custom_target(translations ALL DEPENDS ${QM_FILE})
        endif()
    endif()

    # macOS bundle settings
    if(APPLE)
        set_source_files_properties(mac/Info.plist PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        set_target_properties(CalorieCalcGUI PROPERTIES
            OUTPUT_NAME "calorie counter"
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist"
        )
    endif()
endif()
