#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QScrollArea>
#include <QGroupBox>
#include <QPalette>
#include <QApplication>
#include <QStyle>
#include <QFont>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QStringConverter>
#include <QTime>
#include <QPlainTextEdit>
#include <QPrinter>
#include <QPainter>
#include <QPageSize>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <algorithm>
#ifdef QT_CHARTS_LIB
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QChart>
#endif
#include "../CsvImportExport.h"
#include "AuthStore.h"
#include "ProfileDialog.h"
#include <map>
#include <cctype>
#include <QTabWidget>

MainWindow::MainWindow(const QString& loggedInLogin, QWidget* parent)
    : QMainWindow(parent),
      loggedInLogin_(loggedInLogin.trimmed()),
      userDirSlug_(AuthStore::storageFolderForLogin(loggedInLogin_)) {
    setupUi();
}

void MainWindow::setupUi() {
    auto* appTabs = new QTabWidget(this);
    auto* caloriesPage = new QWidget(appTabs);
    auto* fitnessPage = new QWidget(appTabs);
    auto* rootLayout = new QHBoxLayout(caloriesPage);

    // Left column (controls)
    auto* leftColumn = new QVBoxLayout();

    // Top controls: profile + calendar + basic metrics
    auto* topBox = new QGroupBox(tr("Профіль і календар"), caloriesPage);
    auto* topLayout = new QVBoxLayout(topBox);

    auto* profileRow = new QHBoxLayout();
    profileCombo_ = new QComboBox(topBox);
    profileCombo_->setEditable(false);
    {
        const QString only = loggedInLogin_.isEmpty() ? QStringLiteral("user") : loggedInLogin_;
        profileCombo_->addItem(only);
    }
    profileRow->addWidget(new QLabel(tr("Профіль:"), topBox));
    profileRow->addWidget(profileCombo_);
    profileSettingsBtn_ = new QPushButton(tr("Профіль"), topBox);
    profileRow->addWidget(profileSettingsBtn_);

    auto* userAccountLabel = new QLabel(tr("Обліковий запис: %1").arg(loggedInLogin_), topBox);
    userAccountLabel->setWordWrap(true);

    auto* metricsRow = new QHBoxLayout();
    ageSpin_ = new QSpinBox(topBox);
    ageSpin_->setRange(10, 100);
    ageSpin_->setValue(30);
    ageSpin_->setSuffix(tr(" р."));
    heightSpin_ = new QDoubleSpinBox(topBox);
    heightSpin_->setRange(120.0, 230.0);
    heightSpin_->setDecimals(1);
    heightSpin_->setValue(175.0);
    heightSpin_->setSuffix(tr(" см"));
    activityCombo_ = new QComboBox(topBox);
    activityCombo_->addItems({tr("Сидячий"), tr("Помірний"), tr("Активний")});
    activityCombo_->setCurrentIndex(1); // помірний
    metricsRow->addWidget(new QLabel(tr("Вік:"), topBox));
    metricsRow->addWidget(ageSpin_);
    metricsRow->addWidget(new QLabel(tr("Зріст:"), topBox));
    metricsRow->addWidget(heightSpin_);
    metricsRow->addWidget(new QLabel(tr("Активність:"), topBox));
    metricsRow->addWidget(activityCombo_);

    // Автозбереження метаданих профілю при зміні
    connect(ageSpin_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int){
        saveProfileMeta(currentProfile());
    });
    connect(heightSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double){
        saveProfileMeta(currentProfile());
    });
    connect(activityCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int){
        saveProfileMeta(currentProfile());
    });

    calendar_ = new QCalendarWidget(topBox);
    calendar_->setGridVisible(true);
    selectedDate_ = QDate::currentDate();
    calendar_->setSelectedDate(selectedDate_);

    topLayout->addLayout(profileRow);
    topLayout->addWidget(userAccountLabel);
    topLayout->addLayout(metricsRow);
    topLayout->addWidget(calendar_);
    topBox->setLayout(topLayout);

    // Search and add box
    auto* leftBox = new QGroupBox(tr("Пошук продуктів"), caloriesPage);
    auto* leftLayout = new QVBoxLayout(leftBox);

    searchEdit_ = new QLineEdit(leftBox);
    searchEdit_->setPlaceholderText(tr("Введіть назву продукту..."));
    auto* searchBtn = new QPushButton(tr("Пошук"), leftBox);
    searchBtn->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
    
    // Category filter
    categoryFilter_ = new QComboBox(leftBox);
    categoryFilter_->addItem(tr("Всі категорії"));
    auto categories = foodDb_.getAllCategories();
    for (const auto& cat : categories) {
        categoryFilter_->addItem(QString::fromStdString(cat));
    }
    
    resultsList_ = new QListWidget(leftBox);
    resultsList_->setAlternatingRowColors(true);

    auto* amountRow = new QHBoxLayout();
    amountSpin_ = new QSpinBox(leftBox);
    amountSpin_->setRange(1, 2000);
    amountSpin_->setValue(100);
    amountSpin_->setSuffix(" г");
    mealTypeCombo_ = new QComboBox(leftBox);
    mealTypeCombo_->addItems({tr("Сніданок"), tr("Обід"), tr("Вечеря"), tr("Перекус")});
    addButton_ = new QPushButton(tr("Додати"), leftBox);
    addButton_->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    amountRow->addWidget(amountSpin_);
    amountRow->addWidget(mealTypeCombo_);
    amountRow->addWidget(addButton_);

    // Custom food section
    auto* customBox = new QGroupBox(tr("Додати свій продукт"), caloriesPage);
    auto* customForm = new QFormLayout(customBox);

    customNameEdit_ = new QLineEdit(customBox);
    customNameEdit_->setPlaceholderText(tr("Назва (напр. Мій батончик)"));

    customCaloriesEdit_ = new QDoubleSpinBox(customBox);
    customCaloriesEdit_->setRange(0.0, 2000.0);
    customCaloriesEdit_->setDecimals(1);
    customCaloriesEdit_->setSuffix(" ккал/100г");

    customCarbsEdit_ = new QDoubleSpinBox(customBox);
    customCarbsEdit_->setRange(0.0, 200.0);
    customCarbsEdit_->setDecimals(1);
    customCarbsEdit_->setSuffix(" г/100г");

    customProteinEdit_ = new QDoubleSpinBox(customBox);
    customProteinEdit_->setRange(0.0, 200.0);
    customProteinEdit_->setDecimals(1);
    customProteinEdit_->setSuffix(" г/100г");

    customFatEdit_ = new QDoubleSpinBox(customBox);
    customFatEdit_->setRange(0.0, 200.0);
    customFatEdit_->setDecimals(1);
    customFatEdit_->setSuffix(" г/100г");

    customAddButton_ = new QPushButton(tr("Додати продукт до бази"), customBox);

    customForm->addRow(tr("Назва"), customNameEdit_);
    customForm->addRow(tr("Калорії"), customCaloriesEdit_);
    customForm->addRow(tr("Вуглеводи"), customCarbsEdit_);
    customForm->addRow(tr("Білки"), customProteinEdit_);
    customForm->addRow(tr("Жири"), customFatEdit_);
    customForm->addRow(customAddButton_);

    leftLayout->setSpacing(8);
    leftLayout->addWidget(searchEdit_);
    auto* searchRow = new QHBoxLayout();
    searchRow->addWidget(searchBtn);
    searchRow->addWidget(new QLabel(tr("Категорія:"), leftBox));
    searchRow->addWidget(categoryFilter_);
    leftLayout->addLayout(searchRow);
    leftLayout->addWidget(resultsList_);
    leftLayout->addLayout(amountRow);
    leftLayout->addWidget(customBox);
    leftBox->setLayout(leftLayout);

    // Pack left column
    leftColumn->addWidget(topBox);
    leftColumn->addWidget(leftBox);

    // Right panel: diary and stats (with tabs for meal sections)
    auto* rightBox = new QGroupBox(tr("Щоденник"), caloriesPage);
    auto* rightLayout = new QVBoxLayout(rightBox);

    diaryTabs_ = new QTabWidget(rightBox);
    breakfastList_ = new QListWidget(diaryTabs_);
    lunchList_ = new QListWidget(diaryTabs_);
    dinnerList_ = new QListWidget(diaryTabs_);
    snackList_ = new QListWidget(diaryTabs_);

    breakfastList_->setAlternatingRowColors(true);
    lunchList_->setAlternatingRowColors(true);
    dinnerList_->setAlternatingRowColors(true);
    snackList_->setAlternatingRowColors(true);

    diaryTabs_->addTab(breakfastList_, tr("Сніданок"));
    diaryTabs_->addTab(lunchList_, tr("Обід"));
    diaryTabs_->addTab(dinnerList_, tr("Вечеря"));
    diaryTabs_->addTab(snackList_, tr("Перекус"));

    // Template controls
    auto* tmplRow = new QHBoxLayout();
    saveTemplateBtn_ = new QPushButton(tr("Зберегти як шаблон"), rightBox);
    applyTemplateBtn_ = new QPushButton(tr("Додати шаблон"), rightBox);
    tmplRow->addWidget(saveTemplateBtn_);
    tmplRow->addWidget(applyTemplateBtn_);

    removeButton_ = new QPushButton(tr("Видалити вибране"), rightBox);
    removeButton_->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));

    // Goals and stats
    auto* goalsBox = new QGroupBox(tr("Цілі та статистика"), rightBox);
    auto* goalsLayout = new QVBoxLayout(goalsBox);

    auto* calRow = new QHBoxLayout();
    goalSpin_ = new QSpinBox(goalsBox);
    goalSpin_->setRange(500, 6000);
    goalSpin_->setValue(2000);
    goalSpin_->setSuffix(" ккал");
    calRow->addWidget(new QLabel(tr("Ціль калорій:"), goalsBox));
    calRow->addWidget(goalSpin_);

    caloriesProgress_ = new QProgressBar(goalsBox);
    caloriesProgress_->setRange(0, 100);

    auto* waterRow = new QHBoxLayout();
    waterAddSpin_ = new QSpinBox(goalsBox);
    waterAddSpin_->setRange(50, 1000);
    waterAddSpin_->setSingleStep(50);
    waterAddSpin_->setValue(250);
    waterAddSpin_->setSuffix(" мл");
    waterAddBtn_ = new QPushButton(tr("Випити"), goalsBox);
    waterGoalSpin_ = new QSpinBox(goalsBox);
    waterGoalSpin_->setRange(0, 10000);
    waterGoalSpin_->setSingleStep(100);
    waterGoalSpin_->setValue(2000);
    waterGoalSpin_->setSuffix(" мл");
    waterRow->addWidget(new QLabel(tr("Вода:"), goalsBox));
    waterRow->addWidget(waterAddSpin_);
    waterRow->addWidget(waterAddBtn_);
    waterRow->addSpacing(8);
    waterRow->addWidget(new QLabel(tr("Ціль води:"), goalsBox));
    waterRow->addWidget(waterGoalSpin_);

    waterProgress_ = new QProgressBar(goalsBox);
    waterProgress_->setRange(0, 100);

    auto* weightRow = new QHBoxLayout();
    weightSpin_ = new QDoubleSpinBox(goalsBox);
    weightSpin_->setRange(0.0, 400.0);
    weightSpin_->setDecimals(1);
    weightSpin_->setSuffix(" кг");
    weightRow->addWidget(new QLabel(tr("Вага:"), goalsBox));
    weightRow->addWidget(weightSpin_);

    caloriesLabel_ = new QLabel(goalsBox);
    macrosLabel_ = new QLabel(goalsBox);

    goalsLayout->addLayout(calRow);
    goalsLayout->addWidget(caloriesProgress_);
    goalsLayout->addLayout(waterRow);
    goalsLayout->addWidget(waterProgress_);
    goalsLayout->addLayout(weightRow);
    goalsLayout->addWidget(caloriesLabel_);
    goalsLayout->addWidget(macrosLabel_);

    // Weekly report and actions
    weeklyReportBtn_ = new QPushButton(tr("Звіт за 7 днів"), rightBox);
    
    auto* actionsRow = new QHBoxLayout();
    themeToggleBtn_ = new QPushButton(tr("☀ Світла тема"), rightBox);
    exportCSVBtn_ = new QPushButton(tr("Експорт CSV"), rightBox);
    importCSVBtn_ = new QPushButton(tr("Імпорт CSV"), rightBox);
    exportPDFBtn_ = new QPushButton(tr("Експорт PDF"), rightBox);
    chartsBtn_ = new QPushButton(tr("Графіки"), rightBox);
    assistantBtn_ = new QPushButton(tr("Помічник"), fitnessPage);
    actionsRow->addWidget(themeToggleBtn_);
    actionsRow->addWidget(exportCSVBtn_);
    actionsRow->addWidget(importCSVBtn_);
    actionsRow->addWidget(exportPDFBtn_);
    actionsRow->addWidget(chartsBtn_);

    // Arrange right panel
    auto* goalsContainer = goalsBox;
    rightLayout->setSpacing(8);
    rightLayout->addWidget(diaryTabs_);
    rightLayout->addLayout(tmplRow);
    rightLayout->addWidget(removeButton_);
    rightLayout->addWidget(goalsContainer);

    // Training block (fitness)
    trainingBox_ = new QGroupBox(tr("Тренування"), fitnessPage);
    auto* trainingLayout = new QVBoxLayout(trainingBox_);

    auto* goalRow = new QHBoxLayout();
    trainingGoalCombo_ = new QComboBox(trainingBox_);
    trainingGoalCombo_->addItems({tr("Схуднення"), tr("Набір"), tr("Підтримка")});
    trainingGoalCombo_->setCurrentIndex(2); // maintenance

    generatePlanBtn_ = new QPushButton(tr("Автоплан (7 днів)"), trainingBox_);
    goalRow->addWidget(new QLabel(tr("Ціль:"), trainingBox_));
    goalRow->addWidget(trainingGoalCombo_);
    goalRow->addWidget(generatePlanBtn_);
    trainingLayout->addLayout(goalRow);

    auto* sessionRow = new QHBoxLayout();
    trainingTypeCombo_ = new QComboBox(trainingBox_);
    trainingTypeCombo_->addItems({tr("Сила"), tr("Кардіо"), tr("Мобільність"), tr("Відпочинок")});

    trainingTimeEdit_ = new QTimeEdit(trainingBox_);
    trainingTimeEdit_->setTime(QTime(7, 30));

    trainingDurationSpin_ = new QSpinBox(trainingBox_);
    trainingDurationSpin_->setRange(0, 180);
    trainingDurationSpin_->setValue(45);
    trainingDurationSpin_->setSuffix(" хв");

    trainingStatusCombo_ = new QComboBox(trainingBox_);
    trainingStatusCombo_->addItems({tr("План"), tr("Виконано")});

    sessionRow->addWidget(new QLabel(tr("Тип:"), trainingBox_));
    sessionRow->addWidget(trainingTypeCombo_);
    sessionRow->addWidget(new QLabel(tr("Час:"), trainingBox_));
    sessionRow->addWidget(trainingTimeEdit_);
    sessionRow->addWidget(new QLabel(tr("Тривалість:"), trainingBox_));
    sessionRow->addWidget(trainingDurationSpin_);
    sessionRow->addWidget(new QLabel(tr("Статус:"), trainingBox_));
    sessionRow->addWidget(trainingStatusCombo_);

    trainingLayout->addLayout(sessionRow);

    trainingNotesEdit_ = new QLineEdit(trainingBox_);
    trainingNotesEdit_->setPlaceholderText(tr("Нотатки (напр. вправи, підхід/повторення, прогрес...)"));
    trainingLayout->addWidget(trainingNotesEdit_);

    auto* trainingBtnRow = new QHBoxLayout();
    addTrainingBtn_ = new QPushButton(tr("Додати тренування"), trainingBox_);
    removeTrainingBtn_ = new QPushButton(tr("Видалити вибране"), trainingBox_);
    removeTrainingBtn_->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    trainingBtnRow->addWidget(addTrainingBtn_);
    trainingBtnRow->addWidget(removeTrainingBtn_);
    trainingLayout->addLayout(trainingBtnRow);

    trainingsList_ = new QListWidget(trainingBox_);
    trainingsList_->setAlternatingRowColors(true);
    trainingLayout->addWidget(trainingsList_);

    trainingSummaryLabel_ = new QLabel(trainingBox_);
    trainingLayout->addWidget(trainingSummaryLabel_);

    rightLayout->addWidget(weeklyReportBtn_);
    rightLayout->addLayout(actionsRow);
    rightBox->setLayout(rightLayout);

    // Layout (both columns are scrollable so full page remains accessible on smaller windows)
    auto* leftWidget = new QWidget(caloriesPage);
    leftWidget->setLayout(leftColumn);
    auto* leftScroll = new QScrollArea(caloriesPage);
    leftScroll->setWidgetResizable(true);
    leftScroll->setWidget(leftWidget);

    auto* rightScroll = new QScrollArea(caloriesPage);
    rightScroll->setWidgetResizable(true);
    rightScroll->setWidget(rightBox);

    rootLayout->setSpacing(12);
    rootLayout->addWidget(leftScroll, 1);
    rootLayout->addWidget(rightScroll, 1);
    auto* fitnessLayout = new QVBoxLayout(fitnessPage);

    auto* fitnessSummaryBox = new QGroupBox(tr("Фітнес-дашборд"), fitnessPage);
    auto* fitnessSummaryLayout = new QVBoxLayout(fitnessSummaryBox);
    fitnessKpiLabel_ = new QLabel(fitnessSummaryBox);
    fitnessKpiLabel_->setObjectName("fitnessKpiLabel");
    tipLabel_ = new QLabel(tr("Порада дня: натисніть \"Порада дня\""), fitnessSummaryBox);
    tipLabel_->setWordWrap(true);
    tipLabel_->setObjectName("tipLabel");
    dailyTipBtn_ = new QPushButton(tr("Порада дня"), fitnessSummaryBox);
    fitnessSummaryLayout->addWidget(fitnessKpiLabel_);
    fitnessSummaryLayout->addWidget(tipLabel_);
    fitnessSummaryLayout->addWidget(dailyTipBtn_);

    auto* trainingQuickActions = new QHBoxLayout();
    startWorkoutBtn_ = new QPushButton(tr("Start workout"), fitnessPage);
    quickStartWorkoutBtn_ = new QPushButton(tr("Quick start"), fitnessPage);
    markCompletedBtn_ = new QPushButton(tr("Позначити як виконано"), fitnessPage);
    duplicateTomorrowBtn_ = new QPushButton(tr("Дублювати на завтра"), fitnessPage);
    trainingWeeklyReportBtn_ = new QPushButton(tr("Звіт тренувань (7 днів)"), fitnessPage);
    trainingQuickActions->addWidget(startWorkoutBtn_);
    trainingQuickActions->addWidget(quickStartWorkoutBtn_);
    trainingQuickActions->addWidget(markCompletedBtn_);
    trainingQuickActions->addWidget(duplicateTomorrowBtn_);
    trainingQuickActions->addWidget(trainingWeeklyReportBtn_);

    fitnessLayout->addWidget(fitnessSummaryBox);
    fitnessLayout->addWidget(trainingBox_);
    fitnessLayout->addLayout(trainingQuickActions);
    fitnessLayout->addWidget(assistantBtn_);
    fitnessLayout->addStretch();

    appTabs->addTab(caloriesPage, tr("Калькулятор калорій"));
    appTabs->addTab(fitnessPage, tr("Фітнес"));
    setCentralWidget(appTabs);

    // Signals
    connect(searchBtn, &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(categoryFilter_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::onSearch);
    connect(addButton_, &QPushButton::clicked, this, &MainWindow::onAddFood);
    connect(removeButton_, &QPushButton::clicked, this, &MainWindow::onRemoveMeal);
    connect(goalSpin_, &QSpinBox::valueChanged, this, &MainWindow::onSetGoal);
    connect(customAddButton_, &QPushButton::clicked, this, &MainWindow::onAddCustomFood);
    connect(saveTemplateBtn_, &QPushButton::clicked, this, &MainWindow::onSaveTemplate);
    connect(applyTemplateBtn_, &QPushButton::clicked, this, &MainWindow::onApplyTemplate);
    connect(calendar_, &QCalendarWidget::clicked, this, &MainWindow::onDateChanged);
    connect(profileCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::onProfileChanged);
    connect(profileSettingsBtn_, &QPushButton::clicked, this, &MainWindow::onOpenProfileDialog);
    connect(waterAddBtn_, &QPushButton::clicked, this, &MainWindow::onAddWater);
    connect(waterGoalSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onSetWaterGoal);
    connect(weightSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &MainWindow::onSetWeight);
    connect(weeklyReportBtn_, &QPushButton::clicked, this, &MainWindow::onShowWeeklyReport);
    connect(themeToggleBtn_, &QPushButton::clicked, this, &MainWindow::onToggleTheme);
    connect(exportCSVBtn_, &QPushButton::clicked, this, &MainWindow::onExportCSV);
    connect(importCSVBtn_, &QPushButton::clicked, this, &MainWindow::onImportCSV);
    connect(exportPDFBtn_, &QPushButton::clicked, this, &MainWindow::onExportPDF);
    connect(chartsBtn_, &QPushButton::clicked, this, &MainWindow::onShowCharts);

    connect(addTrainingBtn_, &QPushButton::clicked, this, &MainWindow::onAddTraining);
    connect(removeTrainingBtn_, &QPushButton::clicked, this, &MainWindow::onRemoveTraining);
    connect(generatePlanBtn_, &QPushButton::clicked, this, &MainWindow::onGenerateTrainingPlan7Days);
    connect(assistantBtn_, &QPushButton::clicked, this, &MainWindow::onAskOfflineAssistant);
    connect(markCompletedBtn_, &QPushButton::clicked, this, &MainWindow::onMarkTrainingCompleted);
    connect(duplicateTomorrowBtn_, &QPushButton::clicked, this, &MainWindow::onDuplicateTrainingTomorrow);
    connect(dailyTipBtn_, &QPushButton::clicked, this, &MainWindow::onShowDailyTip);
    connect(trainingWeeklyReportBtn_, &QPushButton::clicked, this, &MainWindow::onShowTrainingWeeklyReport);
    connect(startWorkoutBtn_, &QPushButton::clicked, this, &MainWindow::onStartWorkout);
    connect(quickStartWorkoutBtn_, &QPushButton::clicked, this, &MainWindow::onQuickStartWorkout);

    // Init storage
    ensureStorageDirs();
    loadProfileMeta(currentProfile());
    loadDiaryFor(currentProfile(), selectedDate_);
    loadTrainingFor(currentProfile(), selectedDate_);
    // Initialize controls from diary
    goalSpin_->setValue(static_cast<int>(diary_.getCalorieGoal()));
    waterGoalSpin_->setValue(diary_.getWaterGoalMl());
    weightSpin_->setValue(diary_.getWeightKg());
    refreshDiary();
    refreshStats();
    refreshTraining();
    onShowDailyTip();
    
    // Apply initial theme
    applyLightTheme();
}

