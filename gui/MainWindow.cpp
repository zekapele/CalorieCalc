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

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    applyBlueWhiteTheme();
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
    leftLayout->addWidget(searchBtn);
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

    // Weekly report button
    weeklyReportBtn_ = new QPushButton(tr("Звіт за 7 днів"), rightBox);

    // Arrange right panel
    auto* goalsContainer = goalsBox;
    rightLayout->setSpacing(8);
    rightLayout->addWidget(diaryTabs_);
    rightLayout->addLayout(tmplRow);
    rightLayout->addWidget(removeButton_);
    rightLayout->addWidget(goalsContainer);
    rightLayout->addWidget(weeklyReportBtn_);
    rightBox->setLayout(rightLayout);

    // Layout
    rootLayout->setSpacing(12);
    rootLayout->addLayout(leftColumn, 1);
    rootLayout->addWidget(rightBox, 1);
    setCentralWidget(central);

    // Signals
    connect(searchBtn, &QPushButton::clicked, this, &MainWindow::onSearch);
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

    // Init storage
    ensureStorageDirs();
    loadDiaryFor(currentProfile(), selectedDate_);
    // Initialize controls from diary
    goalSpin_->setValue(static_cast<int>(diary_.getCalorieGoal()));
    waterGoalSpin_->setValue(diary_.getWaterGoalMl());
    weightSpin_->setValue(diary_.getWeightKg());
    refreshDiary();
    refreshStats();
}

void MainWindow::applyBlueWhiteTheme() {
    // Softened black–orange dark theme
    const QColor bgMain(16, 16, 16);
    const QColor bgPanel(24, 24, 24);
    const QColor bgAlt(30, 30, 30);
    const QColor textMain(232, 232, 232);
    const QColor accent(255, 169, 77);

    QPalette pal = qApp->palette();
    pal.setColor(QPalette::Window, bgMain);
    pal.setColor(QPalette::Base, bgPanel);
    pal.setColor(QPalette::AlternateBase, bgAlt);
    pal.setColor(QPalette::Text, textMain);
    pal.setColor(QPalette::WindowText, textMain);
    pal.setColor(QPalette::Button, bgPanel);
    pal.setColor(QPalette::ButtonText, textMain);
    pal.setColor(QPalette::Highlight, accent);
    pal.setColor(QPalette::HighlightedText, QColor(20, 20, 20));
    qApp->setPalette(pal);

    setStyleSheet(
        "QGroupBox { font-weight: 600; border: 1px solid #2C2C2C; border-radius: 8px; margin-top: 12px; color: #E8E8E8; }"
        "QGroupBox::title { padding: 0 8px; color: #FFA94D; }"
        "QPushButton { background: #2A2A2A; color: #E8E8E8; padding: 7px 12px; border: 1px solid #3A3A3A; border-radius: 8px; }"
        "QPushButton:hover { border-color: #FFA94D; background: #333333; }"
        "QLineEdit, QSpinBox, QComboBox, QDoubleSpinBox { padding: 7px 9px; border: 1px solid #3A3A3A; border-radius: 8px; background: #181818; color: #E8E8E8; }"
        "QListWidget { background: #181818; border: 1px solid #2C2C2C; border-radius: 8px; color: #E8E8E8; }"
        "QTabBar::tab { background: #1E1E1E; color: #C8C8C8; padding: 7px 14px; border-top-left-radius: 8px; border-top-right-radius: 8px; margin-right: 4px; border: 1px solid #2C2C2C; }"
        "QTabBar::tab:selected { background: #2A2A2A; color: #FFA94D; border-color: #3A3A3A; }"
        "QTabWidget::pane { border: 1px solid #2C2C2C; border-radius: 8px; top: -1px; }"
        "QLabel { color: #E8E8E8; }"
        "QCalendarWidget QWidget { background-color: #181818; color: #E8E8E8; }"
        "QCalendarWidget QAbstractItemView:enabled { selection-background-color: #FFA94D; selection-color: #141414; }"
        "QProgressBar { border: 1px solid #3A3A3A; border-radius: 8px; background: #181818; color: #E8E8E8; text-align: center; }"
        "QProgressBar::chunk { background: #FFA94D; border-radius: 8px; }"
    );
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
    const auto query = searchEdit_->text().toStdString();
    auto results = foodDb_.searchFoods(query);
    for (const auto& food : results) {
        auto* item = new QListWidgetItem(QString::fromStdString(food.getName()), resultsList_);
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
    for (int i = 0; i < 7; ++i) {
        QDate day = selectedDate_.addDays(-i);
        Diary d; d.setCalorieGoal(2000);
        if (jsonSaver_.load(d, diaryFilePath(profile, day).toStdString())) {
            const double kcal = d.getTotalCalories();
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
