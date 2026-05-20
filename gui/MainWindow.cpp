#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QScrollArea>
#include <QGroupBox>
#include <QPalette>
#include <QApplication>
#include <QClipboard>
#include <QStyle>
#include <QFont>
#include <QFormLayout>
#include <QGridLayout>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDialog>
#include <QFileDialog>
#include <QFile>
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
#include <QSignalBlocker>
#include <algorithm>
#include <cmath>
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
#include <QMenuBar>
#include <QStatusBar>
#include <QDateTime>
#include "LegalDocuments.h"
#include "PerfTrace.h"
#include "TrainingPlanWorkflowDialog.h"
#include "../BatchDiaryLoader.h"
#include "../Parallel.h"
#include "AssistantService.h"
#include "CloudAssistant.h"
#include "AppStyle.h"
#include "ProfileValidation.h"
#include "AppVersion.h"
#include "../Logger.h"
#include <QSettings>
#include <QStringList>

namespace {

void setListPlaceholder(QListWidget* list, const QString& text) {
    auto* item = new QListWidgetItem(text, list);
    item->setFlags(Qt::NoItemFlags);
    QFont f = item->font();
    f.setItalic(true);
    item->setFont(f);
    item->setForeground(list->palette().color(QPalette::PlaceholderText));
}

} // namespace

MainWindow::MainWindow(const QString& loggedInLogin, QWidget* parent)
    : QMainWindow(parent),
      loggedInLogin_(loggedInLogin.trimmed()),
      userDirSlug_(AuthStore::storageFolderForLogin(loggedInLogin_)) {
    setupUi();
}