void MainWindow::applyDarkTheme() {
    applyTheme(true);
}

void MainWindow::applyLightTheme() {
    applyTheme(false);
}

void MainWindow::applyTheme(bool dark) {
    isDarkTheme_ = dark;
    themeToggleBtn_->setText(dark ? tr("☀ Світла тема") : tr("🌙 Темна тема"));
    
    if (dark) {
        // New dark palette: violet + mint accent
        const QColor bgMain(20, 16, 28);
        const QColor bgPanel(31, 24, 42);
        const QColor bgAlt(41, 32, 56);
        const QColor textMain(238, 235, 245);
        const QColor accent(139, 233, 193);
        const QColor border(67, 53, 88);

        QPalette pal = qApp->palette();
        pal.setColor(QPalette::Window, bgMain);
        pal.setColor(QPalette::Base, bgPanel);
        pal.setColor(QPalette::AlternateBase, bgAlt);
        pal.setColor(QPalette::Text, textMain);
        pal.setColor(QPalette::WindowText, textMain);
        pal.setColor(QPalette::Button, bgPanel);
        pal.setColor(QPalette::ButtonText, textMain);
        pal.setColor(QPalette::Highlight, accent);
        pal.setColor(QPalette::HighlightedText, QColor(20, 16, 28));
        qApp->setPalette(pal);

        qApp->setStyleSheet(
            "QMainWindow { background-color: #12121A; }"
            "QGroupBox { font-weight: 600; border: 1px solid #32324A; border-radius: 14px; margin-top: 14px; padding: 10px 10px 12px 10px; color: #F3F4FF; background: #1A1B27; }"
            "QGroupBox::title { padding: 0 10px; color: #7CE7C9; subcontrol-origin: margin; subcontrol-position: top left; }"
            "QPushButton { background: #2A2D42; color: #F3F4FF; padding: 9px 16px; border: 1px solid #3B3F5C; border-radius: 11px; font-weight: 600; }"
            "QPushButton:hover { background: #333754; border-color: #7CE7C9; }"
            "QPushButton:pressed { background: #23263A; }"
            "QLineEdit, QSpinBox, QComboBox, QDoubleSpinBox, QTimeEdit, QPlainTextEdit { padding: 8px 12px; border: 1px solid #3B3F5C; border-radius: 10px; background: #171925; color: #F3F4FF; selection-background-color: #7CE7C9; selection-color: #101218; }"
            "QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDoubleSpinBox:focus, QTimeEdit:focus, QPlainTextEdit:focus { border: 2px solid #7CE7C9; background: #1D2030; }"
            "QListWidget { background: #171925; border: 1px solid #3B3F5C; border-radius: 10px; color: #F3F4FF; }"
            "QListWidget::item { padding: 7px; border-radius: 6px; }"
            "QListWidget::item:selected { background: #7CE7C9; color: #101218; }"
            "QListWidget::item:hover { background: #2A2E45; }"
            "QTabBar::tab { background: #202338; color: #A9AFD1; padding: 10px 18px; border-top-left-radius: 10px; border-top-right-radius: 10px; margin-right: 3px; border: 1px solid #343854; border-bottom: none; }"
            "QTabBar::tab:selected { background: #181B2B; color: #7CE7C9; border-bottom: 2px solid #7CE7C9; font-weight: 700; }"
            "QTabBar::tab:hover:!selected { background: #2A2E45; color: #E3E7FF; }"
            "QTabWidget::pane { border: 1px solid #343854; border-radius: 10px; top: -1px; background: #171925; }"
            "QLabel { color: #F0F2FF; }"
            "QCalendarWidget { background-color: #171925; color: #F0F2FF; border: 1px solid #343854; border-radius: 10px; }"
            "QCalendarWidget QAbstractItemView:enabled { selection-background-color: #7CE7C9; selection-color: #101218; background-color: #171925; }"
            "QCalendarWidget QHeaderView::section { background-color: #21253A; color: #F0F2FF; border: none; padding: 7px; }"
            "QProgressBar { border: 1px solid #343854; border-radius: 10px; background: #171925; color: #EAF2FF; text-align: center; height: 20px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #7CE7C9, stop:1 #69B9FF); border-radius: 10px; }"
            "QLabel#fitnessKpiLabel { font-size: 14px; font-weight: 700; color: #7CE7C9; padding: 8px; background: #21253A; border: 1px solid #343854; border-radius: 10px; }"
            "QLabel#tipLabel { color: #EAFDF7; background: #202738; border-left: 3px solid #7CE7C9; padding: 9px; border-radius: 10px; }"
        );
    } else {
        // New light palette: lavender + mint accent
        QPalette pal = qApp->palette();
        pal.setColor(QPalette::Window, QColor(248, 246, 255));
        pal.setColor(QPalette::Base, QColor(255, 255, 255));
        pal.setColor(QPalette::AlternateBase, QColor(245, 242, 255));
        pal.setColor(QPalette::Text, QColor(41, 31, 59));
        pal.setColor(QPalette::WindowText, QColor(41, 31, 59));
        pal.setColor(QPalette::Button, QColor(244, 239, 255));
        pal.setColor(QPalette::ButtonText, QColor(41, 31, 59));
        pal.setColor(QPalette::Highlight, QColor(52, 211, 153));
        pal.setColor(QPalette::HighlightedText, Qt::white);
        qApp->setPalette(pal);

        qApp->setStyleSheet(
            "QMainWindow { background-color: #F3F5FB; }"
            "QGroupBox { font-weight: 600; border: 1px solid #D7DDEB; border-radius: 14px; margin-top: 14px; padding: 10px 10px 12px 10px; color: #1F2638; background: #FFFFFF; }"
            "QGroupBox::title { padding: 0 10px; color: #4A6CF7; subcontrol-origin: margin; subcontrol-position: top left; }"
            "QPushButton { background: #EEF3FF; color: #1F2638; padding: 9px 16px; border: 1px solid #CDD8F6; border-radius: 11px; font-weight: 600; }"
            "QPushButton:hover { background: #E4ECFF; border-color: #4A6CF7; color: #193A91; }"
            "QPushButton:pressed { background: #D9E5FF; }"
            "QLineEdit, QSpinBox, QComboBox, QDoubleSpinBox, QTimeEdit, QPlainTextEdit { padding: 8px 12px; border: 1px solid #D1D9EA; border-radius: 10px; background: #FFFFFF; color: #1F2638; selection-background-color: #4A6CF7; selection-color: #FFFFFF; }"
            "QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDoubleSpinBox:focus, QTimeEdit:focus, QPlainTextEdit:focus { border: 2px solid #4A6CF7; background: #FBFCFF; }"
            "QListWidget { background: #FFFFFF; border: 1px solid #D7DDEB; border-radius: 10px; color: #1F2638; }"
            "QListWidget::item { padding: 7px; border-radius: 6px; }"
            "QListWidget::item:selected { background: #4A6CF7; color: #FFFFFF; }"
            "QListWidget::item:hover { background: #F2F6FF; }"
            "QTabBar::tab { background: #EAF0FF; color: #5A6788; padding: 10px 18px; border-top-left-radius: 10px; border-top-right-radius: 10px; margin-right: 3px; border: 1px solid #D4DCEF; border-bottom: none; }"
            "QTabBar::tab:selected { background: #FFFFFF; color: #2746C6; border-bottom: 2px solid #4A6CF7; font-weight: 700; }"
            "QTabBar::tab:hover:!selected { background: #DFE9FF; color: #3A4A76; }"
            "QTabWidget::pane { border: 1px solid #D4DCEF; border-radius: 10px; top: -1px; background: #FFFFFF; }"
            "QLabel { color: #1F2638; }"
            "QCalendarWidget { background-color: #FFFFFF; color: #1F2638; border: 1px solid #D4DCEF; border-radius: 10px; }"
            "QCalendarWidget QAbstractItemView:enabled { selection-background-color: #4A6CF7; selection-color: #FFFFFF; background-color: #FFFFFF; }"
            "QCalendarWidget QHeaderView::section { background-color: #EEF3FF; color: #1F2638; border: none; padding: 7px; }"
            "QProgressBar { border: 1px solid #D4DCEF; border-radius: 10px; background: #EEF2FA; color: #1F2638; text-align: center; height: 20px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4A6CF7, stop:1 #45D3B1); border-radius: 10px; }"
            "QLabel#fitnessKpiLabel { font-size: 14px; font-weight: 700; color: #2746C6; padding: 8px; background: #EEF3FF; border: 1px solid #D4DCEF; border-radius: 10px; }"
            "QLabel#tipLabel { color: #1B4B44; background: #EAFBF6; border-left: 3px solid #45D3B1; padding: 9px; border-radius: 10px; }"
        );
    }
}

