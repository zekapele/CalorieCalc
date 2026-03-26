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
#include <map>
#include <cctype>
#include <QTabWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
}

void MainWindow::setupUi() {
    auto* appTabs = new QTabWidget(this);
    auto* caloriesPage = new QWidget(appTabs);
    auto* fitnessPage = new QWidget(appTabs);
    auto* rootLayout = new QHBoxLayout(caloriesPage);

    // Left column (controls)
    auto* leftColumn = new QVBoxLayout();

    // Top controls: profile + calendar
    auto* topBox = new QGroupBox(tr("Профіль і календар"), caloriesPage);
    auto* topLayout = new QVBoxLayout(topBox);

    auto* profileRow = new QHBoxLayout();
    profileCombo_ = new QComboBox(topBox);
    profileCombo_->setEditable(true);
    profileCombo_->addItem(tr("Без профілю"));
    profileCombo_->setCurrentIndex(0);
    profileRow->addWidget(new QLabel(tr("Профіль:"), topBox));
    profileRow->addWidget(profileCombo_);

    calendar_ = new QCalendarWidget(topBox);
    calendar_->setGridVisible(true);
    selectedDate_ = QDate::currentDate();
    calendar_->setSelectedDate(selectedDate_);

    topLayout->addLayout(profileRow);
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
    assistantBtn_ = new QPushButton(tr("AI-помічник"), fitnessPage);
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
    markCompletedBtn_ = new QPushButton(tr("Позначити як виконано"), fitnessPage);
    duplicateTomorrowBtn_ = new QPushButton(tr("Дублювати на завтра"), fitnessPage);
    trainingQuickActions->addWidget(markCompletedBtn_);
    trainingQuickActions->addWidget(duplicateTomorrowBtn_);

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
    connect(profileCombo_->lineEdit(), &QLineEdit::editingFinished, [this]() {
        const QString name = profileCombo_->currentText().trimmed();
        if (name.isEmpty()) return;
        int idx = profileCombo_->findText(name);
        if (idx < 0) profileCombo_->addItem(name);
        profileCombo_->setCurrentText(name);
        onProfileChanged(profileCombo_->currentIndex());
    });
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

    // Init storage
    ensureStorageDirs();
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
            "QGroupBox { font-weight: 600; border: 1px solid #433558; border-radius: 12px; margin-top: 14px; padding-top: 8px; color: #EEEAF5; background: #1F182A; }"
            "QGroupBox::title { padding: 0 10px; color: #8BE9C1; subcontrol-origin: margin; subcontrol-position: top left; }"
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3A2D4F, stop:1 #2D2340); color: #EEEAF5; padding: 8px 16px; border: 1px solid #564375; border-radius: 10px; font-weight: 600; }"
            "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4A3A65, stop:1 #3A2E53); border-color: #8BE9C1; color: #B5F5DC; }"
            "QPushButton:pressed { background: #271E37; }"
            "QLineEdit, QSpinBox, QComboBox, QDoubleSpinBox { padding: 8px 12px; border: 1px solid #564375; border-radius: 10px; background: #20192E; color: #EEEAF5; selection-background-color: #8BE9C1; selection-color: #1F182A; }"
            "QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDoubleSpinBox:focus { border: 2px solid #8BE9C1; background: #2A2139; }"
            "QListWidget { background: #20192E; border: 1px solid #564375; border-radius: 10px; color: #EEEAF5; }"
            "QListWidget::item { padding: 6px; border-radius: 4px; }"
            "QListWidget::item:selected { background: #8BE9C1; color: #1F182A; }"
            "QListWidget::item:hover { background: #2E2441; }"
            "QTabBar::tab { background: #2A2139; color: #B7AFC7; padding: 10px 18px; border-top-left-radius: 10px; border-top-right-radius: 10px; margin-right: 2px; border: 1px solid #433558; border-bottom: none; }"
            "QTabBar::tab:selected { background: #20192E; color: #8BE9C1; border-color: #433558; border-bottom: 2px solid #8BE9C1; font-weight: 700; }"
            "QTabBar::tab:hover:!selected { background: #332848; color: #E5E0F0; }"
            "QTabWidget::pane { border: 1px solid #433558; border-radius: 8px; top: -1px; background: #20192E; }"
            "QLabel { color: #EEEAF5; }"
            "QCalendarWidget { background-color: #20192E; color: #EEEAF5; border: 1px solid #433558; border-radius: 10px; }"
            "QCalendarWidget QAbstractItemView:enabled { selection-background-color: #8BE9C1; selection-color: #1F182A; background-color: #20192E; }"
            "QCalendarWidget QHeaderView::section { background-color: #2A2139; color: #EEEAF5; border: none; padding: 8px; }"
            "QProgressBar { border: 1px solid #433558; border-radius: 10px; background: #20192E; color: #EEEAF5; text-align: center; height: 20px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8BE9C1, stop:1 #7CD6F3); border-radius: 10px; }"
            "QLabel#fitnessKpiLabel { font-size: 14px; font-weight: 700; color: #8BE9C1; padding: 6px 8px; background: #2A2139; border: 1px solid #433558; border-radius: 8px; }"
            "QLabel#tipLabel { color: #E9FFF6; background: #2A2A3F; border-left: 3px solid #8BE9C1; padding: 8px; border-radius: 8px; }"
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
            "QMainWindow { background-color: #F7F4FF; }"
            "QGroupBox { font-weight: 600; border: 1px solid #DDD4F2; border-radius: 12px; margin-top: 14px; padding-top: 8px; color: #291F3B; background: #FFFFFF; }"
            "QGroupBox::title { padding: 0 10px; color: #6D4AFF; subcontrol-origin: margin; subcontrol-position: top left; }"
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F4EFFF, stop:1 #EDE6FF); color: #291F3B; padding: 8px 16px; border: 1px solid #D8CCF2; border-radius: 10px; font-weight: 600; }"
            "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #EDE6FF, stop:1 #E4D9FF); border-color: #34D399; color: #195F48; }"
            "QPushButton:pressed { background: #DCD1FA; }"
            "QLineEdit, QSpinBox, QComboBox, QDoubleSpinBox { padding: 8px 12px; border: 1px solid #D8CCF2; border-radius: 10px; background: #FFFFFF; color: #291F3B; selection-background-color: #34D399; selection-color: #FFFFFF; }"
            "QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDoubleSpinBox:focus { border: 2px solid #34D399; background: #FCFCFF; }"
            "QListWidget { background: #FFFFFF; border: 1px solid #E1D8F4; border-radius: 10px; color: #291F3B; }"
            "QListWidget::item { padding: 6px; border-radius: 4px; }"
            "QListWidget::item:selected { background: #34D399; color: #FFFFFF; }"
            "QListWidget::item:hover { background: #F5F0FF; }"
            "QTabBar::tab { background: #F3EEFF; color: #6E6290; padding: 10px 18px; border-top-left-radius: 10px; border-top-right-radius: 10px; margin-right: 2px; border: 1px solid #E1D8F4; border-bottom: none; }"
            "QTabBar::tab:selected { background: #FFFFFF; color: #6D4AFF; border-color: #E1D8F4; border-bottom: 2px solid #34D399; font-weight: 700; }"
            "QTabBar::tab:hover:!selected { background: #ECE3FF; color: #4F426E; }"
            "QTabWidget::pane { border: 1px solid #E1D8F4; border-radius: 8px; top: -1px; background: #FFFFFF; }"
            "QLabel { color: #291F3B; }"
            "QCalendarWidget { background-color: #FFFFFF; color: #291F3B; border: 1px solid #E1D8F4; border-radius: 10px; }"
            "QCalendarWidget QAbstractItemView:enabled { selection-background-color: #34D399; selection-color: #FFFFFF; background-color: #FFFFFF; }"
            "QCalendarWidget QHeaderView::section { background-color: #F6F1FF; color: #291F3B; border: none; padding: 8px; }"
            "QProgressBar { border: 1px solid #D8CCF2; border-radius: 10px; background: #F5F0FF; color: #291F3B; text-align: center; height: 20px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #34D399, stop:1 #7CD6F3); border-radius: 10px; }"
            "QLabel#fitnessKpiLabel { font-size: 14px; font-weight: 700; color: #3E2EA8; padding: 6px 8px; background: #F1ECFF; border: 1px solid #D8CCF2; border-radius: 8px; }"
            "QLabel#tipLabel { color: #2A3F35; background: #ECFFF7; border-left: 3px solid #34D399; padding: 8px; border-radius: 8px; }"
        );
    }
}

