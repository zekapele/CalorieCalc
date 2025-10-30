cmake_minimum_required(VERSION 3.10)
project(CalorieCalc)

set(CMAKE_CXX_STANDARD 20)

add_executable(CalorieCalc 
    main.cpp
    Food.cpp
    FoodDatabase.cpp
    SavedMeal.cpp
    Diary.cpp
    JsonSaveStrategy.cpp
    TextSaveStrategy.cpp
    StatisticsCalculator.cpp
)

# GUI target (Qt Widgets)
option(BUILD_GUI "Build Qt GUI" ON)
if(BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    find_package(Qt6 COMPONENTS Widgets QUIET)
    if(NOT Qt6_FOUND)
        find_package(Qt5 COMPONENTS Widgets REQUIRED)
        set(QT_LIB Qt5::Widgets)
    else()
        set(QT_LIB Qt6::Widgets)
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
    )
    target_link_libraries(CalorieCalcGUI PRIVATE ${QT_LIB})
    target_include_directories(CalorieCalcGUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

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