QString MainWindow::currentProfile() const {
    if (profileCombo_ && !profileCombo_->currentText().trimmed().isEmpty())
        return profileCombo_->currentText().trimmed();
    return loggedInLogin_.isEmpty() ? QStringLiteral("user") : loggedInLogin_;
}

QString MainWindow::profileStorageDir(const QString& profile) const {
    const QString p = profile.trimmed();
    if (p == tr("Без профілю") || p == tr("Основний")) return QStringLiteral("guest");
    if (!loggedInLogin_.isEmpty() && p == loggedInLogin_) return QStringLiteral("guest");
    return p;
}

QString MainWindow::userDataRoot() const {
    return QDir(QDir::currentPath()).filePath(QStringLiteral("data/%1").arg(userDirSlug_));
}

void MainWindow::ensureStorageDirs() {
    QDir base(QDir::currentPath());
    if (!base.exists("data")) base.mkdir("data");
    const QString root = userDataRoot();
    if (!QDir(root).exists()) QDir().mkpath(root);
    QDir userDir(root);
    const QString profileDirName = profileStorageDir(currentProfile());
    if (!userDir.exists(profileDirName)) userDir.mkdir(profileDirName);
}

QString MainWindow::diaryFilePath(const QString& profile, const QDate& date) const {
    const QString profileDirName = profileStorageDir(profile);
    const QString fileName = date.toString("yyyy-MM-dd");
    return QDir(userDataRoot()).filePath(QStringLiteral("%1/%2").arg(profileDirName, fileName));
}

