#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
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
    auto* central = new QWidget(this);
    auto* rootLayout = new QHBoxLayout(central);

    // Left column (controls)
    auto* leftColumn = new QVBoxLayout();

    // Top controls: profile + calendar
    auto* topBox = new QGroupBox(tr("Профіль і календар"), central);
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
    auto* leftBox = new QGroupBox(tr("Пошук продуктів"), central);
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
    auto* customBox = new QGroupBox(tr("Додати свій продукт"), central);
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
    auto* rightBox = new QGroupBox(tr("Щоденник"), central);
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
    rightLayout->addWidget(weeklyReportBtn_);
    rightLayout->addLayout(actionsRow);
    rightBox->setLayout(rightLayout);

    // Layout
    rootLayout->setSpacing(12);
    rootLayout->addLayout(leftColumn, 1);
    rootLayout->addWidget(rightBox, 1);
    setCentralWidget(central);

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

    // Init storage
    ensureStorageDirs();
    loadDiaryFor(currentProfile(), selectedDate_);
    // Initialize controls from diary
    goalSpin_->setValue(static_cast<int>(diary_.getCalorieGoal()));
    waterGoalSpin_->setValue(diary_.getWaterGoalMl());
    weightSpin_->setValue(diary_.getWeightKg());
    refreshDiary();
    refreshStats();
    
    // Apply initial theme
    applyDarkTheme();
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
        // Modern dark theme with elegant colors
        const QColor bgMain(18, 18, 22);        // Deep dark blue-gray
        const QColor bgPanel(26, 28, 34);       // Slightly lighter panel
        const QColor bgAlt(32, 34, 42);         // Alternate background
        const QColor textMain(230, 230, 235);   // Soft white
        const QColor accent(100, 181, 246);     // Modern blue accent
        const QColor accentHover(129, 199, 252); // Lighter blue for hover
        const QColor border(45, 47, 55);         // Subtle border

        QPalette pal = qApp->palette();
        pal.setColor(QPalette::Window, bgMain);
        pal.setColor(QPalette::Base, bgPanel);
        pal.setColor(QPalette::AlternateBase, bgAlt);
        pal.setColor(QPalette::Text, textMain);
        pal.setColor(QPalette::WindowText, textMain);
        pal.setColor(QPalette::Button, bgPanel);
        pal.setColor(QPalette::ButtonText, textMain);
        pal.setColor(QPalette::Highlight, accent);
        pal.setColor(QPalette::HighlightedText, QColor(18, 18, 22));
        qApp->setPalette(pal);

        setStyleSheet(
            "QGroupBox { font-weight: 600; border: 1px solid #2D2F37; border-radius: 10px; margin-top: 14px; padding-top: 8px; color: #E6E6EB; background: #1A1A1E; }"
            "QGroupBox::title { padding: 0 10px; color: #64B5F6; subcontrol-origin: margin; subcontrol-position: top left; }"
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2A2C36, stop:1 #1E2028); color: #E6E6EB; padding: 8px 16px; border: 1px solid #2D2F37; border-radius: 8px; font-weight: 500; }"
            "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3A3C46, stop:1 #2E3038); border-color: #64B5F6; color: #81C7FC; }"
            "QPushButton:pressed { background: #1A1C24; }"
            "QLineEdit, QSpinBox, QComboBox, QDoubleSpinBox { padding: 8px 12px; border: 1px solid #2D2F37; border-radius: 8px; background: #1A1A1E; color: #E6E6EB; selection-background-color: #64B5F6; selection-color: #1A1A1E; }"
            "QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDoubleSpinBox:focus { border: 2px solid #64B5F6; background: #1E2024; }"
            "QListWidget { background: #1A1A1E; border: 1px solid #2D2F37; border-radius: 8px; color: #E6E6EB; }"
            "QListWidget::item { padding: 6px; border-radius: 4px; }"
            "QListWidget::item:selected { background: #64B5F6; color: #1A1A1E; }"
            "QListWidget::item:hover { background: #2A2C36; }"
            "QTabBar::tab { background: #1E2028; color: #A0A0A5; padding: 10px 18px; border-top-left-radius: 10px; border-top-right-radius: 10px; margin-right: 2px; border: 1px solid #2D2F37; border-bottom: none; }"
            "QTabBar::tab:selected { background: #1A1A1E; color: #64B5F6; border-color: #2D2F37; border-bottom: 2px solid #64B5F6; font-weight: 600; }"
            "QTabBar::tab:hover:!selected { background: #24262E; color: #C0C0C5; }"
            "QTabWidget::pane { border: 1px solid #2D2F37; border-radius: 8px; top: -1px; background: #1A1A1E; }"
            "QLabel { color: #E6E6EB; }"
            "QCalendarWidget { background-color: #1A1A1E; color: #E6E6EB; border: 1px solid #2D2F37; border-radius: 8px; }"
            "QCalendarWidget QAbstractItemView:enabled { selection-background-color: #64B5F6; selection-color: #1A1A1E; background-color: #1A1A1E; }"
            "QCalendarWidget QHeaderView::section { background-color: #1E2028; color: #E6E6EB; border: none; padding: 8px; }"
            "QProgressBar { border: 1px solid #2D2F37; border-radius: 10px; background: #1A1A1E; color: #E6E6EB; text-align: center; height: 20px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #64B5F6, stop:1 #81C7FC); border-radius: 10px; }"
        );
    } else {
        // Modern light theme with fresh colors
        QPalette pal = qApp->palette();
        pal.setColor(QPalette::Window, QColor(250, 250, 252));
        pal.setColor(QPalette::Base, QColor(255, 255, 255));
        pal.setColor(QPalette::AlternateBase, QColor(248, 249, 251));
        pal.setColor(QPalette::Text, QColor(30, 30, 35));
        pal.setColor(QPalette::WindowText, QColor(30, 30, 35));
        pal.setColor(QPalette::Button, QColor(245, 247, 250));
        pal.setColor(QPalette::ButtonText, QColor(30, 30, 35));
        pal.setColor(QPalette::Highlight, QColor(59, 130, 246));
        pal.setColor(QPalette::HighlightedText, Qt::white);
        qApp->setPalette(pal);

        setStyleSheet(
            "QGroupBox { font-weight: 600; border: 1px solid #E1E4E8; border-radius: 10px; margin-top: 14px; padding-top: 8px; color: #1E1E23; background: #FFFFFF; }"
            "QGroupBox::title { padding: 0 10px; color: #3B82F6; subcontrol-origin: margin; subcontrol-position: top left; }"
            "QPushButton { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F5F7FA, stop:1 #E8EBF0); color: #1E1E23; padding: 8px 16px; border: 1px solid #D1D5DB; border-radius: 8px; font-weight: 500; }"
            "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #E8EBF0, stop:1 #DCE0E5); border-color: #3B82F6; color: #2563EB; }"
            "QPushButton:pressed { background: #D1D5DB; }"
            "QLineEdit, QSpinBox, QComboBox, QDoubleSpinBox { padding: 8px 12px; border: 1px solid #D1D5DB; border-radius: 8px; background: #FFFFFF; color: #1E1E23; selection-background-color: #3B82F6; selection-color: #FFFFFF; }"
            "QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QDoubleSpinBox:focus { border: 2px solid #3B82F6; background: #FAFBFC; }"
            "QListWidget { background: #FFFFFF; border: 1px solid #E1E4E8; border-radius: 8px; color: #1E1E23; }"
            "QListWidget::item { padding: 6px; border-radius: 4px; }"
            "QListWidget::item:selected { background: #3B82F6; color: #FFFFFF; }"
            "QListWidget::item:hover { background: #F3F4F6; }"
            "QTabBar::tab { background: #F5F7FA; color: #6B7280; padding: 10px 18px; border-top-left-radius: 10px; border-top-right-radius: 10px; margin-right: 2px; border: 1px solid #E1E4E8; border-bottom: none; }"
            "QTabBar::tab:selected { background: #FFFFFF; color: #3B82F6; border-color: #E1E4E8; border-bottom: 2px solid #3B82F6; font-weight: 600; }"
            "QTabBar::tab:hover:!selected { background: #F9FAFB; color: #4B5563; }"
            "QTabWidget::pane { border: 1px solid #E1E4E8; border-radius: 8px; top: -1px; background: #FFFFFF; }"
            "QLabel { color: #1E1E23; }"
            "QCalendarWidget { background-color: #FFFFFF; color: #1E1E23; border: 1px solid #E1E4E8; border-radius: 8px; }"
            "QCalendarWidget QAbstractItemView:enabled { selection-background-color: #3B82F6; selection-color: #FFFFFF; background-color: #FFFFFF; }"
            "QCalendarWidget QHeaderView::section { background-color: #F9FAFB; color: #1E1E23; border: none; padding: 8px; }"
            "QProgressBar { border: 1px solid #D1D5DB; border-radius: 10px; background: #F3F4F6; color: #1E1E23; text-align: center; height: 20px; }"
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3B82F6, stop:1 #60A5FA); border-radius: 10px; }"
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

void MainWindow::onDateChanged(const QDate& date) {
    // Save previous
    saveDiaryFor(currentProfile(), selectedDate_);
    // Switch
    selectedDate_ = date;
    loadDiaryFor(currentProfile(), selectedDate_);
    refreshDiary();
    refreshStats();
}

void MainWindow::onProfileChanged(int) {
    // Ensure dir exists for new profile
    ensureStorageDirs();
    // Save previous
    saveDiaryFor(currentProfile(), selectedDate_);
    // Load for new profile/current date
    loadDiaryFor(currentProfile(), selectedDate_);
    refreshDiary();
    refreshStats();
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
