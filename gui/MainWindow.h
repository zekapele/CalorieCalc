#pragma once
#include <vector>
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
    explicit MainWindow(const QString& loggedInLogin, QWidget* parent = nullptr);

private slots:
    void onSearch();
    void onAddFood();
    void onRemoveMeal();
    void onSetGoal(int value);
    void onSetProteinGoal(int value);
    void onSetCarbGoal(int value);
    void onSetFatGoal(int value);
    void onAddCustomFood();
    void onSaveTemplate();
    void onApplyTemplate();
    void onDateChanged(const QDate& date);
    void onProfileChanged(int index);
    void onAddWater();
    void onSetWaterGoal(int value);
    void onSetWeight(double value);
    void onOpenProfileDialog();
    void onShowWeeklyReport();
    void onToggleTheme();
    void onExportCSV();
    void onImportCSV();
    void onExportPDF();
    void onExportTrainingCSV();
    void onExportTrainingPDF();
    void onShowCharts();
    void onShowProgressOverview();
    void onShowProductAbout();
    void onShowProductVisionAbout();
    void onShowUrFrNfrSummary();
    void onNfrSelfCheck();
    void onParallelComputeBenchmark();
    void onCopyYesterdayMeals();
    void onRepeatLastMeal();
    void onWaterQuick250();
    void onWaterQuick500();
    void onCopyDaySummaryToClipboard();
    void onResetMacroGoalsToDefaults();

    // Fitness/training slots
    void onAddTraining();
    void onRemoveTraining();
    void onEditSelectedTraining();
    void onGenerateTrainingPlan7Days();
    void onAskOfflineAssistant();
    void onMarkTrainingCompleted();
    void onDuplicateTrainingTomorrow();
    void onShowDailyTip();
    void onShowTrainingWeeklyReport();
    void onStartWorkout();
    void onQuickStartWorkout();
    void onCompleteWorkout();

private:
    void setupUi();
    void applyDarkTheme();
    void applyLightTheme();
    void applyTheme(bool dark);
    void updateAppChrome();
    void updateAssistantModeIndicator();
    void refreshDiary();
    void refreshStats();
    void refreshTraining();
    void removeFromSelectedTab();

    // Per-day/profile helpers
    QString userDataRoot() const;
    QString currentProfile() const;
    /** Папка на диску (guest для «Основний»/логін за замовчуванням/«Без профілю»). */
    QString profileStorageDir(const QString& profile) const;
    void ensureStorageDirs();
    QString diaryFilePath(const QString& profile, const QDate& date) const;
    void loadDiaryFor(const QString& profile, const QDate& date);
    void saveDiaryFor(const QString& profile, const QDate& date);

    QString trainingFilePath(const QString& profile, const QDate& date) const;
    void loadTrainingFor(const QString& profile, const QDate& date);
    void saveTrainingFor(const QString& profile, const QDate& date);
    int calculateConsistencyStreak() const;
    void loadProfileMeta(const QString& profile);
    void saveProfileMeta(const QString& profile) const;
    double estimateCalorieGoalFromProfile() const;
    void appendAssistantFeedback(const QString& query,
                                 const QString& response,
                                 const QString& verdict,
                                 const QString& comment);
    int countCompletedTrainingSessionsLast7Days() const;
    int trainingAdaptationVolume() const;
    TrainingPreferences buildTrainingPreferences() const;

    /** Паралельне завантаження щоденників (індекс у векторі = індекс у days). */
    QVector<Diary> loadDiariesForDates(const QVector<QDate>& days) const;

    // Data
    FoodDatabase foodDb_;
    Diary diary_;
    JsonSaveStrategy jsonSaver_;
    TrainingDiary trainingDiary_;
    TrainingJsonSaveStrategy trainingSaver_;

    // Widgets
    // Top controls
    QComboBox* profileCombo_{}; // лише логін (один пункт)
    QPushButton* profileSettingsBtn_{};
    QCalendarWidget* calendar_{};
    QSpinBox* ageSpin_{};           // years
    QDoubleSpinBox* heightSpin_{};  // cm
    QComboBox* activityCombo_{};    // activity level

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
    QPushButton* copyYesterdayBtn_{};
    QPushButton* repeatLastMealBtn_{};
    QMap<QString, QVector<SavedMeal>> templatesPerMealType_;

    // Stats and goals
    QSpinBox* goalSpin_{};
    QSpinBox* proteinGoalSpin_{};
    QSpinBox* carbGoalSpin_{};
    QSpinBox* fatGoalSpin_{};
    QLabel* caloriesLabel_{};
    QLabel* macrosLabel_{};
    QProgressBar* caloriesProgress_{};
    QProgressBar* macroProteinProgress_{};
    QProgressBar* macroCarbProgress_{};
    QProgressBar* macroFatProgress_{};

    // Water tracking
    QSpinBox* waterAddSpin_{}; // amount to add
    QPushButton* waterAddBtn_{};
    QPushButton* waterQuick250Btn_{};
    QPushButton* waterQuick500Btn_{};
    QSpinBox* waterGoalSpin_{};
    QProgressBar* waterProgress_{};

    // Weight
    QDoubleSpinBox* weightSpin_{};
    QLabel* bmiLabel_{};

    // Weekly report
    QPushButton* weeklyReportBtn_{};
    QPushButton* copyDaySummaryBtn_{};
    QPushButton* resetMacroDefaultsBtn_{};

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
    QPushButton* editTrainingBtn_{};
    QListWidget* trainingsList_{};
    QLabel* trainingSummaryLabel_{};
    QPushButton* generatePlanBtn_{};
    QPushButton* assistantBtn_{};
    QPushButton* markCompletedBtn_{};
    QPushButton* duplicateTomorrowBtn_{};
    QPushButton* dailyTipBtn_{};
    QPushButton* trainingWeeklyReportBtn_{};
    QPushButton* exportTrainingCSVBtn_{};
    QPushButton* exportTrainingPdfBtn_{};
    QLabel* assistantModeBadge_{};
    QPushButton* progressOverviewBtn_{};
    QPushButton* startWorkoutBtn_{};
    QPushButton* quickStartWorkoutBtn_{};
    QPushButton* completeWorkoutBtn_{};
    QLabel* headerDateLabel_{};
    bool workoutSessionActive_{false};
    std::vector<TrainingSession> activeWorkoutPlan_;
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

    QString loggedInLogin_;
    QString userDirSlug_; // data/<userDirSlug>/guest/...
};