void MainWindow::loadDiaryFor(const QString& profile, const QDate& date) {
    // Load from memory if present
    if (diaries_.contains(profile) && diaries_[profile].contains(date)) {
        diary_ = diaries_[profile][date];
        return;
    }
    // Try disk JSON
    const QString path = diaryFilePath(profile, date);
    Diary tmp;
    if (jsonSaver_.load(tmp, path.toStdString())) {
        diary_ = tmp;
        diaries_[profile][date] = tmp;
        return;
    }
    // Fresh diary
    diary_ = Diary();
    diaries_[profile][date] = diary_;
}

void MainWindow::saveDiaryFor(const QString& profile, const QDate& date) {
    diaries_[profile][date] = diary_;
    ensureStorageDirs();
    const QString path = diaryFilePath(profile, date);
    jsonSaver_.save(diary_, path.toStdString());
}

QString MainWindow::trainingFilePath(const QString& profile, const QDate& date) const {
    const QString profileDirName = profileStorageDir(profile);
    const QString fileName = date.toString("yyyy-MM-dd");
    return QDir(userDataRoot()).filePath(QStringLiteral("%1/%2").arg(profileDirName, fileName));
}

void MainWindow::loadProfileMeta(const QString& profile) {
    ensureStorageDirs();
    const QString profileDirName = profileStorageDir(profile);
    const QString filePath = QDir(userDataRoot()).filePath(QStringLiteral("%1/profile.json").arg(profileDirName));

    // Defaults
    ageSpin_->setValue(30);
    heightSpin_->setValue(175.0);
    activityCombo_->setCurrentIndex(1); // помірний

    QFile f(filePath);
    if (!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Use defaults and, якщо goal ще дефолтний, спробуємо оновити ціль.
        if (diary_.getCalorieGoal() == 2000.0) {
            double est = estimateCalorieGoalFromProfile();
            diary_.setCalorieGoal(est);
            goalSpin_->setValue(static_cast<int>(est));
        }
        return;
    }

    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.startsWith(QLatin1String("\"age\""))) {
            const int idx = line.indexOf(':');
            if (idx >= 0) {
                bool ok = false;
                int age = line.mid(idx + 1).remove(',').trimmed().toInt(&ok);
                if (ok) ageSpin_->setValue(age);
            }
        } else if (line.startsWith(QLatin1String("\"height_cm\""))) {
            const int idx = line.indexOf(':');
            if (idx >= 0) {
                bool ok = false;
                double h = line.mid(idx + 1).remove(',').trimmed().toDouble(&ok);
                if (ok) heightSpin_->setValue(h);
            }
        } else if (line.startsWith(QLatin1String("\"activity\""))) {
            const int firstQuote = line.indexOf('"', line.indexOf(':'));
            const int lastQuote = line.lastIndexOf('"');
            if (firstQuote >= 0 && lastQuote > firstQuote) {
                QString act = line.mid(firstQuote + 1, lastQuote - firstQuote - 1);
                int idxAct = 1;
                if (act == QLatin1String("sedentary")) idxAct = 0;
                else if (act == QLatin1String("moderate")) idxAct = 1;
                else if (act == QLatin1String("active")) idxAct = 2;
                activityCombo_->setCurrentIndex(idxAct);
            }
        }
    }
    f.close();

    if (diary_.getCalorieGoal() == 2000.0) {
        double est = estimateCalorieGoalFromProfile();
        diary_.setCalorieGoal(est);
        goalSpin_->setValue(static_cast<int>(est));
    }
}

void MainWindow::saveProfileMeta(const QString& profile) const {
    if (!ageSpin_ || !heightSpin_ || !activityCombo_) return;

    const QString profileDirName = profileStorageDir(profile);
    QDir ud(userDataRoot());
    if (!ud.exists()) QDir().mkpath(userDataRoot());
    if (!ud.exists(profileDirName)) ud.mkdir(profileDirName);

    const QString filePath = ud.filePath(QStringLiteral("%1/profile.json").arg(profileDirName));
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out << "{\n";
    out << "  \"age\": " << ageSpin_->value() << ",\n";
    out << "  \"height_cm\": " << heightSpin_->value() << ",\n";

    QString actStr = "moderate";
    if (activityCombo_->currentIndex() == 0) actStr = "sedentary";
    else if (activityCombo_->currentIndex() == 1) actStr = "moderate";
    else if (activityCombo_->currentIndex() == 2) actStr = "active";

    out << "  \"activity\": \"" << actStr << "\"\n";
    out << "}\n";
    f.close();
}

double MainWindow::estimateCalorieGoalFromProfile() const {
    // Дуже спрощена формула: базовий метаболізм ~ 24 * вага, з поправкою на активність.
    double weight = weightSpin_ ? weightSpin_->value() : 0.0;
    if (weight <= 0.0) weight = 70.0; // якщо вага не вказана
    int age = ageSpin_ ? ageSpin_->value() : 30;

    double bmr = 24.0 * weight;
    // Невелика вікова корекція
    if (age > 40) bmr *= 0.95;
    if (age > 55) bmr *= 0.9;

    double factor = 1.2;
    if (activityCombo_) {
        if (activityCombo_->currentIndex() == 0) factor = 1.2;
        else if (activityCombo_->currentIndex() == 1) factor = 1.4;
        else factor = 1.6;
    }
    double tdee = bmr * factor;
    // Округлимо до найближчих 50 ккал
    double rounded = std::round(tdee / 50.0) * 50.0;
    return std::clamp(rounded, 1400.0, 4000.0);
}

void MainWindow::loadTrainingFor(const QString& profile, const QDate& date) {
    if (trainings_.contains(profile) && trainings_[profile].contains(date)) {
        trainingDiary_ = trainings_[profile][date];
        return;
    }

    const QString path = trainingFilePath(profile, date);
    TrainingDiary tmp;
    if (trainingSaver_.load(tmp, path.toStdString())) {
        trainingDiary_ = tmp;
        trainings_[profile][date] = tmp;
        return;
    }

    trainingDiary_ = TrainingDiary();
    trainings_[profile][date] = trainingDiary_;
}

void MainWindow::saveTrainingFor(const QString& profile, const QDate& date) {
    trainings_[profile][date] = trainingDiary_;
    ensureStorageDirs();
    const QString path = trainingFilePath(profile, date);
    trainingSaver_.save(trainingDiary_, path.toStdString());
}

void MainWindow::onDateChanged(const QDate& date) {
    // Save previous
    saveDiaryFor(currentProfile(), selectedDate_);
    saveTrainingFor(currentProfile(), selectedDate_);
    // Switch
    selectedDate_ = date;
    loadDiaryFor(currentProfile(), selectedDate_);
    loadTrainingFor(currentProfile(), selectedDate_);
    refreshDiary();
    refreshStats();
    refreshTraining();
    onShowDailyTip();
}

void MainWindow::onProfileChanged(int) {
    // Ensure dir exists for new profile
    ensureStorageDirs();
    // Save previous
    saveDiaryFor(currentProfile(), selectedDate_);
    saveTrainingFor(currentProfile(), selectedDate_);
    // Load for new profile/current date
    loadDiaryFor(currentProfile(), selectedDate_);
    loadTrainingFor(currentProfile(), selectedDate_);
    refreshDiary();
    refreshStats();
    refreshTraining();
    onShowDailyTip();
}

void MainWindow::onSearch() {
    resultsList_->clear();
    const QString query = searchEdit_->text().trimmed();
    QString selectedCategory = categoryFilter_->currentText();
    
    std::vector<Food> results;
    
    // Filter by category if not "Всі категорії"
    if (selectedCategory != tr("Всі категорії")) {
        results = foodDb_.searchByCategory(selectedCategory.toStdString());
        // Then filter by query if provided
        if (!query.isEmpty()) {
            std::vector<Food> filtered;
            std::string lowerQuery = query.toStdString();
            std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
            for (const auto& food : results) {
                std::string foodName = food.getName();
                std::transform(foodName.begin(), foodName.end(), foodName.begin(), ::tolower);
                if (foodName.find(lowerQuery) != std::string::npos) {
                    filtered.push_back(food);
                }
            }
            results = filtered;
        }
    } else {
        // Search all foods
        if (query.isEmpty()) {
            results = foodDb_.getAllFoods();
        } else {
            results = foodDb_.searchFoods(query.toStdString());
        }
    }
    
    for (const auto& food : results) {
        QString itemText = QString::fromStdString(food.getName()) + " [" + 
                          QString::fromStdString(food.getCategory()) + "] — " +
                          QString::number(food.getCalories(), 'f', 0) + tr(" ккал/100г");
        auto* item = new QListWidgetItem(itemText, resultsList_);
        item->setData(Qt::UserRole, QString::fromStdString(food.getName()));
    }
}