void MainWindow::setupUi() {
    auto* central = new QWidget(this);
    auto* centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(14, 12, 14, 10);
    centralLayout->setSpacing(12);

    auto* header = new QWidget(central);
    header->setObjectName(QStringLiteral("appHeader"));
    auto* headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(20, 14, 20, 14);
    auto* headerTextCol = new QVBoxLayout();
    headerTextCol->setSpacing(2);
    auto* appTitle = new QLabel(QStringLiteral("CalorieCalc"), header);
    appTitle->setObjectName(QStringLiteral("appTitle"));
    auto* appSubtitle = new QLabel(tr("Харчування · тренування · прогрес"), header);
    appSubtitle->setObjectName(QStringLiteral("appSubtitle"));
    headerTextCol->addWidget(appTitle);
    headerTextCol->addWidget(appSubtitle);
    headerLay->addLayout(headerTextCol);
    headerLay->addStretch();
    headerDateLabel_ = new QLabel(header);
    headerDateLabel_->setObjectName(QStringLiteral("headerDate"));
    headerLay->addWidget(headerDateLabel_);
    centralLayout->addWidget(header);

    auto* appTabs = new QTabWidget(central);
    appTabs->setObjectName(QStringLiteral("mainTabs"));
    auto* caloriesPage = new QWidget(appTabs);
    caloriesPage->setObjectName(QStringLiteral("pageRoot"));
    auto* fitnessPage = new QWidget(appTabs);
    fitnessPage->setObjectName(QStringLiteral("pageRoot"));

    auto* caloriesPageLay = new QVBoxLayout(caloriesPage);
    caloriesPageLay->setContentsMargins(6, 6, 6, 6);
    auto* rootLayout = new QHBoxLayout();
    caloriesPageLay->addLayout(rootLayout);

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

    auto* userAccountLabel = new QLabel(tr("Вітаємо, %1").arg(loggedInLogin_), topBox);
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
        refreshStats();
    });
    connect(heightSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double){
        saveProfileMeta(currentProfile());
        refreshStats();
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
    copyYesterdayBtn_ = new QPushButton(tr("З учора"), rightBox);
    copyYesterdayBtn_->setToolTip(tr("Скопіювати всі прийоми їжі з попереднього дня в поточний"));
    repeatLastMealBtn_ = new QPushButton(tr("Повторити останній"), rightBox);
    repeatLastMealBtn_->setToolTip(tr("Додати ще раз останній записаний прийом їжі"));
    tmplRow->addWidget(saveTemplateBtn_);
    tmplRow->addWidget(applyTemplateBtn_);
    tmplRow->addWidget(copyYesterdayBtn_);
    tmplRow->addWidget(repeatLastMealBtn_);

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

    auto* macroRow = new QHBoxLayout();
    proteinGoalSpin_ = new QSpinBox(goalsBox);
    proteinGoalSpin_->setRange(0, 400);
    proteinGoalSpin_->setValue(150);
    proteinGoalSpin_->setSuffix(tr(" г"));
    carbGoalSpin_ = new QSpinBox(goalsBox);
    carbGoalSpin_->setRange(0, 600);
    carbGoalSpin_->setValue(250);
    carbGoalSpin_->setSuffix(tr(" г"));
    fatGoalSpin_ = new QSpinBox(goalsBox);
    fatGoalSpin_->setRange(0, 200);
    fatGoalSpin_->setValue(70);
    fatGoalSpin_->setSuffix(tr(" г"));
    macroRow->addWidget(new QLabel(tr("Ціль білку:"), goalsBox));
    macroRow->addWidget(proteinGoalSpin_);
    macroRow->addSpacing(8);
    macroRow->addWidget(new QLabel(tr("Вугл.:"), goalsBox));
    macroRow->addWidget(carbGoalSpin_);
    macroRow->addSpacing(8);
    macroRow->addWidget(new QLabel(tr("Жири:"), goalsBox));
    macroRow->addWidget(fatGoalSpin_);
    macroRow->addStretch();
    resetMacroDefaultsBtn_ = new QPushButton(tr("Типові БЖВ"), goalsBox);
    resetMacroDefaultsBtn_->setToolTip(tr("Цілі білок / вуглеводи / жири: 150 / 250 / 70 г"));
    macroRow->addWidget(resetMacroDefaultsBtn_);

    caloriesProgress_ = new QProgressBar(goalsBox);
    caloriesProgress_->setRange(0, 100);

    auto* waterRow = new QHBoxLayout();
    waterAddSpin_ = new QSpinBox(goalsBox);
    waterAddSpin_->setRange(50, 1000);
    waterAddSpin_->setSingleStep(50);
    waterAddSpin_->setValue(250);
    waterAddSpin_->setSuffix(" мл");
    waterAddBtn_ = new QPushButton(tr("Випити"), goalsBox);
    waterQuick250Btn_ = new QPushButton(tr("+250 мл"), goalsBox);
    waterQuick500Btn_ = new QPushButton(tr("+500 мл"), goalsBox);
    waterGoalSpin_ = new QSpinBox(goalsBox);
    waterGoalSpin_->setRange(0, 10000);
    waterGoalSpin_->setSingleStep(100);
    waterGoalSpin_->setValue(2000);
    waterGoalSpin_->setSuffix(" мл");
    waterRow->addWidget(new QLabel(tr("Вода:"), goalsBox));
    waterRow->addWidget(waterAddSpin_);
    waterRow->addWidget(waterAddBtn_);
    waterRow->addWidget(waterQuick250Btn_);
    waterRow->addWidget(waterQuick500Btn_);
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
    bmiLabel_ = new QLabel(goalsBox);
    bmiLabel_->setToolTip(tr("Індекс маси тіла за поточною вагою та зростом зверху"));
    weightRow->addSpacing(12);
    weightRow->addWidget(bmiLabel_);

    caloriesLabel_ = new QLabel(goalsBox);
    caloriesLabel_->setObjectName(QStringLiteral("statsHero"));
    caloriesLabel_->setWordWrap(true);
    macrosLabel_ = new QLabel(goalsBox);
    macrosLabel_->setObjectName(QStringLiteral("mutedHint"));

    auto* macroBarsRow = new QHBoxLayout();
    macroProteinProgress_ = new QProgressBar(goalsBox);
    macroProteinProgress_->setRange(0, 100);
    macroProteinProgress_->setFormat(tr("Б %p%"));
    macroProteinProgress_->setMaximumHeight(22);
    macroCarbProgress_ = new QProgressBar(goalsBox);
    macroCarbProgress_->setRange(0, 100);
    macroCarbProgress_->setFormat(tr("В %p%"));
    macroCarbProgress_->setMaximumHeight(22);
    macroFatProgress_ = new QProgressBar(goalsBox);
    macroFatProgress_->setRange(0, 100);
    macroFatProgress_->setFormat(tr("Ж %p%"));
    macroFatProgress_->setMaximumHeight(22);
    macroBarsRow->addWidget(macroProteinProgress_);
    macroBarsRow->addWidget(macroCarbProgress_);
    macroBarsRow->addWidget(macroFatProgress_);

    goalsLayout->addLayout(calRow);
    goalsLayout->addLayout(macroRow);
    goalsLayout->addWidget(caloriesProgress_);
    goalsLayout->addLayout(waterRow);
    goalsLayout->addWidget(waterProgress_);
    goalsLayout->addLayout(weightRow);
    goalsLayout->addWidget(caloriesLabel_);
    goalsLayout->addWidget(macrosLabel_);
    goalsLayout->addLayout(macroBarsRow);

    // Weekly report and actions
    weeklyReportBtn_ = new QPushButton(tr("Звіт за 7 днів"), rightBox);
    copyDaySummaryBtn_ = new QPushButton(tr("Копіювати підсумок дня"), rightBox);
    copyDaySummaryBtn_->setToolTip(
        tr("Дата, профіль, калорії та макроси, вода, вага, кількість прийомів — у буфер обміну"));

    auto* actionsRow = new QHBoxLayout();
    themeToggleBtn_ = new QPushButton(tr("☀ Світла тема"), rightBox);
    exportCSVBtn_ = new QPushButton(tr("Експорт CSV"), rightBox);
    importCSVBtn_ = new QPushButton(tr("Імпорт CSV"), rightBox);
    exportPDFBtn_ = new QPushButton(tr("Експорт PDF"), rightBox);
    chartsBtn_ = new QPushButton(tr("Графіки"), rightBox);
    assistantBtn_ = new QPushButton(tr("Помічник"), fitnessPage);
    assistantBtn_->setToolTip(tr("Рекомендації на основі щоденника харчування та тренувань."));
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

    generatePlanBtn_ = new QPushButton(tr("План на 7 днів"), trainingBox_);
    generatePlanBtn_->setToolTip(
        tr("Персональний план: перевірка даних, перегляд, корекція та збереження на тиждень."));
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
    editTrainingBtn_ = new QPushButton(tr("Редагувати план"), trainingBox_);
    editTrainingBtn_->setToolTip(tr("Змінити тип, час, тривалість або статус обраної сесії"));
    trainingBtnRow->addWidget(addTrainingBtn_);
    trainingBtnRow->addWidget(editTrainingBtn_);
    trainingBtnRow->addWidget(removeTrainingBtn_);
    trainingLayout->addLayout(trainingBtnRow);

    trainingsList_ = new QListWidget(trainingBox_);
    trainingsList_->setAlternatingRowColors(true);
    trainingLayout->addWidget(trainingsList_);

    trainingSummaryLabel_ = new QLabel(trainingBox_);
    trainingLayout->addWidget(trainingSummaryLabel_);

    rightLayout->addWidget(weeklyReportBtn_);
    rightLayout->addWidget(copyDaySummaryBtn_);
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
    fitnessLayout->setContentsMargins(12, 12, 12, 12);
    fitnessLayout->setSpacing(14);

    auto* fitnessSummaryBox = new QGroupBox(tr("Фітнес-дашборд та прогрес"), fitnessPage);
    fitnessSummaryBox->setObjectName(QStringLiteral("heroCard"));
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

    auto* quickActionsBox = new QGroupBox(tr("Швидкі дії"), fitnessPage);
    auto* trainingQuickActions = new QGridLayout(quickActionsBox);
    trainingQuickActions->setHorizontalSpacing(10);
    trainingQuickActions->setVerticalSpacing(10);
    startWorkoutBtn_ = new QPushButton(tr("Почати тренування"), quickActionsBox);
    quickStartWorkoutBtn_ = new QPushButton(tr("Швидкий старт"), quickActionsBox);
    completeWorkoutBtn_ = new QPushButton(tr("Завершити тренування"), quickActionsBox);
    completeWorkoutBtn_->setToolTip(tr("Завершити активну сесію тренування"));
    markCompletedBtn_ = new QPushButton(tr("Позначити як виконано"), quickActionsBox);
    duplicateTomorrowBtn_ = new QPushButton(tr("Дублювати на завтра"), quickActionsBox);
    trainingWeeklyReportBtn_ = new QPushButton(tr("Звіт тренувань (7 днів)"), quickActionsBox);
    exportTrainingCSVBtn_ = new QPushButton(tr("Експорт тренувань CSV"), quickActionsBox);
    exportTrainingPdfBtn_ = new QPushButton(tr("Експорт тренувань PDF"), quickActionsBox);
    exportTrainingPdfBtn_->setToolTip(tr("Звіт по сесіях за 60 днів (як CSV), у форматі PDF"));
    progressOverviewBtn_ = new QPushButton(tr("Зведення прогресу"), quickActionsBox);
    progressOverviewBtn_->setToolTip(tr("Короткий огляд серії, виконаних сесій і рівня адаптації плану"));
    trainingQuickActions->addWidget(startWorkoutBtn_, 0, 0);
    trainingQuickActions->addWidget(quickStartWorkoutBtn_, 0, 1);
    trainingQuickActions->addWidget(completeWorkoutBtn_, 0, 2);
    trainingQuickActions->addWidget(markCompletedBtn_, 1, 0);
    trainingQuickActions->addWidget(duplicateTomorrowBtn_, 1, 1);
    trainingQuickActions->addWidget(trainingWeeklyReportBtn_, 1, 2);
    trainingQuickActions->addWidget(exportTrainingCSVBtn_, 2, 0);
    trainingQuickActions->addWidget(exportTrainingPdfBtn_, 2, 1);
    trainingQuickActions->addWidget(progressOverviewBtn_, 2, 2);

    fitnessLayout->addWidget(fitnessSummaryBox);
    fitnessLayout->addWidget(trainingBox_, 1);
    fitnessLayout->addWidget(quickActionsBox);
    fitnessLayout->addWidget(assistantBtn_);
    fitnessLayout->addStretch();

    AppStyle::markPrimary(addButton_);
    AppStyle::markPrimary(startWorkoutBtn_);
    AppStyle::markAccent(generatePlanBtn_);
    AppStyle::markAccent(assistantBtn_);
    AppStyle::markAccent(quickStartWorkoutBtn_);
    AppStyle::markDanger(removeButton_);
    AppStyle::markDanger(removeTrainingBtn_);

    appTabs->addTab(caloriesPage, style()->standardIcon(QStyle::SP_FileDialogListView), tr("Харчування"));
    appTabs->addTab(fitnessPage, style()->standardIcon(QStyle::SP_ArrowRight), tr("Фітнес"));

    auto* menuHelp = menuBar()->addMenu(tr("Довідка"));
    menuHelp->addAction(tr("Умови використання…"), this, [this] {
        showLegalDocument(this, LegalDocumentKind::TermsOfService);
    });
    menuHelp->addAction(tr("Політика конфіденційності…"), this, [this] {
        showLegalDocument(this, LegalDocumentKind::PrivacyPolicy);
    });
    menuHelp->addSeparator();
    menuHelp->addAction(tr("Швидкий старт…"), this, [this] {
        QMessageBox::information(
            this,
            tr("Швидкий старт"),
            tr("1) Оберіть дату в календарі.\n"
               "2) Додайте прийоми їжі з пошуку продуктів.\n"
               "3) Налаштуйте цілі калорій і макросів.\n"
               "4) На вкладці «Фітнес» додайте тренування або згенеруйте план на 7 днів.\n"
               "5) Експорт CSV/PDF — у нижній панелі."));
    });
    menuHelp->addAction(tr("Вимоги UR/FR/NFR (зведення)…"), this, &MainWindow::onShowUrFrNfrSummary);
    menuHelp->addSeparator();
    menuHelp->addAction(tr("Продуктивність (локальний MVP)…"), this, [this] {
        QMessageBox::information(
            this,
            tr("Продуктивність"),
            tr("У десктопній версії дані зберігаються локально; типові дії (день, пошук, збереження) розраховані "
               "на швидку відповідь на звичайному ПК. Під час великих експортів інтерфейс періодично оновлюється, "
               "щоб уникнути зависань.\n\n"
               "NFR-1 (відгук ≤2 с): меню «Перевірка швидкості…».\n"
               "NFR-2 (масштаб): REST API + tools/loadtest/load_api.py (roadmap production)."));
    });
    menuHelp->addAction(tr("Перевірка швидкості…"), this, &MainWindow::onNfrSelfCheck);
    menuHelp->addAction(tr("Паралельні обчислення…"), this, &MainWindow::onParallelComputeBenchmark);
    menuHelp->addSeparator();
    menuHelp->addAction(tr("Про CalorieCalc…"), this, &MainWindow::onShowProductAbout);
    menuHelp->addAction(tr("Бачення продукту…"), this, &MainWindow::onShowProductVisionAbout);

    centralLayout->addWidget(appTabs, 1);
    setCentralWidget(central);
    statusBar()->setSizeGripEnabled(true);

    assistantModeBadge_ = new QLabel(this);
    assistantModeBadge_->setObjectName(QStringLiteral("assistantModeBadge"));
    statusBar()->addPermanentWidget(assistantModeBadge_);
    updateAssistantModeIndicator();

    // Signals
    connect(searchBtn, &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(categoryFilter_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::onSearch);
    connect(addButton_, &QPushButton::clicked, this, &MainWindow::onAddFood);
    connect(removeButton_, &QPushButton::clicked, this, &MainWindow::onRemoveMeal);
    connect(goalSpin_, &QSpinBox::valueChanged, this, &MainWindow::onSetGoal);
    connect(proteinGoalSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onSetProteinGoal);
    connect(carbGoalSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onSetCarbGoal);
    connect(fatGoalSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onSetFatGoal);
    connect(customAddButton_, &QPushButton::clicked, this, &MainWindow::onAddCustomFood);
    connect(saveTemplateBtn_, &QPushButton::clicked, this, &MainWindow::onSaveTemplate);
    connect(applyTemplateBtn_, &QPushButton::clicked, this, &MainWindow::onApplyTemplate);
    connect(copyYesterdayBtn_, &QPushButton::clicked, this, &MainWindow::onCopyYesterdayMeals);
    connect(repeatLastMealBtn_, &QPushButton::clicked, this, &MainWindow::onRepeatLastMeal);
    connect(calendar_, &QCalendarWidget::clicked, this, &MainWindow::onDateChanged);
    connect(profileCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::onProfileChanged);
    connect(profileSettingsBtn_, &QPushButton::clicked, this, &MainWindow::onOpenProfileDialog);
    connect(waterAddBtn_, &QPushButton::clicked, this, &MainWindow::onAddWater);
    connect(waterQuick250Btn_, &QPushButton::clicked, this, &MainWindow::onWaterQuick250);
    connect(waterQuick500Btn_, &QPushButton::clicked, this, &MainWindow::onWaterQuick500);
    connect(waterGoalSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onSetWaterGoal);
    connect(weightSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &MainWindow::onSetWeight);
    connect(weeklyReportBtn_, &QPushButton::clicked, this, &MainWindow::onShowWeeklyReport);
    connect(copyDaySummaryBtn_, &QPushButton::clicked, this, &MainWindow::onCopyDaySummaryToClipboard);
    connect(resetMacroDefaultsBtn_, &QPushButton::clicked, this, &MainWindow::onResetMacroGoalsToDefaults);
    connect(themeToggleBtn_, &QPushButton::clicked, this, &MainWindow::onToggleTheme);
    connect(exportCSVBtn_, &QPushButton::clicked, this, &MainWindow::onExportCSV);
    connect(importCSVBtn_, &QPushButton::clicked, this, &MainWindow::onImportCSV);
    connect(exportPDFBtn_, &QPushButton::clicked, this, &MainWindow::onExportPDF);
    connect(chartsBtn_, &QPushButton::clicked, this, &MainWindow::onShowCharts);

    connect(addTrainingBtn_, &QPushButton::clicked, this, &MainWindow::onAddTraining);
    connect(removeTrainingBtn_, &QPushButton::clicked, this, &MainWindow::onRemoveTraining);
    connect(editTrainingBtn_, &QPushButton::clicked, this, &MainWindow::onEditSelectedTraining);
    connect(trainingsList_, &QListWidget::itemDoubleClicked, this, &MainWindow::onEditSelectedTraining);
    connect(generatePlanBtn_, &QPushButton::clicked, this, &MainWindow::onGenerateTrainingPlan7Days);
    connect(assistantBtn_, &QPushButton::clicked, this, &MainWindow::onAskOfflineAssistant);
    connect(markCompletedBtn_, &QPushButton::clicked, this, &MainWindow::onMarkTrainingCompleted);
    connect(duplicateTomorrowBtn_, &QPushButton::clicked, this, &MainWindow::onDuplicateTrainingTomorrow);
    connect(dailyTipBtn_, &QPushButton::clicked, this, &MainWindow::onShowDailyTip);
    connect(trainingWeeklyReportBtn_, &QPushButton::clicked, this, &MainWindow::onShowTrainingWeeklyReport);
    connect(exportTrainingCSVBtn_, &QPushButton::clicked, this, &MainWindow::onExportTrainingCSV);
    connect(exportTrainingPdfBtn_, &QPushButton::clicked, this, &MainWindow::onExportTrainingPDF);
    connect(progressOverviewBtn_, &QPushButton::clicked, this, &MainWindow::onShowProgressOverview);
    connect(startWorkoutBtn_, &QPushButton::clicked, this, &MainWindow::onStartWorkout);
    connect(quickStartWorkoutBtn_, &QPushButton::clicked, this, &MainWindow::onQuickStartWorkout);
    connect(completeWorkoutBtn_, &QPushButton::clicked, this, &MainWindow::onCompleteWorkout);

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

    applyTheme(QSettings().value(QStringLiteral("ui/darkTheme"), true).toBool());
    updateAppChrome();
}

void MainWindow::updateAppChrome() {
    if (headerDateLabel_) {
        headerDateLabel_->setText(
            QLocale().toString(selectedDate_, QLocale::LongFormat));
    }
    if (statusBar()) {
        statusBar()->showMessage(
            tr("Профіль: %1  ·  Обраний день: %2")
                .arg(currentProfile())
                .arg(selectedDate_.toString(QStringLiteral("dd.MM.yyyy"))));
    }
    updateAssistantModeIndicator();
}

void MainWindow::updateAssistantModeIndicator() {
    if (!assistantModeBadge_) return;
    if (CloudAssistant::isConfigured()) {
        assistantModeBadge_->setText(tr("Помічник: хмара + локальний резерв"));
        assistantModeBadge_->setToolTip(
            tr("Задано API-ключ (CALORIECALC_API_KEY або OPENAI_API_KEY). "
               "Відповіді спочатку з хмари; при помилці — локальна логіка."));
    } else {
        assistantModeBadge_->setText(tr("Помічник: лише локально"));
        assistantModeBadge_->setToolTip(
            tr("Без API-ключа використовується лише офлайн-помічник (база продуктів і правила)."));
    }
}

void MainWindow::applyDarkTheme() {
    applyTheme(true);
}

void MainWindow::applyLightTheme() {
    applyTheme(false);
}

void MainWindow::applyTheme(bool dark) {
    isDarkTheme_ = dark;
    if (themeToggleBtn_) {
        themeToggleBtn_->setText(dark ? tr("☀ Світла тема") : tr("🌙 Темна тема"));
    }
    AppStyle::applyTheme(qApp, dark);
    QSettings().setValue(QStringLiteral("ui/darkTheme"), dark);
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
    if (!jsonSaver_.save(diary_, path.toStdString())) {
        LOG_ERROR(std::string("Failed to save diary: ") + path.toStdString());
        if (statusBar()) {
            statusBar()->showMessage(tr("Не вдалося зберегти щоденник харчування"), 6000);
        }
    }
}

QVector<Diary> MainWindow::loadDiariesForDates(const QVector<QDate>& days) const {
    QVector<Diary> result(days.size());
    const QString profile = currentProfile();

    std::vector<BatchDiaryLoader::Job> jobs;
    jobs.reserve(static_cast<size_t>(days.size()));

    for (int i = 0; i < days.size(); ++i) {
        const QDate& day = days[i];
        if (day == selectedDate_) {
            result[i] = diary_;
            continue;
        }
        if (diaries_.contains(profile) && diaries_[profile].contains(day)) {
            result[i] = diaries_[profile].value(day);
            continue;
        }
        BatchDiaryLoader::Job job;
        job.index = static_cast<size_t>(i);
        job.filePath = diaryFilePath(profile, day).toStdString();
        jobs.push_back(std::move(job));
        result[i] = Diary();
        result[i].setCalorieGoal(2000);
    }

    if (!jobs.empty()) {
        const auto loaded = BatchDiaryLoader::loadAll(jobs, jsonSaver_);
        for (const auto& r : loaded) {
            if (r.loaded && r.index < static_cast<size_t>(result.size())) {
                result[static_cast<int>(r.index)] = r.diary;
            }
        }
    }
    return result;
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

void MainWindow::appendAssistantFeedback(const QString& query,
                                         const QString& response,
                                         const QString& verdict,
                                         const QString& comment) {
    auto esc = [](const QString& s) -> QString {
        QString t = s;
        t.replace(QLatin1Char('"'), QLatin1String("\"\""));
        if (t.contains(QLatin1Char(',')) || t.contains(QLatin1Char('\n')) || t.contains(QLatin1Char('\r')))
            return QLatin1Char('"') + t + QLatin1Char('"');
        return t;
    };
    ensureStorageDirs();
    const QString path = QDir(userDataRoot()).filePath(QStringLiteral("pomichnyk_feedback.csv"));
    QFile f(path);
    const bool newFile = !f.exists();
    if (!f.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося записати файл відгуку."));
        return;
    }
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    if (newFile) {
        out << QStringLiteral("timestamp,verdict,query,response,comment\n");
    }
    out << QDateTime::currentDateTimeUtc().toString(Qt::ISODate) << QLatin1Char(',') << esc(verdict) << QLatin1Char(',')
        << esc(query) << QLatin1Char(',') << esc(response) << QLatin1Char(',') << esc(comment) << QLatin1Char('\n');
    f.close();
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
    if (!trainingSaver_.save(trainingDiary_, path.toStdString())) {
        LOG_ERROR(std::string("Failed to save training diary: ") + path.toStdString());
        if (statusBar()) {
            statusBar()->showMessage(tr("Не вдалося зберегти тренування"), 6000);
        }
    }
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
    updateAppChrome();
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
    updateAppChrome();
}

void MainWindow::onSearch() {
    PerfTrace::begin(QStringLiteral("food_search"));
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
            results = foodDb_.searchFoodsParallel(query.toStdString());
        }
    }
    
    if (results.empty()) {
        const QString hint = query.isEmpty()
                                 ? tr("Уведіть назву продукту або оберіть категорію.")
                                 : tr("Нічого не знайдено. Спробуйте інший запит або додайте свій продукт нижче.");
        setListPlaceholder(resultsList_, hint);
    } else {
        for (const auto& food : results) {
            QString itemText = QString::fromStdString(food.getName()) + " [" +
                               QString::fromStdString(food.getCategory()) + "] — " +
                               QString::number(food.getCalories(), 'f', 0) + tr(" ккал/100г");
            auto* item = new QListWidgetItem(itemText, resultsList_);
            item->setData(Qt::UserRole, QString::fromStdString(food.getName()));
        }
    }
    const qint64 ms = PerfTrace::endMs(QStringLiteral("food_search"));
    if (ms >= 0 && ms > 2000) {
        statusBar()->showMessage(tr("Пошук зайняв %1 мс (довше за 2 с)").arg(ms), 5000);
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

void MainWindow::onSetProteinGoal(int value) {
    diary_.setProteinGoalG(static_cast<double>(value));
    refreshStats();
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onSetCarbGoal(int value) {
    diary_.setCarbGoalG(static_cast<double>(value));
    refreshStats();
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onSetFatGoal(int value) {
    diary_.setFatGoalG(static_cast<double>(value));
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
    refreshStats();
}

void MainWindow::onOpenProfileDialog() {
    ProfileDialog dlg(loggedInLogin_, this);
    dlg.setProfileValues(weightSpin_->value(), ageSpin_->value(), heightSpin_->value(), activityCombo_->currentIndex());
    if (dlg.exec() != QDialog::Accepted) return;
    if (dlg.accountWasDeleted()) {
        QApplication::quit();
        return;
    }

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
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Звіт за 7 днів"));
    auto* lay = new QVBoxLayout(&dlg);
    auto* list = new QListWidget(&dlg);
    lay->addWidget(list);

    QVector<QDate> days;
    days.reserve(7);
    for (int i = 6; i >= 0; --i) {
        days.append(selectedDate_.addDays(-i));
    }
    const QVector<Diary> loaded = loadDiariesForDates(days);

    double sumCalories = 0.0;
    for (int i = 0; i < days.size(); ++i) {
        const QDate& day = days[i];
        const Diary& d = loaded[i];
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

void MainWindow::onShowProgressOverview() {
    const int streak = calculateConsistencyStreak();
    const int completed = countCompletedTrainingSessionsLast7Days();
    const int adapt = trainingAdaptationVolume();
    QString adaptText;
    if (adapt <= -1) adaptText = tr("легший тиждень (−5 хв до робочих сесій у наступному плані)");
    else if (adapt == 0) adaptText = tr("стандартна тривалість");
    else if (adapt == 1) adaptText = tr("легкий прогрес (+5 хв до робочих сесій)");
    else adaptText = tr("помірний прогрес (+10 хв до робочих сесій)");

    QMessageBox::information(
        this,
        tr("Зведення прогресу"),
        tr("Серія активності (харчування / тренування): %1 дн.\n"
           "Виконано тренувальних сесій за 7 днів (включно з обраною датою назад): %2\n"
           "Наступний план (автогенерація): %3\n\n"
           "Детальніше: «Звіт за 7 днів», «Звіт тренувань (7 днів)», «Графіки».")
            .arg(streak)
            .arg(completed)
            .arg(adaptText));
}

void MainWindow::onShowProductAbout() {
    QMessageBox::about(
        this,
        tr("Про CalorieCalc"),
        tr("<h3>CalorieCalc %1</h3>"
           "<p>Щоденник харчування та тренувань на вашому комп’ютері. "
           "Дані зберігаються локально.</p>"
           "<p><b>Можливості:</b> калорії й макроси, вода, вага, план тренувань, "
           "сесії «Почати тренування», звіти та експорт PDF/CSV, помічник.</p>"
           "<p>© %2 CalorieCalc</p>")
            .arg(AppVersion::versionString())
            .arg(QString::fromUtf8(AppVersion::kBuildYear)));
}

void MainWindow::onShowProductVisionAbout() {
    QMessageBox::information(
        this,
        tr("CalorieCalc — бачення продукту"),
        tr("Персоналізований облік харчування та тренувань: цілі, план на тиждень "
           "(генератор з адаптацією тривалості), щоденники, звіти та графіки.\n\n"
           "Аудиторія (бізнес-аналіз):\n"
           "  • новачки — швидкий старт і автоплан;\n"
           "  • активні користувачі — статистика й графіки;\n"
           "  • спортсмени — автоматизація плану та помічник.\n\n"
           "Мобільна хмарна синхронізація — roadmap (див. docs/Vision_CalorieCalc.md)."));
}

void MainWindow::onShowUrFrNfrSummary() {
    QMessageBox::information(
        this,
        tr("Вимоги UR/FR/NFR"),
        tr("<b>Функціональні</b>\n"
           "UR-1 Профіль — реєстрація, вік/вага/зріст, ціль (FR-1.1, FR-1.2)\n"
           "UR-2 План тренувань — генерація, редагування (FR-2.1, FR-2.2)\n"
           "UR-3 Калорії — журнал продуктів, підрахунок за день (FR-3.1, FR-3.2)\n"
           "UR-4 Помічник — питання та рекомендації (FR-4.1, FR-4.2)\n"
           "UR-5 Прогрес — звіти, графіки (FR-5.1, FR-5.2)\n\n"
           "<b>Нефункціональні</b>\n"
           "NFR-1 Відгук ≤2 с — «Перевірка швидкості…»\n"
           "NFR-2 Масштаб — REST API + load test (roadmap)\n"
           "NFR-4 Захист ПД — згода, політика, видалення акаунта\n\n"
           "Детально: docs/Requirements_UR_FR_NFR.md, RequirementsTraceability.md"));
}

void MainWindow::onNfrSelfCheck() {
    PerfTrace::begin(QStringLiteral("nfr_food_search"));
    (void)foodDb_.searchFoodsParallel("кур");
    const qint64 searchMs = PerfTrace::endMs(QStringLiteral("nfr_food_search"));

    PerfTrace::begin(QStringLiteral("nfr_week_plan"));
    TrainingPlanGenerator gen;
    const TrainingPreferences prefs = buildTrainingPreferences();
    (void)gen.generateWeekParallel(prefs);
    const qint64 planMs = PerfTrace::endMs(QStringLiteral("nfr_week_plan"));

    const bool searchOk = searchMs >= 0 && searchMs <= 2000;
    const bool planOk = planMs >= 0 && planMs <= 2000;

    QMessageBox::information(
        this,
        tr("Перевірка швидкості"),
        tr("Час типових операцій (орієнтир NFR-1 — до 2 с):\n"
           "  • Пошук продуктів (паралельно): %1 мс — %2\n"
           "  • План на тиждень (паралельно): %3 мс — %4\n\n"
           "Детальне порівняння потоків: меню «Паралельні обчислення…».")
            .arg(searchMs)
            .arg(searchOk ? tr("OK") : tr("перевищено"))
            .arg(planMs)
            .arg(planOk ? tr("OK") : tr("перевищено")));
}

void MainWindow::onParallelComputeBenchmark() {
    const unsigned hw = std::max(1u, Parallel::threadCount(0, 64));

    PerfTrace::begin(QStringLiteral("bench_search_seq"));
    (void)foodDb_.searchFoods("а");
    const qint64 searchSeq = PerfTrace::endMs(QStringLiteral("bench_search_seq"));

    PerfTrace::begin(QStringLiteral("bench_search_par"));
    (void)foodDb_.searchFoodsParallel("а");
    const qint64 searchPar = PerfTrace::endMs(QStringLiteral("bench_search_par"));

    QVector<QDate> days;
    days.reserve(30);
    for (int i = 29; i >= 0; --i) {
        days.append(selectedDate_.addDays(-i));
    }

    PerfTrace::begin(QStringLiteral("bench_diary_seq"));
    for (const QDate& day : days) {
        if (day == selectedDate_) {
            continue;
        }
        Diary d;
        d.setCalorieGoal(2000);
        jsonSaver_.load(d, diaryFilePath(currentProfile(), day).toStdString());
    }
    const qint64 diarySeq = PerfTrace::endMs(QStringLiteral("bench_diary_seq"));

    PerfTrace::begin(QStringLiteral("bench_diary_par"));
    (void)loadDiariesForDates(days);
    const qint64 diaryPar = PerfTrace::endMs(QStringLiteral("bench_diary_par"));

    TrainingPlanGenerator gen;
    const TrainingPreferences prefs = buildTrainingPreferences();

    PerfTrace::begin(QStringLiteral("bench_plan_seq"));
    (void)gen.generateWeek(prefs);
    const qint64 planSeq = PerfTrace::endMs(QStringLiteral("bench_plan_seq"));

    PerfTrace::begin(QStringLiteral("bench_plan_par"));
    (void)gen.generateWeekParallel(prefs);
    const qint64 planPar = PerfTrace::endMs(QStringLiteral("bench_plan_par"));

    QMessageBox::information(
        this,
        tr("Паралельні обчислення"),
        tr("Потоків (авто): %1\n\n"
           "Пошук «а»:\n"
           "  • послідовно: %2 мс\n"
           "  • паралельно: %3 мс\n\n"
           "Завантаження 30 днів щоденника:\n"
           "  • послідовно: %4 мс\n"
           "  • паралельно: %5 мс\n\n"
           "План на 7 днів:\n"
           "  • послідовно: %6 мс\n"
           "  • паралельно: %7 мс\n\n"
           "Реалізація: std::async, BatchDiaryLoader, searchFoodsParallel.")
            .arg(hw)
            .arg(searchSeq)
            .arg(searchPar)
            .arg(diarySeq)
            .arg(diaryPar)
            .arg(planSeq)
            .arg(planPar));
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
    int dayCounter = 0;
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
        if ((++dayCounter % 8) == 0) QApplication::processEvents();
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

void MainWindow::onExportTrainingCSV() {
    const QString filename =
        QFileDialog::getSaveFileName(this, tr("Експорт тренувань CSV"), QString(), tr("CSV (*.csv)"));
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Помилка"), tr("Не вдалося відкрити файл для запису."));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "Дата,Профіль,Тип,Тривалість_хв,Час,Статус,Нотатки\n";

    const QString profile = currentProfile();
    const QDate startDate = selectedDate_.addDays(-60);
    int dayCounter = 0;
    for (QDate d = startDate; d <= selectedDate_; d = d.addDays(1)) {
        TrainingDiary td;
        if (trainings_.contains(profile) && trainings_[profile].contains(d)) {
            td = trainings_[profile][d];
        } else {
            trainingSaver_.load(td, trainingFilePath(profile, d).toStdString());
        }
        for (const auto& s : td.getAllSessions()) {
            const QString status = QString::fromStdString(TrainingSession::statusToString(s.getStatus()));
            QString notes = QString::fromStdString(s.getNotes());
            notes.replace(QLatin1Char('"'), QLatin1String("\"\""));
            if (notes.contains(QLatin1Char(',')) || notes.contains(QLatin1Char('\n')) || notes.contains(QLatin1Char('\r'))) {
                notes = QLatin1Char('"') + notes + QLatin1Char('"');
            }
            out << d.toString(QStringLiteral("yyyy-MM-dd")) << QLatin1Char(',')
                << profile << QLatin1Char(',')
                << QString::fromStdString(s.getType()) << QLatin1Char(',')
                << s.getDurationMin() << QLatin1Char(',')
                << QString::fromStdString(s.getTimeHHmm()) << QLatin1Char(',')
                << status << QLatin1Char(',')
                << notes << QLatin1Char('\n');
        }
        if ((++dayCounter % 14) == 0) QApplication::processEvents();
    }

    file.close();
    QMessageBox::information(this, tr("Готово"), tr("Тренування експортовано у CSV (до 60 днів назад)."));
}

void MainWindow::onExportTrainingPDF() {
    const QString filename =
        QFileDialog::getSaveFileName(this, tr("Експорт тренувань PDF"), QString(), tr("PDF (*.pdf)"));
    if (filename.isEmpty()) return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename);
    printer.setPageSize(QPageSize::A4);

    QPainter painter(&printer);
    painter.setFont(QFont(QStringLiteral("Arial"), 11));

    int y = 50;
    const QString profile = currentProfile();
    painter.drawText(50, y, tr("Тренування — звіт (до 60 днів назад)"));
    y += 24;
    painter.drawText(50, y, tr("Профіль: %1, остання дата у звіті: %2").arg(profile).arg(selectedDate_.toString(QStringLiteral("yyyy-MM-dd"))));
    y += 36;

    auto typeUa = [this](const std::string& t) -> QString {
        if (t == "Strength") return tr("Сила");
        if (t == "Cardio") return tr("Кардіо");
        if (t == "Mobility") return tr("Мобільність");
        return QString::fromStdString(t);
    };

    const QDate startDate = selectedDate_.addDays(-60);
    int dayCounter = 0;
    bool anySession = false;
    for (QDate d = startDate; d <= selectedDate_; d = d.addDays(1)) {
        TrainingDiary td;
        if (trainings_.contains(profile) && trainings_[profile].contains(d)) {
            td = trainings_[profile][d];
        } else {
            trainingSaver_.load(td, trainingFilePath(profile, d).toStdString());
        }
        for (const auto& s : td.getAllSessions()) {
            anySession = true;
            const QString status = QString::fromStdString(TrainingSession::statusToString(s.getStatus()));
            const QString line = d.toString(QStringLiteral("yyyy-MM-dd")) + QLatin1String(" | ")
                + typeUa(s.getType()) + QLatin1String(" | ")
                + QString::fromStdString(s.getTimeHHmm()) + QLatin1String(" | ")
                + QString::number(s.getDurationMin()) + tr(" хв | ") + status + QLatin1String(" | ")
                + QString::fromStdString(s.getNotes());
            painter.drawText(50, y, line);
            y += 18;
            if (y > printer.pageRect(QPrinter::DevicePixel).height() - 60) {
                printer.newPage();
                y = 50;
            }
        }
        if ((++dayCounter % 14) == 0) QApplication::processEvents();
    }

    if (!anySession) {
        painter.drawText(50, y, tr("(Немає записів тренувань за цей період.)"));
    }

    painter.end();
    QMessageBox::information(this, tr("Готово"), tr("PDF з тренуваннями створено."));
}

void MainWindow::onCopyYesterdayMeals() {
    const QDate yesterday = selectedDate_.addDays(-1);
    const QString profile = currentProfile();
    Diary src;
    if (diaries_.contains(profile) && diaries_[profile].contains(yesterday)) {
        src = diaries_[profile][yesterday];
    } else if (!jsonSaver_.load(src, diaryFilePath(profile, yesterday).toStdString())) {
        QMessageBox::information(this, tr("Немає даних"), tr("За вчора немає збереженого щоденника."));
        return;
    }
    if (src.getMealsCount() == 0) {
        QMessageBox::information(this, tr("Порожньо"), tr("За вчора немає прийомів їжі для копіювання."));
        return;
    }
    if (QMessageBox::question(this, tr("Копіювання"),
                              tr("Додати %1 прийом(ів) з %2 до поточного дня?")
                                  .arg(static_cast<int>(src.getMealsCount()))
                                  .arg(yesterday.toString(QStringLiteral("yyyy-MM-dd"))))
        != QMessageBox::Yes) {
        return;
    }
    for (const auto& m : src.getAllMeals()) {
        diary_.addMeal(m);
    }
    refreshDiary();
    refreshStats();
    saveDiaryFor(profile, selectedDate_);
    QMessageBox::information(this, tr("Готово"), tr("Прийоми скопійовано."));
}

void MainWindow::onRepeatLastMeal() {
    if (diary_.getMealsCount() == 0) {
        QMessageBox::information(this, tr("Немає"), tr("Щоденник порожній — немає останнього прийому."));
        return;
    }
    const auto meals = diary_.getAllMeals();
    diary_.addMeal(meals.back());
    refreshDiary();
    refreshStats();
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onWaterQuick250() {
    diary_.addWater(250);
    refreshStats();
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onWaterQuick500() {
    diary_.addWater(500);
    refreshStats();
    saveDiaryFor(currentProfile(), selectedDate_);
}

void MainWindow::onCopyDaySummaryToClipboard() {
    QString t;
    t += tr("Дата: %1\n").arg(selectedDate_.toString(QStringLiteral("yyyy-MM-dd")));
    t += tr("Профіль: %1\n").arg(currentProfile());
    t += tr("Калорії: %1 / %2 ккал\n")
             .arg(QString::number(diary_.getTotalCalories(), 'f', 0))
             .arg(QString::number(diary_.getCalorieGoal(), 'f', 0));
    t += tr("Білок: %1 / %2 г\n")
             .arg(QString::number(diary_.getTotalProtein(), 'f', 1))
             .arg(QString::number(diary_.getProteinGoalG(), 'f', 0));
    t += tr("Вуглеводи: %1 / %2 г\n")
             .arg(QString::number(diary_.getTotalCarbs(), 'f', 1))
             .arg(QString::number(diary_.getCarbGoalG(), 'f', 0));
    t += tr("Жири: %1 / %2 г\n")
             .arg(QString::number(diary_.getTotalFat(), 'f', 1))
             .arg(QString::number(diary_.getFatGoalG(), 'f', 0));
    t += tr("Вода: %1 / %2 мл\n").arg(diary_.getWaterMl()).arg(diary_.getWaterGoalMl());
    t += tr("Вага: %1 кг\n").arg(QString::number(diary_.getWeightKg(), 'f', 1));
    t += tr("Прийомів їжі: %1\n").arg(static_cast<int>(diary_.getMealsCount()));

    QApplication::clipboard()->setText(t.trimmed());
    statusBar()->showMessage(tr("Підсумок дня скопійовано в буфер обміну"), 4000);
}

void MainWindow::onResetMacroGoalsToDefaults() {
    diary_.setProteinGoalG(150.0);
    diary_.setCarbGoalG(250.0);
    diary_.setFatGoalG(70.0);
    const QSignalBlocker bp(*proteinGoalSpin_);
    const QSignalBlocker bc(*carbGoalSpin_);
    const QSignalBlocker bf(*fatGoalSpin_);
    proteinGoalSpin_->setValue(150);
    carbGoalSpin_->setValue(250);
    fatGoalSpin_->setValue(70);
    refreshStats();
    saveDiaryFor(currentProfile(), selectedDate_);
    statusBar()->showMessage(tr("Цілі БЖВ: 150 / 250 / 70 г"), 3000);
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
    
#ifdef QT_CHARTS_LIB
    QVector<QDate> days30;
    days30.reserve(30);
    for (int i = 29; i >= 0; --i) {
        days30.append(selectedDate_.addDays(-i));
    }
    const QVector<Diary> diaries30 = loadDiariesForDates(days30);

    auto* tabWidget = new QTabWidget(&chartDlg);
    
    // Calories chart (7 days)
    QChart* caloriesChart = new QChart();
    QBarSeries* caloriesSeries = new QBarSeries();
    QBarSet* caloriesSet = new QBarSet(tr("Калорії"));
    
    QStringList categories;
    for (int i = 6; i >= 0; --i) {
        const QDate day = selectedDate_.addDays(-i);
        const Diary& d = diaries30[29 - i];
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
        const Diary& d = diaries30[29 - i];
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
        const QDate day = selectedDate_.addDays(-i);
        const Diary& d = diaries30[29 - i];
        const double weight = d.getWeightKg();
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
    auto* warn = new QLabel(
        tr("Графіки недоступні в цій збірці.\n\n"
           "Встановіть повну версію CalorieCalc з підтримкою графіків або зверніться до підтримки."),
        &chartDlg);
    warn->setWordWrap(true);
    layout->addWidget(warn);
#endif
    chartDlg.exec();
}

void MainWindow::refreshDiary() {
    QStringList breakfastLines;
    QStringList lunchLines;
    QStringList dinnerLines;
    QStringList snackLines;

    const auto meals = diary_.getAllMeals();
    for (const auto& m : meals) {
        const auto& f = m.getFood();
        const QString line = QString::fromStdString(f.getName()) + " — "
            + QString::number(f.getAmount(), 'f', 0) + " г, "
            + QString::number(m.getTotalCalories(), 'f', 0) + " ккал";
        const QString mealName = QString::fromStdString(m.getMealName());
        if (mealName == tr("Сніданок")) {
            breakfastLines << line;
        } else if (mealName == tr("Обід")) {
            lunchLines << line;
        } else if (mealName == tr("Вечеря")) {
            dinnerLines << line;
        } else {
            snackLines << line;
        }
    }

    const auto fillMealList = [](QListWidget* list, const QStringList& lines, const QString& emptyHint) {
        list->clear();
        if (lines.isEmpty()) {
            setListPlaceholder(list, emptyHint);
        } else {
            for (const QString& line : lines) {
                list->addItem(line);
            }
        }
    };

    fillMealList(breakfastList_, breakfastLines, tr("Поки немає записів — додайте продукт з пошуку."));
    fillMealList(lunchList_, lunchLines, tr("Поки немає записів — додайте продукт з пошуку."));
    fillMealList(dinnerList_, dinnerLines, tr("Поки немає записів — додайте продукт з пошуку."));
    fillMealList(snackList_, snackLines, tr("Поки немає записів — додайте продукт з пошуку."));
}

void MainWindow::refreshStats() {
    QSignalBlocker bGoal(*goalSpin_);
    QSignalBlocker bWaterGoal(*waterGoalSpin_);
    QSignalBlocker bProt(*proteinGoalSpin_);
    QSignalBlocker bCarb(*carbGoalSpin_);
    QSignalBlocker bFat(*fatGoalSpin_);

    goalSpin_->setValue(static_cast<int>(diary_.getCalorieGoal()));
    waterGoalSpin_->setValue(diary_.getWaterGoalMl());
    proteinGoalSpin_->setValue(static_cast<int>(std::lround(diary_.getProteinGoalG())));
    carbGoalSpin_->setValue(static_cast<int>(std::lround(diary_.getCarbGoalG())));
    fatGoalSpin_->setValue(static_cast<int>(std::lround(diary_.getFatGoalG())));

    const double eaten = diary_.getTotalCalories();
    const double remaining = diary_.getRemainingCalories();

    caloriesLabel_->setText(
        tr("Калорії: з'їдено %1 ккал / ціль %2 ккал / залишок %3 ккал")
            .arg(QString::number(eaten, 'f', 0))
            .arg(QString::number(diary_.getCalorieGoal(), 'f', 0))
            .arg(QString::number(remaining, 'f', 0))
    );

    macrosLabel_->setText(
        tr("Б: %1 / %2 г  |  Ж: %3 / %4 г  |  В: %5 / %6 г")
            .arg(QString::number(diary_.getTotalProtein(), 'f', 1))
            .arg(QString::number(diary_.getProteinGoalG(), 'f', 0))
            .arg(QString::number(diary_.getTotalFat(), 'f', 1))
            .arg(QString::number(diary_.getFatGoalG(), 'f', 0))
            .arg(QString::number(diary_.getTotalCarbs(), 'f', 1))
            .arg(QString::number(diary_.getCarbGoalG(), 'f', 0))
    );

    // Progress bars
    const int calProgress = diary_.getCalorieGoal() > 0
        ? static_cast<int>((eaten / diary_.getCalorieGoal()) * 100.0)
        : 0;
    caloriesProgress_->setValue(std::clamp(calProgress, 0, 100));

    const int waterProgress = diary_.getWaterGoalMl() > 0
        ? static_cast<int>((static_cast<double>(diary_.getWaterMl()) / diary_.getWaterGoalMl()) * 100.0)
        : 0;
    waterProgress_->setValue(std::clamp(waterProgress, 0, 100));

    auto macroPct = [](double eaten, double goal) -> int {
        if (goal <= 0.0) return 0;
        return std::clamp(static_cast<int>((eaten / goal) * 100.0), 0, 100);
    };
    macroProteinProgress_->setValue(macroPct(diary_.getTotalProtein(), diary_.getProteinGoalG()));
    macroCarbProgress_->setValue(macroPct(diary_.getTotalCarbs(), diary_.getCarbGoalG()));
    macroFatProgress_->setValue(macroPct(diary_.getTotalFat(), diary_.getFatGoalG()));

    const double hCm = heightSpin_->value();
    const double wKg = diary_.getWeightKg();
    if (hCm > 0.0 && wKg > 0.0) {
        const double m = hCm / 100.0;
        const double bmi = wKg / (m * m);
        bmiLabel_->setText(tr("ІМТ: %1").arg(QString::number(bmi, 'f', 1)));
    } else {
        bmiLabel_->setText(tr("ІМТ: —"));
    }
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

    if (sessions.empty()) {
        setListPlaceholder(trainingsList_, tr("Немає тренувань на цей день. Додайте сесію або згенеруйте план."));
    }

    trainingSummaryLabel_->setText(
        tr("Тренування сьогодні: %1 хв").arg(QString::number(trainingDiary_.getTotalDurationMin())));

    const int streak = calculateConsistencyStreak();
    fitnessKpiLabel_->setText(
        tr("Серія активності: %1 дн.  |  Сесій сьогодні: %2")
            .arg(streak)
            .arg(static_cast<int>(sessions.size())));
}

int MainWindow::calculateConsistencyStreak() const {
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

int MainWindow::countCompletedTrainingSessionsLast7Days() const {
    const QString profile = currentProfile();
    int n = 0;
    for (int i = 0; i < 7; ++i) {
        const QDate day = selectedDate_.addDays(-i);
        TrainingDiary td;
        if (trainings_.contains(profile) && trainings_[profile].contains(day)) {
            td = trainings_[profile][day];
        } else {
            trainingSaver_.load(td, trainingFilePath(profile, day).toStdString());
        }
        for (const auto& s : td.getAllSessions()) {
            if (s.getStatus() == TrainingSession::Status::Completed) ++n;
        }
    }
    return n;
}

int MainWindow::trainingAdaptationVolume() const {
    const int completed = countCompletedTrainingSessionsLast7Days();
    if (completed >= 6) return 2;
    if (completed >= 3) return 1;
    if (completed == 0) return -1;
    return 0;
}

TrainingPreferences MainWindow::buildTrainingPreferences() const {
    TrainingPreferences prefs;
    const QString goalText = trainingGoalCombo_ ? trainingGoalCombo_->currentText() : QString();
    if (goalText == tr("Схуднення")) prefs.goal = "cutting";
    else if (goalText == tr("Набір")) prefs.goal = "bulk";
    else prefs.goal = "maintenance";
    prefs.adaptationVolume = trainingAdaptationVolume();
    if (ageSpin_) prefs.ageYears = ageSpin_->value();
    if (heightSpin_) prefs.heightCm = heightSpin_->value();
    const double w = weightSpin_ ? weightSpin_->value() : diary_.getWeightKg();
    if (w > 0.0) prefs.weightKg = w;
    return prefs;
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

void MainWindow::onEditSelectedTraining() {
    auto* item = trainingsList_->currentItem();
    if (!item) {
        QMessageBox::information(this, tr("Редагування"), tr("Оберіть сесію в списку тренувань."));
        return;
    }
    const int idx = item->data(Qt::UserRole).toInt();
    auto sessions = trainingDiary_.getAllSessions();
    if (idx < 0 || idx >= static_cast<int>(sessions.size())) return;

    const auto& cur = sessions[static_cast<size_t>(idx)];

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Редагувати сесію"));
    auto* form = new QFormLayout(&dlg);

    auto* typeCombo = new QComboBox(&dlg);
    typeCombo->addItems({tr("Сила"), tr("Кардіо"), tr("Мобільність"), tr("Відпочинок")});
    const QString curType = QString::fromStdString(cur.getType());
    if (curType == QLatin1String("Strength")) typeCombo->setCurrentIndex(0);
    else if (curType == QLatin1String("Cardio")) typeCombo->setCurrentIndex(1);
    else if (curType == QLatin1String("Mobility")) typeCombo->setCurrentIndex(2);
    else typeCombo->setCurrentIndex(3);

    auto* timeEdit = new QTimeEdit(&dlg);
    timeEdit->setDisplayFormat(QStringLiteral("HH:mm"));
    const QTime parsed = QTime::fromString(QString::fromStdString(cur.getTimeHHmm()), QStringLiteral("HH:mm"));
    timeEdit->setTime(parsed.isValid() ? parsed : QTime(7, 30));

    auto* durSpin = new QSpinBox(&dlg);
    durSpin->setRange(0, 180);
    durSpin->setSuffix(tr(" хв"));
    durSpin->setValue(cur.getDurationMin());

    auto* statusCombo = new QComboBox(&dlg);
    statusCombo->addItems({tr("План"), tr("Виконано")});
    statusCombo->setCurrentIndex(cur.getStatus() == TrainingSession::Status::Completed ? 1 : 0);

    auto* notesEdit = new QLineEdit(&dlg);
    notesEdit->setText(QString::fromStdString(cur.getNotes()));

    form->addRow(tr("Тип:"), typeCombo);
    form->addRow(tr("Час:"), timeEdit);
    form->addRow(tr("Тривалість:"), durSpin);
    form->addRow(tr("Статус:"), statusCombo);
    form->addRow(tr("Нотатки:"), notesEdit);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dlg);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    if (dlg.exec() != QDialog::Accepted) return;

    QString typeStr;
    switch (typeCombo->currentIndex()) {
        case 0: typeStr = QStringLiteral("Strength"); break;
        case 1: typeStr = QStringLiteral("Cardio"); break;
        case 2: typeStr = QStringLiteral("Mobility"); break;
        default: typeStr = QStringLiteral("Rest"); break;
    }
    const int duration = durSpin->value();
    if (duration <= 0 && typeStr != QLatin1String("Rest")) {
        QMessageBox::warning(this, tr("Редагування"), tr("Вкажіть тривалість більше 0 хв."));
        return;
    }
    const auto status = statusCombo->currentIndex() == 1 ? TrainingSession::Status::Completed
                                                         : TrainingSession::Status::Planned;
    trainingDiary_.replaceSession(
        idx,
        TrainingSession(typeStr.toStdString(),
                        duration,
                        timeEdit->time().toString(QStringLiteral("HH:mm")).toStdString(),
                        status,
                        notesEdit->text().trimmed().toStdString()));
    refreshTraining();
    saveTrainingFor(currentProfile(), selectedDate_);
}

void MainWindow::onGenerateTrainingPlan7Days() {
    TrainingPlanWorkflowDialog::Context ctx;
    ctx.profile.ageYears = ageSpin_ ? ageSpin_->value() : 30;
    ctx.profile.heightCm = heightSpin_ ? heightSpin_->value() : 175.0;
    ctx.profile.weightKg = weightSpin_ ? weightSpin_->value() : diary_.getWeightKg();
    if (ctx.profile.weightKg <= 0.0) ctx.profile.weightKg = 70.0;
    if (trainingGoalCombo_) {
        const QString g = trainingGoalCombo_->currentText();
        if (g == tr("Схуднення")) ctx.profile.goalIndex = 0;
        else if (g == tr("Набір")) ctx.profile.goalIndex = 1;
        else ctx.profile.goalIndex = 2;
    }
    ctx.startDate = selectedDate_;
    ctx.adaptationVolume = trainingAdaptationVolume();
    ctx.diary = &diary_;
    ctx.trainingToday = &trainingDiary_;

    const QString profile = currentProfile();
    auto saveFn = [this, profile](const QMap<QDate, TrainingDiary>& plan) -> bool {
        ensureStorageDirs();
        for (auto it = plan.cbegin(); it != plan.cend(); ++it) {
            const QDate& day = it.key();
            const TrainingDiary& td = it.value();
            trainings_[profile][day] = td;
            const QString path = trainingFilePath(profile, day);
            if (!trainingSaver_.save(td, path.toStdString())) return false;
        }
        if (plan.contains(selectedDate_)) trainingDiary_ = plan[selectedDate_];
        return true;
    };

    TrainingPlanWorkflowDialog dlg(ctx, saveFn, this);
    const int rc = dlg.exec();
    if (rc != QDialog::Accepted && !dlg.planActivated()) return;

    const auto prof = dlg.profileResult();
    if (ageSpin_) ageSpin_->setValue(prof.ageYears);
    if (heightSpin_) heightSpin_->setValue(prof.heightCm);
    if (weightSpin_) weightSpin_->setValue(prof.weightKg);
    if (trainingGoalCombo_) trainingGoalCombo_->setCurrentIndex(prof.goalIndex);
    diary_.setWeightKg(prof.weightKg);
    saveProfileMeta(profile);
    saveDiaryFor(profile, selectedDate_);

    loadTrainingFor(profile, selectedDate_);
    refreshTraining();
    refreshStats();
}

void MainWindow::onAskOfflineAssistant() {
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Помічник"));
    dlg.resize(720, 580);

    QString lastQuery;
    QString lastResponse;

    auto* lay = new QVBoxLayout(&dlg);
    auto* chat = new QPlainTextEdit(&dlg);
    chat->setReadOnly(true);
    chat->setPlaceholderText(tr("Тут будуть відповіді..."));

    auto* input = new QLineEdit(&dlg);
    input->setPlaceholderText(tr("Поставте питання про харчування або тренування..."));

    auto* askBtn = new QPushButton(tr("Задати питання"), &dlg);
    auto* modeLabel = new QLabel(&dlg);
    modeLabel->setWordWrap(true);
    if (CloudAssistant::isConfigured()) {
        modeLabel->setText(tr("Режим: хмарна модель (API-ключ знайдено) + локальний резерв."));
    } else {
        modeLabel->setText(
            tr("Режим: локальна база продуктів і правила. Для хмарної моделі задайте "
               "CALORIECALC_API_KEY або OPENAI_API_KEY і перезапустіть програму."));
    }

    auto* feedbackLabel = new QLabel(tr("Оцінка та корекція останньої поради (для аналізу якості рекомендацій):"), &dlg);
    auto* verdictCombo = new QComboBox(&dlg);
    verdictCombo->addItem(tr("— оберіть оцінку —"), QString());
    verdictCombo->addItem(tr("Корисно"), QStringLiteral("helpful"));
    verdictCombo->addItem(tr("Не підійшло / потребує корекції"), QStringLiteral("reject"));
    auto* feedbackComment = new QLineEdit(&dlg);
    feedbackComment->setPlaceholderText(tr("Що саме змінити або уточнити? (необов’язково)"));
    auto* saveFeedbackBtn = new QPushButton(tr("Зберегти відгук для останньої відповіді"), &dlg);

    lay->addWidget(chat, 1);
    lay->addWidget(input);
    lay->addWidget(askBtn);
    lay->addWidget(modeLabel);
    lay->addWidget(feedbackLabel);
    lay->addWidget(verdictCombo);
    lay->addWidget(feedbackComment);
    lay->addWidget(saveFeedbackBtn);

    connect(askBtn, &QPushButton::clicked, [&, this]() {
        const QString q = input->text().trimmed();
        if (q.isEmpty()) return;

        chat->appendPlainText(tr("Ви: %1").arg(q));
        askBtn->setEnabled(false);
        chat->appendPlainText(tr("… обробка запиту"));
        qApp->processEvents();

        AssistantService::Request req;
        req.question = q;
        req.diary = &diary_;
        req.training = &trainingDiary_;
        const TrainingPreferences prefs = buildTrainingPreferences();
        req.prefs = &prefs;
        req.foodDb = &foodDb_;
        req.activityStreakDays = calculateConsistencyStreak();
        req.preferCloud = true;

        const AssistantService::Response resp = AssistantService::answer(req);

        QString block = chat->toPlainText();
        const QString waitLine = tr("… обробка запиту");
        if (block.endsWith(waitLine)) {
            block.chop(waitLine.size());
            while (block.endsWith(QLatin1Char('\n'))) {
                block.chop(1);
            }
            chat->setPlainText(block);
        }

        const QString header = resp.usedCloud
                                   ? tr("[Хмарна модель]")
                                   : tr("[%1]").arg(resp.modeLabel);
        const QString r = header + QLatin1Char('\n') + resp.text;
        chat->appendPlainText(r);
        askBtn->setEnabled(true);

        lastQuery = q;
        lastResponse = resp.text;
        input->clear();
    });

    connect(saveFeedbackBtn, &QPushButton::clicked, [&]() {
        if (lastResponse.isEmpty()) {
            QMessageBox::information(&dlg, tr("Немає відповіді"),
                                    tr("Спочатку отримайте відповідь помічника."));
            return;
        }
        const QString v = verdictCombo->currentData().toString();
        if (v.isEmpty()) {
            QMessageBox::warning(&dlg, tr("Оцінка"), tr("Оберіть оцінку поради."));
            return;
        }
        appendAssistantFeedback(lastQuery, lastResponse, v, feedbackComment->text());
        QMessageBox::information(
            &dlg,
            tr("Дякуємо"),
            tr("Відгук збережено у файлі pomichnyk_feedback.csv у каталозі даних користувача."));
    });

    dlg.exec();
    updateAssistantModeIndicator();
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

void MainWindow::onCompleteWorkout() {
    if (!workoutSessionActive_) {
        QMessageBox::warning(
            this, tr("Завершення тренування"),
            tr("Немає активної сесії тренування.\nСпочатку натисніть «Почати тренування»."));
        return;
    }
    QMessageBox::information(
        this, tr("Завершення тренування"),
        tr("Активна сесія відкрита у вікні тренування.\n"
           "Натисніть «Завершити тренування» у цьому вікні, щоб зберегти результат."));
}

void MainWindow::onStartWorkout() {
    if (workoutSessionActive_) {
        QMessageBox::warning(this, tr("Тренування"),
                             tr("Сесія тренування вже активна. Завершіть її перед новим стартом."));
        return;
    }

    auto typeUa = [this](const std::string& t) -> QString {
        if (t == "Strength") return tr("Сила");
        if (t == "Cardio") return tr("Кардіо");
        if (t == "Mobility") return tr("Мобільність");
        return tr("Тренування");
    };

    auto profileMetaSaved = [this]() -> bool {
        const QString profile = currentProfile();
        saveProfileMeta(profile);
        saveDiaryFor(profile, selectedDate_);
        const QString filePath =
            QDir(userDataRoot())
                .filePath(QStringLiteral("%1/profile.json").arg(profileStorageDir(profile)));
        return QFileInfo::exists(filePath);
    };

    // 1) Personal data (US: enter personal data / start without required data)
    const bool hasPersonalData =
        (weightSpin_->value() > 0.0 && ageSpin_->value() > 0 && heightSpin_->value() > 0.0);
    if (!hasPersonalData) {
        QDialog dataDlg(this);
        dataDlg.setWindowTitle(tr("Персональні дані для тренування"));
        auto* lay = new QVBoxLayout(&dataDlg);
        lay->addWidget(new QLabel(
            tr("Заповніть обов’язкові поля для персонального плану."), &dataDlg));
        auto* form = new QFormLayout();
        auto* ageBox = new QSpinBox(&dataDlg);
        ageBox->setRange(10, 100);
        ageBox->setSuffix(tr(" р."));
        ageBox->setValue(ageSpin_->value() > 0 ? ageSpin_->value() : 30);
        auto* heightBox = new QDoubleSpinBox(&dataDlg);
        heightBox->setRange(120.0, 230.0);
        heightBox->setDecimals(1);
        heightBox->setSuffix(tr(" см"));
        heightBox->setValue(heightSpin_->value() > 0.0 ? heightSpin_->value() : 175.0);
        auto* weightBox = new QDoubleSpinBox(&dataDlg);
        weightBox->setRange(30.0, 250.0);
        weightBox->setDecimals(1);
        weightBox->setSuffix(tr(" кг"));
        weightBox->setValue(weightSpin_->value() > 0.0 ? weightSpin_->value() : 70.0);
        auto* goalBox = new QComboBox(&dataDlg);
        goalBox->addItems({tr("Схуднення"), tr("Набір"), tr("Підтримка")});
        auto* levelBox = new QComboBox(&dataDlg);
        levelBox->addItems({tr("Початковий"), tr("Середній"), tr("Просунутий")});
        auto* durationBox = new QSpinBox(&dataDlg);
        durationBox->setRange(10, 120);
        durationBox->setValue(std::max(10, trainingDurationSpin_->value()));
        durationBox->setSuffix(tr(" хв"));
        form->addRow(tr("Вік *:"), ageBox);
        form->addRow(tr("Зріст *:"), heightBox);
        form->addRow(tr("Вага *:"), weightBox);
        form->addRow(tr("Ціль:"), goalBox);
        form->addRow(tr("Рівень:"), levelBox);
        form->addRow(tr("Тривалість:"), durationBox);
        lay->addLayout(form);
        auto* validationLbl = new QLabel(&dataDlg);
        validationLbl->setWordWrap(true);
        validationLbl->setObjectName(QStringLiteral("validationError"));
        lay->addWidget(validationLbl);
        auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dataDlg);
        lay->addWidget(btns);
        connect(btns, &QDialogButtonBox::rejected, &dataDlg, &QDialog::reject);
        connect(btns, &QDialogButtonBox::accepted, &dataDlg, [&]() {
            const QString bodyErr = ProfileValidation::errorForTrainingBody(ageBox->value(),
                                                                          heightBox->value(),
                                                                          weightBox->value());
            if (!bodyErr.isEmpty()) {
                validationLbl->setText(bodyErr);
                return;
            }
            if (durationBox->value() < 10) {
                validationLbl->setText(tr("Тривалість: мінімум 10 хв."));
                return;
            }
            validationLbl->clear();
            dataDlg.accept();
        });
        if (dataDlg.exec() != QDialog::Accepted) return;

        ageSpin_->setValue(ageBox->value());
        heightSpin_->setValue(heightBox->value());
        weightSpin_->setValue(weightBox->value());
        diary_.setWeightKg(weightBox->value());
        trainingGoalCombo_->setCurrentText(goalBox->currentText());
        trainingDurationSpin_->setValue(durationBox->value());
        activityCombo_->setCurrentIndex(levelBox->currentIndex());

        while (!profileMetaSaved()) {
            if (QMessageBox::warning(this, tr("Помилка збереження"),
                                     tr("Не вдалося зберегти персональні дані. Повторити?"),
                                     QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Retry)
                != QMessageBox::Retry) {
                return;
            }
        }
    }

    // 2) Generate plan (US: personalized training plan)
    TrainingPreferences prefs;
    const QString goalText = trainingGoalCombo_->currentText();
    if (goalText == tr("Схуднення")) prefs.goal = "cutting";
    else if (goalText == tr("Набір")) prefs.goal = "bulk";
    else prefs.goal = "maintenance";

    TrainingPlanGenerator generator;

    auto buildPlan = [&]() -> std::vector<TrainingSession> {
        std::vector<TrainingSession> sessions = generator.generateSessionsForDay(0, prefs);
        if (sessions.empty()) {
            sessions.push_back(TrainingSession("Cardio", std::max(20, trainingDurationSpin_->value()), "07:30",
                                               TrainingSession::Status::Planned, ""));
        } else {
            for (size_t i = 0; i < sessions.size(); ++i) {
                TrainingSession s = sessions[i];
                if (s.getDurationMin() <= 0) continue;
                int d = s.getDurationMin();
                if (i == 0) d = std::max(d, std::max(10, trainingDurationSpin_->value()));
                sessions[i] = TrainingSession(s.getType(), d, s.getTimeHHmm(), TrainingSession::Status::Planned,
                                              s.getNotes());
            }
        }
        return sessions;
    };

    std::vector<TrainingSession> plannedSessions;
    for (;;) {
        try {
            plannedSessions = buildPlan();
        } catch (...) {
            plannedSessions.clear();
        }
        if (!plannedSessions.empty()) break;
        if (QMessageBox::warning(this, tr("Помилка генерації плану"),
                                 tr("Не вдалося згенерувати план тренування. Повторити?"),
                                 QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Retry)
            != QMessageBox::Retry) {
            return;
        }
    }

    QString planBody;
    int plannedMinutes = 0;
    for (const auto& s : plannedSessions) {
        if (s.getDurationMin() <= 0) continue;
        planBody += tr("- %1 о %2, %3 хв\n")
                         .arg(typeUa(s.getType()))
                         .arg(QString::fromStdString(s.getTimeHHmm()))
                         .arg(s.getDurationMin());
        plannedMinutes += s.getDurationMin();
    }

    QMessageBox planMsg(this);
    planMsg.setWindowTitle(tr("План тренування"));
    planMsg.setText(tr("План на сьогодні:\n%1\nРазом: %2 хв").arg(planBody).arg(plannedMinutes));
    auto* usePlanBtn = planMsg.addButton(tr("За планом"), QMessageBox::AcceptRole);
    auto* noPlanBtn = planMsg.addButton(tr("Без плану (швидко)"), QMessageBox::ActionRole);
    planMsg.addButton(QMessageBox::Cancel);
    planMsg.exec();
    if (planMsg.clickedButton() == nullptr
        || planMsg.clickedButton() == planMsg.button(QMessageBox::Cancel)) {
        return;
    }
    const bool usePlan = (planMsg.clickedButton() == usePlanBtn);
    if (!usePlan) {
        plannedSessions.clear();
        const int dur = std::max(10, trainingDurationSpin_->value());
        plannedSessions.push_back(TrainingSession("Cardio", dur,
                                                  QTime::currentTime().toString("HH:mm").toStdString(),
                                                  TrainingSession::Status::Planned, "Швидке тренування"));
        plannedMinutes = dur;
        planBody = tr("- %1 о %2, %3 хв\n")
                       .arg(typeUa("Cardio"))
                       .arg(QString::fromStdString(plannedSessions.back().getTimeHHmm()))
                       .arg(dur);
    }

    std::vector<TrainingSession> workoutSteps;
    for (const auto& s : plannedSessions) {
        if (s.getDurationMin() > 0) workoutSteps.push_back(s);
    }
    if (usePlan && workoutSteps.empty()) {
        QMessageBox::information(this, tr("Немає вправ"),
                                 tr("У плані немає вправ на сьогодні. Згенеруйте план на 7 днів або оберіть "
                                    "«Без плану»."));
        return;
    }

    // 3) Start session confirmation (US: start workout)
    QMessageBox::information(
        this, tr("Тренування розпочато"),
        tr("Сесія тренування розпочата.\n\n%1Разом: %2 хв.").arg(planBody).arg(plannedMinutes));

    workoutSessionActive_ = true;
    activeWorkoutPlan_ = workoutSteps;

    // 4) Perform exercises (US: perform exercises)
    QDialog workoutDlg(this);
    workoutDlg.setWindowTitle(tr("Тренування виконується"));
    workoutDlg.resize(560, 440);
    auto* lay = new QVBoxLayout(&workoutDlg);
    auto* info = new QLabel(tr("Відмічайте виконані вправи з плану. Можна додати ще вправу."), &workoutDlg);
    auto* stepsBox = new QGroupBox(tr("Вправи з плану"), &workoutDlg);
    auto* stepsLay = new QVBoxLayout(stepsBox);
    QVector<QCheckBox*> stepBoxes;
    auto* detailLabel = new QLabel(&workoutDlg);
    detailLabel->setWordWrap(true);
    detailLabel->setObjectName(QStringLiteral("exerciseDetail"));

    auto showStepDetail = [&](QCheckBox* cb) {
        const int idx = stepBoxes.indexOf(cb);
        if (idx < 0 || idx >= workoutSteps.size()) return;
        const auto& s = workoutSteps[static_cast<size_t>(idx)];
        const QString notes = QString::fromStdString(s.getNotes()).trimmed();
        detailLabel->setText(
            tr("Тип: %1\nЧас: %2\nТривалість: %3 хв%4")
                .arg(typeUa(s.getType()))
                .arg(QString::fromStdString(s.getTimeHHmm()))
                .arg(s.getDurationMin())
                .arg(notes.isEmpty() ? QString() : tr("\nПримітки: %1").arg(notes)));
    };

    for (const auto& s : workoutSteps) {
        auto* cb = new QCheckBox(
            tr("%1 — %2 хв (%3)")
                .arg(typeUa(s.getType()))
                .arg(s.getDurationMin())
                .arg(QString::fromStdString(s.getTimeHHmm())),
            stepsBox);
        stepsLay->addWidget(cb);
        stepBoxes.append(cb);
        connect(cb, &QCheckBox::clicked, &workoutDlg, [cb, showStepDetail]() { showStepDetail(cb); });
    }
    if (!stepBoxes.isEmpty()) showStepDetail(stepBoxes.first());

    QVector<QCheckBox*> extraSteps;
    auto* addStepBtn = new QPushButton(tr("Додати вправу"), &workoutDlg);
    auto* progress = new QProgressBar(&workoutDlg);
    progress->setRange(0, 100);
    auto* finishBtn = new QPushButton(tr("Завершити тренування"), &workoutDlg);
    finishBtn->setEnabled(false);
    lay->addWidget(info);
    lay->addWidget(stepsBox);
    lay->addWidget(detailLabel);
    lay->addWidget(addStepBtn);
    lay->addWidget(progress);
    lay->addWidget(finishBtn);

    auto recomputeProgress = [=, &extraSteps]() mutable {
        const int total = stepBoxes.size() + extraSteps.size();
        int done = 0;
        for (auto* cb : stepBoxes) {
            if (cb->isChecked()) ++done;
        }
        for (auto* cb : extraSteps) {
            if (cb->isChecked()) ++done;
        }
        progress->setValue(total > 0 ? (done * 100) / total : 0);
        finishBtn->setEnabled(done == total && total > 0);
    };
    for (auto* cb : stepBoxes) {
        connect(cb, &QCheckBox::toggled, &workoutDlg, recomputeProgress);
    }
    connect(addStepBtn, &QPushButton::clicked, &workoutDlg, [&]() {
        auto* cb = new QCheckBox(tr("Додаткова вправа %1").arg(extraSteps.size() + 1), stepsBox);
        stepsLay->addWidget(cb);
        extraSteps.append(cb);
        connect(cb, &QCheckBox::toggled, &workoutDlg, recomputeProgress);
        recomputeProgress();
    });
    connect(finishBtn, &QPushButton::clicked, &workoutDlg, &QDialog::accept);
    recomputeProgress();

    const int workoutRc = workoutDlg.exec();
    workoutSessionActive_ = false;
    activeWorkoutPlan_.clear();
    if (workoutRc != QDialog::Accepted) return;

    // 5) Complete workout + save with retry (US: complete workout)
    auto persistWorkout = [&]() -> bool {
        TrainingDiary snapshot = trainingDiary_;
        for (const auto& s : plannedSessions) {
            if (s.getDurationMin() <= 0) continue;
            snapshot.addSession(TrainingSession(s.getType(), s.getDurationMin(), s.getTimeHHmm(),
                                                TrainingSession::Status::Completed, tr("Сесія тренування").toStdString()));
        }
        const QString profile = currentProfile();
        const QString path = trainingFilePath(profile, selectedDate_);
        if (!trainingSaver_.save(snapshot, path.toStdString())) return false;
        trainingDiary_ = snapshot;
        trainings_[profile][selectedDate_] = snapshot;
        return true;
    };

    while (!persistWorkout()) {
        if (QMessageBox::warning(this, tr("Помилка збереження"),
                                 tr("Не вдалося завершити тренування (помилка збереження). Повторити?"),
                                 QMessageBox::Retry | QMessageBox::Cancel, QMessageBox::Retry)
            != QMessageBox::Retry) {
            return;
        }
    }
    refreshTraining();

    int savedMinutes = 0;
    QString savedSummary;
    for (const auto& s : plannedSessions) {
        if (s.getDurationMin() <= 0) continue;
        savedMinutes += s.getDurationMin();
        savedSummary += tr("%1 (%2 хв)\n").arg(typeUa(s.getType())).arg(s.getDurationMin());
    }

    QMessageBox::information(
        this, tr("Тренування завершено"),
        tr("Тренування успішно завершено.\nЗбережено сесій:\n%1Разом: %2 хв.\nПерегляньте дашборд фітнесу.")
            .arg(savedSummary)
            .arg(savedMinutes));
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
