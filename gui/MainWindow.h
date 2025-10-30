#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QDoubleSpinBox>
#include <QMap>
#include <QVector>
#include <QCalendarWidget>
#include <QDate>
#include <QString>
#include <QDir>
#include <QProgressBar>
#include "FoodDatabase.h"
#include "Diary.h"
#include "JsonSaveStrategy.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onSearch();
    void onAddFood();
    void onRemoveMeal();
    void onSetGoal(int value);
    void onAddCustomFood();
    void onSaveTemplate();
    void onApplyTemplate();
    void onDateChanged(const QDate& date);
    void onProfileChanged(int index);
    void onAddWater();
    void onSetWaterGoal(int value);
    void onSetWeight(double value);
    void onShowWeeklyReport();

private:
    void setupUi();
    void applyBlueWhiteTheme();
    void refreshDiary();
    void refreshStats();
    void removeFromSelectedTab();

    // Per-day/profile helpers
    QString currentProfile() const;
    void ensureStorageDirs();
    QString diaryFilePath(const QString& profile, const QDate& date) const;
    void loadDiaryFor(const QString& profile, const QDate& date);
    void saveDiaryFor(const QString& profile, const QDate& date);

    // Data
    FoodDatabase foodDb_;
    Diary diary_;
    JsonSaveStrategy jsonSaver_;

    // Widgets
    // Top controls
    QComboBox* profileCombo_{}; // "Без профілю" + custom names
    QCalendarWidget* calendar_{};

    QLineEdit* searchEdit_{};
    QListWidget* resultsList_{};
    QSpinBox* amountSpin_{};
    QComboBox* mealTypeCombo_{};
    QPushButton* addButton_{};

    // Custom food section
    QLineEdit* customNameEdit_{};
    QDoubleSpinBox* customCaloriesEdit_{};
    QDoubleSpinBox* customCarbsEdit_{};
    QDoubleSpinBox* customProteinEdit_{};
    QDoubleSpinBox* customFatEdit_{};
    QPushButton* customAddButton_{};

    QTabWidget* diaryTabs_{};
    QListWidget* breakfastList_{};
    QListWidget* lunchList_{};
    QListWidget* dinnerList_{};
    QListWidget* snackList_{};
    QPushButton* removeButton_{};

    // Templates controls
    QPushButton* saveTemplateBtn_{};
    QPushButton* applyTemplateBtn_{};
    QMap<QString, QVector<SavedMeal>> templatesPerMealType_;

    // Stats and goals
    QSpinBox* goalSpin_{};
    QLabel* caloriesLabel_{};
    QLabel* macrosLabel_{};
    QProgressBar* caloriesProgress_{};

    // Water tracking
    QSpinBox* waterAddSpin_{}; // amount to add
    QPushButton* waterAddBtn_{};
    QSpinBox* waterGoalSpin_{};
    QProgressBar* waterProgress_{};

    // Weight
    QDoubleSpinBox* weightSpin_{};

    // Weekly report
    QPushButton* weeklyReportBtn_{};

    // In-memory storage: profile -> (date -> diary snapshot)
    QMap<QString, QMap<QDate, Diary>> diaries_;
    QDate selectedDate_;
};