void MainWindow::onAddFood() {
    auto* item = resultsList_->currentItem();
    if (!item) return;

    const auto name = item->data(Qt::UserRole).toString().toStdString();
    auto results = foodDb_.searchFoods(name);
    if (results.empty()) return;
    auto food = results.front();

    const int grams = amountSpin_->value();
    const QString mealType = mealTypeCombo_->currentText();

    SavedMeal meal(mealType.toStdString(), food, grams);
    diary_.addMeal(meal);

    refreshDiary();
    refreshStats();
    // Save instantly to disk for persistence
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onRemoveMeal() {
    removeFromSelectedTab();
    refreshDiary();
    refreshStats();
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onSetGoal(int value) {
    diary_.setCalorieGoal(value);
    refreshStats();
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onAddCustomFood() {
    const QString name = customNameEdit_->text().trimmed();
    const double kcal = customCaloriesEdit_->value();
    const double carbs = customCarbsEdit_->value();
    const double protein = customProteinEdit_->value();
    const double fat = customFatEdit_->value();

    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Помилка"), tr("Вкажіть назву продукту."));
        return;
    }
    if (kcal <= 0.0 && carbs <= 0.0 && protein <= 0.0 && fat <= 0.0) {
        QMessageBox::warning(this, tr("Помилка"), tr("Заповніть хоча б одне поле харчової цінності."));
        return;
    }

    Food custom(name.toStdString(), kcal, carbs, protein, fat);
    foodDb_.addFood(custom);

    customNameEdit_->clear();
    customCaloriesEdit_->setValue(0.0);
    customCarbsEdit_->setValue(0.0);
    customProteinEdit_->setValue(0.0);
    customFatEdit_->setValue(0.0);

    QMessageBox::information(this, tr("Готово"), tr("Продукт додано до бази. Тепер його можна шукати."));
}

void MainWindow::onSaveTemplate() {
    QString mealType;
    switch (diaryTabs_->currentIndex()) {
        case 0: mealType = tr("Сніданок"); break;
        case 1: mealType = tr("Обід"); break;
        case 2: mealType = tr("Вечеря"); break;
        case 3: mealType = tr("Перекус"); break;
        default: return;
    }

    QVector<SavedMeal> tmpl;
    const auto meals = diary_.getAllMeals();
    for (const auto& m : meals) {
        if (QString::fromStdString(m.getMealName()) == mealType) tmpl.push_back(m);
    }

    if (tmpl.isEmpty()) {
        QMessageBox::information(this, tr("Порожньо"), tr("Немає елементів для збереження у шаблон."));
        return;
    }

    templatesPerMealType_[mealType] = tmpl;
    QMessageBox::information(this, tr("Шаблон збережено"), tr("Шаблон для '%1' оновлено.").arg(mealType));
}

void MainWindow::onApplyTemplate() {
    QString mealType;
    switch (diaryTabs_->currentIndex()) {
        case 0: mealType = tr("Сніданок"); break;
        case 1: mealType = tr("Обід"); break;
        case 2: mealType = tr("Вечеря"); break;
        case 3: mealType = tr("Перекус"); break;
        default: return;
    }

    if (!templatesPerMealType_.contains(mealType) || templatesPerMealType_[mealType].isEmpty()) {
        QMessageBox::information(this, tr("Немає шаблону"), tr("Спершу збережіть шаблон для цієї секції."));
        return;
    }

    for (const auto& meal : templatesPerMealType_[mealType]) diary_.addMeal(meal);

    refreshDiary();
    refreshStats();
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onAddWater() {
    diary_.addWater(waterAddSpin_->value());
    refreshStats();
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onSetWaterGoal(int value) {
    diary_.setWaterGoal(value);
    refreshStats();
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onSetWeight(double value) {
    diary_.setWeightKg(value);
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onOpenProfileDialog() {
    ProfileDialog dlg(loggedInLogin_, this);
    dlg.setProfileValues(weightSpin_->value(), ageSpin_->value(), heightSpin_->value(), activityCombo_->currentIndex());
    if (dlg.exec() != QDialog::Accepted) return;

    // Оновлюємо поля з діалогу
    if (weightSpin_) weightSpin_->setValue(dlg.weightKg());
    if (ageSpin_) ageSpin_->setValue(dlg.ageYears());
    if (heightSpin_) heightSpin_->setValue(dlg.heightCm());
    if (activityCombo_) activityCombo_->setCurrentIndex(dlg.activityIndex());

    // Застосовуємо до поточного дня/профілю
    diary_.setWeightKg(dlg.weightKg());
    const double estimated = estimateCalorieGoalFromProfile();
    diary_.setCalorieGoal(estimated);
    goalSpin_->setValue(static_cast<int>(estimated));
    saveProfileMeta(currentProfile());
    saveDiaryFor(currentProfile(), selectedDate_);
    refreshStats();
}

void MainWindow::onShowWeeklyReport() {
    // Load last 7 days including selected
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Звіт за 7 днів"));
    auto* lay = new QVBoxLayout(&dlg);
    auto* list = new QListWidget(&dlg);
    lay->addWidget(list);

    const QString profile = currentProfile();
    double sumCalories = 0.0;
    QDate today = QDate::currentDate();
    
    for (int i = 0; i < 7; ++i) {
        QDate day = selectedDate_.addDays(-i);
        Diary d;
        
        // If this is the currently selected day, use the in-memory diary
        if (day == selectedDate_) {
            d = diary_; // Use current diary data
        } else {
            // Try to load from file
            d.setCalorieGoal(2000);
            jsonSaver_.load(d, diaryFilePath(profile, day).toStdString());
        }
        
        const double kcal = d.getTotalCalories();
        if (kcal > 0 || day == selectedDate_) {
            sumCalories += kcal;
            list->addItem(day.toString("yyyy-MM-dd") + ": " + QString::number(kcal, 'f', 0) + tr(" ккал"));
        } else {
            list->addItem(day.toString("yyyy-MM-dd") + tr(": (нема даних)"));
        }
    }
    list->addItem(tr("Разом за 7 днів: ") + QString::number(sumCalories, 'f', 0) + tr(" ккал"));

    dlg.resize(420, 360);
    dlg.exec();
}

void MainWindow::onToggleTheme() {
    applyTheme(!isDarkTheme_);
}

void MainWindow::onExportCSV() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Експорт CSV"), "", "CSV Files (*.csv)");
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося відкрити файл для запису."));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "Дата,Профіль,Секція,Продукт,Кількість (г),Калорії,Білки (г),Жири (г),Вуглеводи (г),Вода (мл),Вага (кг)\n";

    const QString profile = currentProfile();
    QDate startDate = selectedDate_.addDays(-30);
    for (QDate d = startDate; d <= selectedDate_; d = d.addDays(1)) {
        Diary dDiary;
        if (jsonSaver_.load(dDiary, diaryFilePath(profile, d).toStdString())) {
            auto meals = dDiary.getAllMeals();
            for (const auto& meal : meals) {
                const auto& food = meal.getFood();
                out << d.toString("yyyy-MM-dd") << ","
                    << profile << ","
                    << QString::fromStdString(meal.getMealName()) << ","
                    << QString::fromStdString(food.getName()) << ","
                    << food.getAmount() << ","
                    << meal.getTotalCalories() << ","
                    << meal.getTotalProtein() << ","
                    << meal.getTotalFat() << ","
                    << meal.getTotalCarbs() << ","
                    << dDiary.getWaterMl() << ","
                    << dDiary.getWeightKg() << "\n";
            }
        }
    }

    file.close();
    QMessageBox::information(this, tr("Готово"), tr("Дані експортовано у CSV файл."));
}

void MainWindow::onImportCSV() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Імпорт CSV"), "", "CSV Files (*.csv)");
    if (filename.isEmpty()) return;

    std::map<std::string, std::map<std::string, Diary>> importedData;
    if (!CsvImportExport::importFromCSV(filename.toStdString(), importedData)) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося імпортувати дані з CSV файлу."));
        return;
    }

    // Load imported data into current view
    for (const auto& profilePair : importedData) {
        QString profile = QString::fromStdString(profilePair.first);
        for (const auto& datePair : profilePair.second) {
            QString dateStr = QString::fromStdString(datePair.first);
            QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
            if (!date.isValid()) continue;
            
            const Diary& importedDiary = datePair.second;
            QString profileKey = profile.isEmpty()
                ? (loggedInLogin_.isEmpty() ? QStringLiteral("user") : loggedInLogin_)
                : profile;
            
            // Save imported diary
            QString filePath = diaryFilePath(profileKey, date);
            jsonSaver_.save(importedDiary, filePath.toStdString());
        }
    }

    // Refresh current view
    loadDiaryFor(currentProfile(), selectedDate_);
    refreshDiary();
    refreshStats();
    
    QMessageBox::information(this, tr("Готово"), tr("Дані імпортовано з CSV файлу."));
}

void MainWindow::onExportPDF() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Експорт PDF"), "", "PDF Files (*.pdf)");
    if (filename.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename);
    printer.setPageSize(QPageSize::A4);

    QPainter painter(&printer);
    painter.setFont(QFont("Arial", 12));

    int y = 50;
    painter.drawText(50, y, tr("Звіт за %1").arg(selectedDate_.toString("yyyy-MM-dd")));
    y += 30;

    const auto meals = diary_.getAllMeals();
    for (const auto& meal : meals) {
        const auto& food = meal.getFood();
        QString line = QString::fromStdString(meal.getMealName()) + ": " +
            QString::fromStdString(food.getName()) + " — " +
            QString::number(meal.getTotalCalories(), 'f', 0) + tr(" ккал");
        painter.drawText(50, y, line);
        y += 20;
        if (y > printer.pageRect(QPrinter::DevicePixel).height() - 50) {
            printer.newPage();
            y = 50;
        }
    }

    y += 20;
    painter.drawText(50, y, tr("Всього калорій: %1").arg(diary_.getTotalCalories(), 0, 'f', 0));
    y += 20;
    painter.drawText(50, y, tr("Вода: %1 / %2 мл").arg(diary_.getWaterMl()).arg(diary_.getWaterGoalMl()));

    painter.end();
    QMessageBox::information(this, tr("Готово"), tr("PDF файл створено."));
}

void MainWindow::onShowCharts() {
    QDialog chartDlg(this);
    chartDlg.setWindowTitle(tr("Графіки"));
    chartDlg.resize(1000, 700);
    auto* layout = new QVBoxLayout(&chartDlg);
    
    const QString profile = currentProfile();
    
#ifdef QT_CHARTS_LIB
    auto* tabWidget = new QTabWidget(&chartDlg);
    
    // Calories chart (7 days)
    QChart* caloriesChart = new QChart();
    QBarSeries* caloriesSeries = new QBarSeries();
    QBarSet* caloriesSet = new QBarSet(tr("Калорії"));
    
    QStringList categories;
    for (int i = 6; i >= 0; --i) {
        QDate day = selectedDate_.addDays(-i);
        Diary d;
        if (day == selectedDate_) {
            d = diary_;
        } else {
            jsonSaver_.load(d, diaryFilePath(profile, day).toStdString());
        }
        *caloriesSet << d.getTotalCalories();
        categories << day.toString("MM-dd");
    }
    caloriesSeries->append(caloriesSet);
    caloriesChart->addSeries(caloriesSeries);
    caloriesChart->setTitle(tr("Калорії за 7 днів"));
    caloriesChart->setAnimationOptions(QChart::SeriesAnimations);
    
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    caloriesChart->addAxis(axisX, Qt::AlignBottom);
    caloriesSeries->attachAxis(axisX);
    
    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText(tr("Калорії (ккал)"));
    caloriesChart->addAxis(axisY, Qt::AlignLeft);
    caloriesSeries->attachAxis(axisY);
    
    caloriesChart->legend()->setVisible(true);
    caloriesChart->legend()->setAlignment(Qt::AlignBottom);
    
    QChartView* caloriesView = new QChartView(caloriesChart);
    caloriesView->setRenderHint(QPainter::Antialiasing);
    tabWidget->addTab(caloriesView, tr("Калорії (7 днів)"));
    
    // Water chart (7 days)
    QChart* waterChart = new QChart();
    QBarSeries* waterSeries = new QBarSeries();
    QBarSet* waterSet = new QBarSet(tr("Вода"));
    
    for (int i = 6; i >= 0; --i) {
        QDate day = selectedDate_.addDays(-i);
        Diary d;
        if (day == selectedDate_) {
            d = diary_;
        } else {
            jsonSaver_.load(d, diaryFilePath(profile, day).toStdString());
        }
        *waterSet << d.getWaterMl();
    }
    waterSeries->append(waterSet);
    waterChart->addSeries(waterSeries);
    waterChart->setTitle(tr("Вода за 7 днів"));
    waterChart->setAnimationOptions(QChart::SeriesAnimations);
    
    QBarCategoryAxis* waterAxisX = new QBarCategoryAxis();
    waterAxisX->append(categories);
    waterChart->addAxis(waterAxisX, Qt::AlignBottom);
    waterSeries->attachAxis(waterAxisX);
    
    QValueAxis* waterAxisY = new QValueAxis();
    waterAxisY->setTitleText(tr("Вода (мл)"));
    waterChart->addAxis(waterAxisY, Qt::AlignLeft);
    waterSeries->attachAxis(waterAxisY);
    
    waterChart->legend()->setVisible(true);
    waterChart->legend()->setAlignment(Qt::AlignBottom);
    
    QChartView* waterView = new QChartView(waterChart);
    waterView->setRenderHint(QPainter::Antialiasing);
    tabWidget->addTab(waterView, tr("Вода (7 днів)"));
    
    // Weight chart (30 days)
    QChart* weightChart = new QChart();
    QLineSeries* weightSeries = new QLineSeries();
    weightSeries->setName(tr("Вага"));
    
    QVector<QString> weightDates;
    for (int i = 29; i >= 0; --i) {
        QDate day = selectedDate_.addDays(-i);
        Diary d;
        if (day == selectedDate_) {
            d = diary_;
        } else {
            jsonSaver_.load(d, diaryFilePath(profile, day).toStdString());
        }
        double weight = d.getWeightKg();
        if (weight > 0) {
            weightSeries->append(29 - i, weight);
            weightDates.append(day.toString("MM-dd"));
        }
    }
    
    if (weightSeries->count() > 0) {
        weightChart->addSeries(weightSeries);
        weightChart->setTitle(tr("Вага за 30 днів"));
        weightChart->setAnimationOptions(QChart::SeriesAnimations);
        
        QValueAxis* weightAxisX = new QValueAxis();
        weightAxisX->setTitleText(tr("День"));
        weightAxisX->setRange(0, 29);
        weightChart->addAxis(weightAxisX, Qt::AlignBottom);
        weightSeries->attachAxis(weightAxisX);
        
        QValueAxis* weightAxisY = new QValueAxis();
        weightAxisY->setTitleText(tr("Вага (кг)"));
        weightChart->addAxis(weightAxisY, Qt::AlignLeft);
        weightSeries->attachAxis(weightAxisY);
        
        weightChart->legend()->setVisible(true);
        weightChart->legend()->setAlignment(Qt::AlignBottom);
    } else {
        QLabel* noDataLabel = new QLabel(tr("Немає даних про вагу"), &chartDlg);
        noDataLabel->setAlignment(Qt::AlignCenter);
        weightChart->setTitle(tr("Вага за 30 днів"));
    }
    
    QChartView* weightView = new QChartView(weightChart);
    weightView->setRenderHint(QPainter::Antialiasing);
    tabWidget->addTab(weightView, tr("Вага (30 днів)"));

    // Training activity chart (minutes per day, last 14 days)
    QChart* activityChart = new QChart();
    QBarSeries* activitySeries = new QBarSeries();
    QBarSet* activitySet = new QBarSet(tr("Хвилини тренувань"));

    QStringList activityCategories;
    const QString activityProfile = currentProfile();
    // Візьмемо останні 14 днів, щоб видно було динаміку
    for (int i = 13; i >= 0; --i) {
        QDate day = selectedDate_.addDays(-i);

        TrainingDiary dayTraining;
        if (trainings_.contains(activityProfile) && trainings_[activityProfile].contains(day)) {
            dayTraining = trainings_[activityProfile][day];
        } else {
            trainingSaver_.load(dayTraining, trainingFilePath(activityProfile, day).toStdString());
        }

        *activitySet << dayTraining.getTotalDurationMin();
        activityCategories << day.toString("MM-dd");
    }

    activitySeries->append(activitySet);
    activityChart->addSeries(activitySeries);
    activityChart->setTitle(tr("Активність тренувань за 14 днів"));
    activityChart->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis* activityAxisX = new QBarCategoryAxis();
    activityAxisX->append(activityCategories);
    activityChart->addAxis(activityAxisX, Qt::AlignBottom);
    activitySeries->attachAxis(activityAxisX);

    QValueAxis* activityAxisY = new QValueAxis();
    activityAxisY->setTitleText(tr("Хвилини тренувань"));
    activityAxisY->setLabelFormat("%d");
    activityChart->addAxis(activityAxisY, Qt::AlignLeft);
    activitySeries->attachAxis(activityAxisY);

    activityChart->legend()->setVisible(true);
    activityChart->legend()->setAlignment(Qt::AlignBottom);

    QChartView* activityView = new QChartView(activityChart);
    activityView->setRenderHint(QPainter::Antialiasing);
    tabWidget->addTab(activityView, tr("Активність (14 днів)"));
    
    layout->addWidget(tabWidget);
#else
    // Fallback to text charts if Qt Charts is not available
    auto* infoLabel = new QLabel(&chartDlg);
    QString chartText = tr("Графік калорій за останні 7 днів:\n\n");

    QVector<double> calories;
    QVector<QString> dates;
    for (int i = 6; i >= 0; --i) {
        QDate day = selectedDate_.addDays(-i);
        Diary d;
        if (day == selectedDate_) {
            d = diary_;
        } else {
            jsonSaver_.load(d, diaryFilePath(profile, day).toStdString());
        }
        calories.append(d.getTotalCalories());
        dates.append(day.toString("MM-dd"));
    }

    double maxCal = *std::max_element(calories.begin(), calories.end());
    if (maxCal == 0) maxCal = 2000;

    for (int i = 0; i < dates.size(); ++i) {
        int barLength = static_cast<int>((calories[i] / maxCal) * 50);
        QString bar = QString(barLength, QChar(0x2588));
        chartText += dates[i] + ": " + QString::number(calories[i], 'f', 0) + tr(" ккал ") + bar + "\n";
    }

    chartText += "\n" + tr("Графік води за останні 7 днів:\n\n");
    QVector<int> water;
    for (int i = 6; i >= 0; --i) {
        QDate day = selectedDate_.addDays(-i);
        Diary d;
        if (day == selectedDate_) {
            d = diary_;
        } else {
            jsonSaver_.load(d, diaryFilePath(profile, day).toStdString());
        }
        water.append(d.getWaterMl());
    }

    int maxWater = *std::max_element(water.begin(), water.end());
    if (maxWater == 0) maxWater = 2000;

    for (int i = 0; i < dates.size(); ++i) {
        int barLength = static_cast<int>((static_cast<double>(water[i]) / maxWater) * 50);
        QString bar = QString(barLength, QChar(0x2588));
        chartText += dates[i] + ": " + QString::number(water[i]) + tr(" мл ") + bar + "\n";
    }

    // Training activity (ASCII) for last 14 days
    chartText += "\n" + tr("Графік тренувань за останні 14 днів (хв/день):\n\n");
    QVector<int> minutes;
    QVector<QString> activityDates;
    const QString activityProfile = currentProfile();
    for (int i = 13; i >= 0; --i) {
        QDate day = selectedDate_.addDays(-i);
        TrainingDiary dayTraining;
        if (trainings_.contains(activityProfile) && trainings_[activityProfile].contains(day)) {
            dayTraining = trainings_[activityProfile][day];
        } else {
            trainingSaver_.load(dayTraining, trainingFilePath(activityProfile, day).toStdString());
        }
        minutes.append(dayTraining.getTotalDurationMin());
        activityDates.append(day.toString("MM-dd"));
    }
    int maxMin = minutes.isEmpty() ? 0 : *std::max_element(minutes.begin(), minutes.end());
    if (maxMin == 0) maxMin = 60;
    for (int i = 0; i < activityDates.size(); ++i) {
        int barLength = static_cast<int>((static_cast<double>(minutes[i]) / maxMin) * 50);
        QString bar = QString(barLength, QChar(0x2588));
        chartText += activityDates[i] + ": " + QString::number(minutes[i]) + tr(" хв ") + bar + "\n";
    }

    infoLabel->setText(chartText);
    infoLabel->setFont(QFont("Courier", 10));
    layout->addWidget(infoLabel);
#endif
    chartDlg.exec();
}

void MainWindow::refreshDiary() {
    breakfastList_->clear();
    lunchList_->clear();
    dinnerList_->clear();
    snackList_->clear();

    const auto meals = diary_.getAllMeals();
    for (const auto& m : meals) {
        const auto& f = m.getFood();
        const QString line = QString::fromStdString(f.getName()) + " — "
            + QString::number(f.getAmount(), 'f', 0) + " г, "
            + QString::number(m.getTotalCalories(), 'f', 0) + " ккал";
        const QString mealName = QString::fromStdString(m.getMealName());
        if (mealName == tr("Сніданок")) {
            breakfastList_->addItem(line);
        } else if (mealName == tr("Обід")) {
            lunchList_->addItem(line);
        } else if (mealName == tr("Вечеря")) {
            dinnerList_->addItem(line);
        } else {
            snackList_->addItem(line);
        }
    }
}

void MainWindow::refreshStats() {
    goalSpin_->setValue(static_cast<int>(diary_.getCalorieGoal()));
    const double eaten = diary_.getTotalCalories();
    const double remaining = diary_.getRemainingCalories();

    caloriesLabel_->setText(
        tr("Калорії: з'їдено %1 ккал / ціль %2 ккал / залишок %3 ккал")
            .arg(QString::number(eaten, 'f', 0))
            .arg(QString::number(diary_.getCalorieGoal(), 'f', 0))
            .arg(QString::number(remaining, 'f', 0))
    );

    macrosLabel_->setText(
        tr("Б: %1 г  |  Ж: %2 г  |  В: %3 г")
            .arg(QString::number(diary_.getTotalProtein(), 'f', 1))
            .arg(QString::number(diary_.getTotalFat(), 'f', 1))
            .arg(QString::number(diary_.getTotalCarbs(), 'f', 1))
    );

    // Progress bars
    const int calProgress = diary_.getCalorieGoal() > 0
        ? static_cast<int>((eaten / diary_.getCalorieGoal()) * 100.0)
        : 0;
    caloriesProgress_->setValue(std::clamp(calProgress, 0, 100));

    waterGoalSpin_->setValue(diary_.getWaterGoalMl());
    const int waterProgress = diary_.getWaterGoalMl() > 0
        ? static_cast<int>((static_cast<double>(diary_.getWaterMl()) / diary_.getWaterGoalMl()) * 100.0)
        : 0;
    waterProgress_->setValue(std::clamp(waterProgress, 0, 100));
}

void MainWindow::refreshTraining() {
    trainingsList_->clear();

    const auto sessions = trainingDiary_.getAllSessions();
    for (int i = 0; i < static_cast<int>(sessions.size()); ++i) {
        const auto& s = sessions[i];

        QString typeDisplay;
        const std::string storedType = s.getType();
        if (storedType == "Strength") typeDisplay = tr("Сила");
        else if (storedType == "Cardio") typeDisplay = tr("Кардіо");
        else if (storedType == "Mobility") typeDisplay = tr("Мобільність");
        else if (storedType == "Rest") typeDisplay = tr("Відпочинок");
        else typeDisplay = QString::fromStdString(storedType);

        const QString statusDisplay =
            (s.getStatus() == TrainingSession::Status::Completed) ? tr("Виконано") : tr("План");

        const QString timeDisplay = QString::fromStdString(s.getTimeHHmm());
        const int dur = s.getDurationMin();
        const QString notes = QString::fromStdString(s.getNotes());

        QString line = QString("%1 | %2 | %3 хв | %4").arg(timeDisplay, typeDisplay).arg(dur).arg(statusDisplay);
        if (!notes.trimmed().isEmpty()) line += " — " + notes;

        auto* item = new QListWidgetItem(line, trainingsList_);
        item->setData(Qt::UserRole, i);
    }

    trainingSummaryLabel_->setText(
        tr("Тренування сьогодні: %1 хв").arg(QString::number(trainingDiary_.getTotalDurationMin())));

    const int streak = calculateConsistencyStreak();
    fitnessKpiLabel_->setText(
        tr("Серія активності: %1 дн.  |  Сесій сьогодні: %2")
            .arg(streak)
            .arg(static_cast<int>(sessions.size())));
}

int MainWindow::calculateConsistencyStreak() {
    const QString profile = currentProfile();
    int streak = 0;

    for (int i = 0; i < 30; ++i) {
        const QDate day = selectedDate_.addDays(-i);

        Diary dayDiary;
        if (diaries_.contains(profile) && diaries_[profile].contains(day)) {
            dayDiary = diaries_[profile][day];
        } else {
            jsonSaver_.load(dayDiary, diaryFilePath(profile, day).toStdString());
        }

        TrainingDiary dayTraining;
        if (trainings_.contains(profile) && trainings_[profile].contains(day)) {
            dayTraining = trainings_[profile][day];
        } else {
            trainingSaver_.load(dayTraining, trainingFilePath(profile, day).toStdString());
        }

        const bool hasNutrition = dayDiary.getTotalCalories() > 0.0;
        const bool hasTraining = dayTraining.getTotalDurationMin() >= 20;
        if (!hasNutrition && !hasTraining) break;
        ++streak;
    }

    return streak;
}

void MainWindow::onAddTraining() {
    const QString typeText = trainingTypeCombo_->currentText();
    std::string type;
    if (typeText == tr("Сила")) type = "Strength";
    else if (typeText == tr("Кардіо")) type = "Cardio";
    else if (typeText == tr("Мобільність")) type = "Mobility";
    else type = "Rest";

    const int durationMin = trainingDurationSpin_->value();
    if (durationMin <= 0 || type == "Rest") return;

    const QString timeStr = trainingTimeEdit_->time().toString("HH:mm");

    const auto status = (trainingStatusCombo_->currentIndex() == 1)
        ? TrainingSession::Status::Completed
        : TrainingSession::Status::Planned;

    const std::string notes = trainingNotesEdit_->text().trimmed().toStdString();

    trainingDiary_.addSession(TrainingSession(type, durationMin, timeStr.toStdString(), status, notes));
    refreshTraining();
    saveTrainingFor(currentProfile(), selectedDate_);
}

void MainWindow::onRemoveTraining() {
    auto* item = trainingsList_->currentItem();
    if (!item) return;

    const int idx = item->data(Qt::UserRole).toInt();
    trainingDiary_.removeSession(idx);
    refreshTraining();
    saveTrainingFor(currentProfile(), selectedDate_);
}

void MainWindow::onGenerateTrainingPlan7Days() {
    const QString goalText = trainingGoalCombo_->currentText();
    TrainingPreferences prefs;
    if (goalText == tr("Схуднення")) prefs.goal = "cutting";
    else if (goalText == tr("Набір")) prefs.goal = "bulk";
    else prefs.goal = "maintenance";

    TrainingPlanGenerator generator;
    const QString profile = currentProfile();

    // Generate for next 7 days starting from selectedDate_ (non-destructive: only fill empty days).
    for (int i = 0; i < 7; ++i) {
        const QDate day = selectedDate_.addDays(i);
        TrainingSession session = generator.generateSessionForDay(i, prefs);

        // Load existing day (from memory or disk).
        TrainingDiary existing;
        bool hasInMemory = trainings_.contains(profile) && trainings_[profile].contains(day);
        if (hasInMemory) {
            existing = trainings_[profile][day];
        } else {
            const QString basePath = trainingFilePath(profile, day);
            if (trainingSaver_.load(existing, basePath.toStdString())) {
                trainings_[profile][day] = existing;
            } else {
                existing = TrainingDiary();
                trainings_[profile][day] = existing;
            }
        }

        if (!existing.isEmpty()) continue;
        if (session.getDurationMin() <= 0) continue; // rest day

        existing.addSession(session);
        trainingDiary_ = existing;
        saveTrainingFor(profile, day);
    }

    loadTrainingFor(profile, selectedDate_);
    refreshTraining();
    QMessageBox::information(this,
                             tr("План згенеровано"),
                             tr("Автоплан на 7 днів збережено. Дні з наявними тренуваннями не перезаписуються."));
}

void MainWindow::onAskOfflineAssistant() {
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Офлайн-помічник"));
    dlg.resize(700, 520);

    auto* lay = new QVBoxLayout(&dlg);
    auto* chat = new QPlainTextEdit(&dlg);
    chat->setReadOnly(true);
    chat->setPlaceholderText(tr("Тут будуть відповіді..."));

    auto* input = new QLineEdit(&dlg);
    input->setPlaceholderText(tr("Поставте питання про харчування або тренування..."));

    auto* btnRow = new QHBoxLayout();
    auto* askOfflineBtn = new QPushButton(tr("Задати (Offline)"), &dlg);
    auto* askOnlineBtn = new QPushButton(tr("Онлайн-помічник (заглушка)"), &dlg);
    btnRow->addWidget(askOfflineBtn);
    btnRow->addWidget(askOnlineBtn);

    lay->addWidget(chat, 1);
    lay->addWidget(input);
    lay->addLayout(btnRow);

    const auto getPrefs = [&]() -> TrainingPreferences {
        const QString goalText = trainingGoalCombo_->currentText();
        TrainingPreferences prefs;
        if (goalText == tr("Схуднення")) prefs.goal = "cutting";
        else if (goalText == tr("Набір")) prefs.goal = "bulk";
        else prefs.goal = "maintenance";
        return prefs;
    };

    connect(askOfflineBtn, &QPushButton::clicked, [&]() {
        const QString q = input->text().trimmed();
        if (q.isEmpty()) return;

        chat->appendPlainText(tr("Ви: %1").arg(q));

        OfflineAssistant assistant;
        const std::string resp = assistant.getRecommendation(q.toStdString(), diary_, trainingDiary_, getPrefs());
        chat->appendPlainText(QString::fromStdString(resp));

        input->clear();
    });

    connect(askOnlineBtn, &QPushButton::clicked, this, &MainWindow::onAskOnlineAssistantPlaceholder);

    dlg.exec();
}

void MainWindow::onAskOnlineAssistantPlaceholder() {
    QMessageBox::information(this,
                             tr("Онлайн-помічник"),
                             tr("У цьому навчальному проєкті онлайн-помічник ще не інтегровано. Використовується локальний помічник."));
}

void MainWindow::onMarkTrainingCompleted() {
    auto* item = trainingsList_->currentItem();
    if (!item) return;

    const int idx = item->data(Qt::UserRole).toInt();
    auto sessions = trainingDiary_.getAllSessions();
    if (idx < 0 || idx >= static_cast<int>(sessions.size())) return;

    const auto& s = sessions[static_cast<size_t>(idx)];
    sessions[static_cast<size_t>(idx)] = TrainingSession(
        s.getType(), s.getDurationMin(), s.getTimeHHmm(), TrainingSession::Status::Completed, s.getNotes());

    trainingDiary_.clear();
    for (const auto& session : sessions) {
        trainingDiary_.addSession(session);
    }

    refreshTraining();
    saveTrainingFor(currentProfile(), selectedDate_);
}

void MainWindow::onDuplicateTrainingTomorrow() {
    auto* item = trainingsList_->currentItem();
    if (!item) return;

    const int idx = item->data(Qt::UserRole).toInt();
    auto sessions = trainingDiary_.getAllSessions();
    if (idx < 0 || idx >= static_cast<int>(sessions.size())) return;

    const auto& source = sessions[static_cast<size_t>(idx)];
    const QDate tomorrow = selectedDate_.addDays(1);
    const QString profile = currentProfile();

    TrainingDiary tomorrowDiary;
    if (trainings_.contains(profile) && trainings_[profile].contains(tomorrow)) {
        tomorrowDiary = trainings_[profile][tomorrow];
    } else {
        trainingSaver_.load(tomorrowDiary, trainingFilePath(profile, tomorrow).toStdString());
    }

    tomorrowDiary.addSession(TrainingSession(
        source.getType(),
        source.getDurationMin(),
        source.getTimeHHmm(),
        TrainingSession::Status::Planned,
        source.getNotes()));

    trainings_[profile][tomorrow] = tomorrowDiary;
    const QString basePath = trainingFilePath(profile, tomorrow);
    trainingSaver_.save(tomorrowDiary, basePath.toStdString());

    QMessageBox::information(this, tr("Готово"), tr("Сесію дубльовано на завтра."));
}

void MainWindow::onShowDailyTip() {
    const QStringList tips = {
        tr("Невелика стабільна рутина краща за ідеальний план раз на тиждень."),
        tr("Після тренування додайте порцію білка для кращого відновлення."),
        tr("Якщо важко почати: зробіть лише 10 хвилин, а потім вирішуйте, чи продовжувати."),
        tr("Пийте воду порціями протягом дня, а не великим обсягом за один раз."),
        tr("Записуйте коротку нотатку після тренування: це допомагає бачити прогрес."),
        tr("Фокус на базових речах: сон, вода, регулярність і помірний дефіцит/профіцит.")
    };

    const int idx = selectedDate_.dayOfYear() % tips.size();
    tipLabel_->setText(tr("Порада дня: %1").arg(tips[idx]));
}

void MainWindow::onShowTrainingWeeklyReport() {
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Звіт тренувань за 7 днів"));
    dlg.resize(520, 420);

    auto* lay = new QVBoxLayout(&dlg);
    auto* list = new QListWidget(&dlg);
    lay->addWidget(list);

    const QString profile = currentProfile();
    int totalMinutes = 0;
    int totalSessions = 0;
    int completedSessions = 0;

    for (int i = 0; i < 7; ++i) {
        const QDate day = selectedDate_.addDays(-i);
        TrainingDiary dayTraining;

        if (trainings_.contains(profile) && trainings_[profile].contains(day)) {
            dayTraining = trainings_[profile][day];
        } else {
            trainingSaver_.load(dayTraining, trainingFilePath(profile, day).toStdString());
        }

        const auto sessions = dayTraining.getAllSessions();
        const int dayMinutes = dayTraining.getTotalDurationMin();
        int dayCompleted = 0;
        for (const auto& s : sessions) {
            if (s.getStatus() == TrainingSession::Status::Completed) {
                ++dayCompleted;
            }
        }

        totalMinutes += dayMinutes;
        totalSessions += static_cast<int>(sessions.size());
        completedSessions += dayCompleted;

        list->addItem(
            tr("%1: %2 хв, сесій %3, виконано %4")
                .arg(day.toString("yyyy-MM-dd"))
                .arg(dayMinutes)
                .arg(static_cast<int>(sessions.size()))
                .arg(dayCompleted)
        );
    }

    const int completionRate = totalSessions > 0
        ? static_cast<int>((static_cast<double>(completedSessions) / totalSessions) * 100.0)
        : 0;

    list->addItem(QString());
    list->addItem(tr("Разом: %1 хв").arg(totalMinutes));
    list->addItem(tr("Сесій: %1").arg(totalSessions));
    list->addItem(tr("Виконано: %1 (%2%)").arg(completedSessions).arg(completionRate));

    dlg.exec();
}

void MainWindow::onStartWorkout() {
    // 1) Check minimal personal data; request if missing
    const bool hasPersonalData = (weightSpin_->value() > 0.0 && ageSpin_->value() > 0 && heightSpin_->value() > 0.0);
    if (!hasPersonalData) {
        QDialog dataDlg(this);
        dataDlg.setWindowTitle(tr("Мінімальні персональні дані"));
        auto* lay = new QVBoxLayout(&dataDlg);
        auto* form = new QFormLayout();
        auto* goalBox = new QComboBox(&dataDlg);
        goalBox->addItems({tr("Схуднення"), tr("Набір"), tr("Підтримка")});
        auto* levelBox = new QComboBox(&dataDlg);
        levelBox->addItems({tr("Початковий"), tr("Середній"), tr("Просунутий")});
        auto* durationBox = new QSpinBox(&dataDlg);
        durationBox->setRange(10, 120);
        durationBox->setValue(30);
        durationBox->setSuffix(tr(" хв"));
        form->addRow(tr("Ціль:"), goalBox);
        form->addRow(tr("Рівень:"), levelBox);
        form->addRow(tr("Тривалість:"), durationBox);
        lay->addLayout(form);
        auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dataDlg);
        lay->addWidget(btns);
        connect(btns, &QDialogButtonBox::accepted, &dataDlg, &QDialog::accept);
        connect(btns, &QDialogButtonBox::rejected, &dataDlg, &QDialog::reject);
        if (dataDlg.exec() != QDialog::Accepted) return;

        trainingGoalCombo_->setCurrentText(goalBox->currentText());
        trainingDurationSpin_->setValue(durationBox->value());
        // Мапимо рівень на активність профілю
        activityCombo_->setCurrentIndex(levelBox->currentIndex());
    }

    // 2) Generate plan for today from current preferences
    TrainingPreferences prefs;
    const QString goalText = trainingGoalCombo_->currentText();
    if (goalText == tr("Схуднення")) prefs.goal = "cutting";
    else if (goalText == tr("Набір")) prefs.goal = "bulk";
    else prefs.goal = "maintenance";

    TrainingPlanGenerator generator;
    TrainingSession planned = generator.generateSessionForDay(0, prefs);
    if (planned.getDurationMin() <= 0 || planned.getType() == "Rest") {
        planned = TrainingSession("Cardio", std::max(20, trainingDurationSpin_->value()), "07:30",
                                  TrainingSession::Status::Planned, "");
    } else {
        planned = TrainingSession(planned.getType(), std::max(10, trainingDurationSpin_->value()),
                                  planned.getTimeHHmm(), TrainingSession::Status::Planned, "");
    }

    // 3) Show plan and start
    const QString typeUa = (planned.getType() == "Strength") ? tr("Сила")
                           : (planned.getType() == "Cardio") ? tr("Кардіо")
                           : (planned.getType() == "Mobility") ? tr("Мобільність")
                           : tr("Тренування");
    if (QMessageBox::question(
            this,
            tr("План тренування"),
            tr("План на сьогодні:\n- Тип: %1\n- Тривалість: %2 хв\n\nПочати тренування?")
                .arg(typeUa)
                .arg(planned.getDurationMin()))
        != QMessageBox::Yes) {
        return;
    }

    // 4) Guided workout progress
    QDialog workoutDlg(this);
    workoutDlg.setWindowTitle(tr("Workout in progress"));
    workoutDlg.resize(520, 360);
    auto* lay = new QVBoxLayout(&workoutDlg);
    auto* info = new QLabel(tr("Виконуйте кроки по черзі. Прогрес оновлюється автоматично."), &workoutDlg);
    auto* c1 = new QCheckBox(tr("Розминка (5 хв)"), &workoutDlg);
    auto* c2 = new QCheckBox(tr("Основний блок"), &workoutDlg);
    auto* c3 = new QCheckBox(tr("Заминка (5 хв)"), &workoutDlg);
    auto* progress = new QProgressBar(&workoutDlg);
    progress->setRange(0, 100);
    progress->setValue(0);
    auto* finishBtn = new QPushButton(tr("Завершити тренування"), &workoutDlg);
    finishBtn->setEnabled(false);
    lay->addWidget(info);
    lay->addWidget(c1);
    lay->addWidget(c2);
    lay->addWidget(c3);
    lay->addWidget(progress);
    lay->addWidget(finishBtn);

    auto recomputeProgress = [=]() {
        int done = (c1->isChecked() ? 1 : 0) + (c2->isChecked() ? 1 : 0) + (c3->isChecked() ? 1 : 0);
        progress->setValue(done * 33 + (done == 3 ? 1 : 0));
        finishBtn->setEnabled(done == 3);
    };
    connect(c1, &QCheckBox::toggled, &workoutDlg, recomputeProgress);
    connect(c2, &QCheckBox::toggled, &workoutDlg, recomputeProgress);
    connect(c3, &QCheckBox::toggled, &workoutDlg, recomputeProgress);
    connect(finishBtn, &QPushButton::clicked, &workoutDlg, &QDialog::accept);

    if (workoutDlg.exec() != QDialog::Accepted) return;

    // 5) Save completed session and show summary
    trainingDiary_.addSession(TrainingSession(planned.getType(), planned.getDurationMin(), planned.getTimeHHmm(),
                                              TrainingSession::Status::Completed, "Started via Start workout"));
    saveTrainingFor(currentProfile(), selectedDate_);
    refreshTraining();

    QMessageBox::information(this, tr("Тренування завершено"),
                             tr("Тренування успішно завершено.\nЗбережено: %1, %2 хв.\nВи можете переглянути прогрес у дашборді.")
                                 .arg(typeUa)
                                 .arg(planned.getDurationMin()));
}

void MainWindow::onQuickStartWorkout() {
    // A1: skip personal data input, generate basic plan
    trainingGoalCombo_->setCurrentText(tr("Підтримка"));
    if (trainingDurationSpin_->value() <= 0) trainingDurationSpin_->setValue(25);
    onStartWorkout();
}

void MainWindow::removeFromSelectedTab() {
    int currentTab = diaryTabs_->currentIndex();
    QListWidget* currentList = nullptr;
    QString tabName;
    switch (currentTab) {
        case 0: currentList = breakfastList_; tabName = tr("Сніданок"); break;
        case 1: currentList = lunchList_; tabName = tr("Обід"); break;
        case 2: currentList = dinnerList_; tabName = tr("Вечеря"); break;
        case 3: currentList = snackList_; tabName = tr("Перекус"); break;
        default: return;
    }
    auto* item = currentList->currentItem();
    if (!item) return;
    int rowInTab = currentList->row(item);

    const auto meals = diary_.getAllMeals();
    int matchIdx = -1;
    int countInTab = -1;
    for (size_t i = 0; i < meals.size(); ++i) {
        if (QString::fromStdString(meals[i].getMealName()) == tabName) {
            ++countInTab;
            if (countInTab == rowInTab) { matchIdx = static_cast<int>(i); break; }
        }
    }
    if (matchIdx >= 0) diary_.removeMeal(matchIdx);
}
