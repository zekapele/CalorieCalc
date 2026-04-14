#include "ProfileDialog.h"
#include "AuthStore.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

ProfileDialog::ProfileDialog(const QString& login, QWidget* parent)
    : QDialog(parent), login_(login) {
    setWindowTitle(tr("Профіль користувача"));
    setModal(true);
    resize(460, 380);

    auto* root = new QVBoxLayout(this);

    auto* infoLabel = new QLabel(tr("Користувач: %1").arg(login_), this);
    root->addWidget(infoLabel);

    auto* bodyGroup = new QGroupBox(tr("Персональні дані"), this);
    auto* bodyForm = new QFormLayout(bodyGroup);

    weightSpin_ = new QDoubleSpinBox(bodyGroup);
    weightSpin_->setRange(0.0, 400.0);
    weightSpin_->setDecimals(1);
    weightSpin_->setSuffix(tr(" кг"));

    ageSpin_ = new QSpinBox(bodyGroup);
    ageSpin_->setRange(10, 100);
    ageSpin_->setSuffix(tr(" р."));

    heightSpin_ = new QDoubleSpinBox(bodyGroup);
    heightSpin_->setRange(120.0, 230.0);
    heightSpin_->setDecimals(1);
    heightSpin_->setSuffix(tr(" см"));

    activityCombo_ = new QComboBox(bodyGroup);
    activityCombo_->addItems({tr("Сидячий"), tr("Помірний"), tr("Активний")});

    bodyForm->addRow(tr("Вага:"), weightSpin_);
    bodyForm->addRow(tr("Вік:"), ageSpin_);
    bodyForm->addRow(tr("Зріст:"), heightSpin_);
    bodyForm->addRow(tr("Активність:"), activityCombo_);
    root->addWidget(bodyGroup);

    auto* pwdGroup = new QGroupBox(tr("Зміна пароля"), this);
    auto* pwdForm = new QFormLayout(pwdGroup);
    currentPasswordEdit_ = new QLineEdit(pwdGroup);
    currentPasswordEdit_->setEchoMode(QLineEdit::Password);
    newPasswordEdit_ = new QLineEdit(pwdGroup);
    newPasswordEdit_->setEchoMode(QLineEdit::Password);
    newPassword2Edit_ = new QLineEdit(pwdGroup);
    newPassword2Edit_->setEchoMode(QLineEdit::Password);
    pwdForm->addRow(tr("Поточний пароль:"), currentPasswordEdit_);
    pwdForm->addRow(tr("Новий пароль:"), newPasswordEdit_);
    pwdForm->addRow(tr("Повтор нового:"), newPassword2Edit_);
    root->addWidget(pwdGroup);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    auto* saveBtn = new QPushButton(tr("Зберегти"), this);
    buttons->addButton(saveBtn, QDialogButtonBox::AcceptRole);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &ProfileDialog::onSaveClicked);
    root->addWidget(buttons);
}

void ProfileDialog::setProfileValues(double weightKg, int ageYears, double heightCm, int activityIndex) {
    weightSpin_->setValue(weightKg);
    ageSpin_->setValue(ageYears);
    heightSpin_->setValue(heightCm);
    activityCombo_->setCurrentIndex(activityIndex);
}

double ProfileDialog::weightKg() const { return weightSpin_->value(); }
int ProfileDialog::ageYears() const { return ageSpin_->value(); }
double ProfileDialog::heightCm() const { return heightSpin_->value(); }
int ProfileDialog::activityIndex() const { return activityCombo_->currentIndex(); }

void ProfileDialog::onSaveClicked() {
    const QString cur = currentPasswordEdit_->text();
    const QString np = newPasswordEdit_->text();
    const QString np2 = newPassword2Edit_->text();

    // Пароль міняємо тільки коли хоч одне поле заповнене
    if (!cur.isEmpty() || !np.isEmpty() || !np2.isEmpty()) {
        if (np != np2) {
            QMessageBox::warning(this, tr("Профіль"), tr("Новий пароль і повтор не збігаються."));
            return;
        }
        QString err;
        if (!AuthStore::changePassword(login_, cur, np, &err)) {
            QMessageBox::warning(this, tr("Профіль"), err);
            return;
        }
    }

    accept();
}