QString MainWindow::currentProfile() const {
    const QString name = profileCombo_ && !profileCombo_->currentText().trimmed().isEmpty()
        ? profileCombo_->currentText().trimmed()
        : tr("Без профілю");
    return name;
}

void MainWindow::ensureStorageDirs() {
    QDir base(QDir::currentPath());
    if (!base.exists("data")) base.mkdir("data");
    QDir dataDir(base.filePath("data"));
    const QString profileDirName = currentProfile() == tr("Без профілю") ? QString("guest") : currentProfile();
    if (!dataDir.exists(profileDirName)) dataDir.mkdir(profileDirName);
}

QString MainWindow::diaryFilePath(const QString& profile, const QDate& date) const {
    const QString profileDirName = (profile == tr("Без профілю")) ? QString("guest") : profile;
    const QString fileName = date.toString("yyyy-MM-dd");
    return QDir(QDir::currentPath()).filePath(QStringLiteral("data/%1/%2").arg(profileDirName, fileName));
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
    const QString profileDirName = (profile == tr("Без профілю")) ? QString("guest") : profile;
    const QString fileName = date.toString("yyyy-MM-dd");
    return QDir(QDir::currentPath()).filePath(QStringLiteral("data/%1/%2").arg(profileDirName, fileName));
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
            QString profileKey = profile.isEmpty() ? tr("Без профілю") : profile;
            
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
    dlg.setWindowTitle(tr("AI-помічник (Offline)"));
    dlg.resize(700, 520);

    auto* lay = new QVBoxLayout(&dlg);
    auto* chat = new QPlainTextEdit(&dlg);
    chat->setReadOnly(true);
    chat->setPlaceholderText(tr("Тут будуть відповіді..."));

    auto* input = new QLineEdit(&dlg);
    input->setPlaceholderText(tr("Поставте питання про харчування або тренування..."));

    auto* btnRow = new QHBoxLayout();
    auto* askOfflineBtn = new QPushButton(tr("Задати (Offline)"), &dlg);
    auto* askOnlineBtn = new QPushButton(tr("Онлайн AI (заглушка)"), &dlg);
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
                             tr("Онлайн AI"),
                             tr("У цьому навчальному проєкті онлайн AI ще не інтегровано. Використовується OfflineAssistant."));
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
