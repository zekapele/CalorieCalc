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
#include <QGroupBox>
#include <QDate>
#include <QString>
#include <QDir>
#include <QProgressBar>
#include <QTimeEdit>
#include "FoodDatabase.h"
#include "Diary.h"
#include "JsonSaveStrategy.h"
#include "TrainingDiary.h"
#include "TrainingJsonSaveStrategy.h"
#include "TrainingPlanGenerator.h"
#include "OfflineAssistant.h"

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
    void onToggleTheme();
    void onExportCSV();
    void onImportCSV();
    void onExportPDF();
    void onShowCharts();

    // Fitness/training slots
    void onAddTraining();
    void onRemoveTraining();
    void onGenerateTrainingPlan7Days();
    void onAskOfflineAssistant();
    void onAskOnlineAssistantPlaceholder();
    void onMarkTrainingCompleted();
    void onDuplicateTrainingTomorrow();
    void onShowDailyTip();

private:
    void setupUi();
    void applyDarkTheme();
    void applyLightTheme();
    void applyTheme(bool dark);
    void refreshDiary();
    void refreshStats();
    void refreshTraining();
    void removeFromSelectedTab();

    // Per-day/profile helpers
    QString currentProfile() const;
    void ensureStorageDirs();
    QString diaryFilePath(const QString& profile, const QDate& date) const;
    void loadDiaryFor(const QString& profile, const QDate& date);
    void saveDiaryFor(const QString& profile, const QDate& date);

    QString trainingFilePath(const QString& profile, const QDate& date) const;
    void loadTrainingFor(const QString& profile, const QDate& date);
    void saveTrainingFor(const QString& profile, const QDate& date);
    int calculateConsistencyStreak();

    // Data
    FoodDatabase foodDb_;
    Diary diary_;
    JsonSaveStrategy jsonSaver_;
    TrainingDiary trainingDiary_;
    TrainingJsonSaveStrategy trainingSaver_;

    // Widgets
    // Top controls
    QComboBox* profileCombo_{}; // "Без профілю" + custom names
    QCalendarWidget* calendar_{};

    QLineEdit* searchEdit_{};
    QListWidget* resultsList_{};
    QComboBox* categoryFilter_{};
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

    // Training (fitness)
    QGroupBox* trainingBox_{};
    QComboBox* trainingGoalCombo_{};
    QComboBox* trainingTypeCombo_{};
    QTimeEdit* trainingTimeEdit_{};
    QSpinBox* trainingDurationSpin_{};
    QComboBox* trainingStatusCombo_{};
    QLineEdit* trainingNotesEdit_{};
    QPushButton* addTrainingBtn_{};
    QPushButton* removeTrainingBtn_{};
    QListWidget* trainingsList_{};
    QLabel* trainingSummaryLabel_{};
    QPushButton* generatePlanBtn_{};
    QPushButton* assistantBtn_{};
    QPushButton* markCompletedBtn_{};
    QPushButton* duplicateTomorrowBtn_{};
    QPushButton* dailyTipBtn_{};
    QLabel* fitnessKpiLabel_{};
    QLabel* tipLabel_{};

    // Theme and export
    QPushButton* themeToggleBtn_{};
    QPushButton* exportCSVBtn_{};
    QPushButton* importCSVBtn_{};
    QPushButton* exportPDFBtn_{};
    QPushButton* chartsBtn_{};
    bool isDarkTheme_{true};

    // In-memory storage: profile -> (date -> diary snapshot)
    QMap<QString, QMap<QDate, Diary>> diaries_;
    QMap<QString, QMap<QDate, TrainingDiary>> trainings_;
    QDate selectedDate_;
};

