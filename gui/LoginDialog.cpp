#include "LoginDialog.h"
#include "AuthStore.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Вхід — CalorieCalc"));
    setModal(true);
    resize(420, 260);

    auto* lay = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    loginEdit_ = new QLineEdit(this);
    loginEdit_->setPlaceholderText(tr("Логін"));
    passwordEdit_ = new QLineEdit(this);
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setPlaceholderText(tr("Пароль"));
    password2Edit_ = new QLineEdit(this);
    password2Edit_->setEchoMode(QLineEdit::Password);
    password2Edit_->setPlaceholderText(tr("Повтор пароля (для реєстрації)"));

    form->addRow(tr("Логін:"), loginEdit_);
    form->addRow(tr("Пароль:"), passwordEdit_);
    form->addRow(tr("Повтор:"), password2Edit_);

    hintLabel_ = new QLabel(
        tr("Локальні облікові записи зберігаються у data/_auth/users.json. Пароль зберігається як SHA-256 з сіллю."),
        this);
    hintLabel_->setWordWrap(true);

    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    auto* loginBtn = new QPushButton(tr("Увійти"), this);
    auto* regBtn = new QPushButton(tr("Зареєструватися"), this);
    btnBox->addButton(loginBtn, QDialogButtonBox::AcceptRole);
    btnBox->addButton(regBtn, QDialogButtonBox::ActionRole);

    lay->addLayout(form);
    lay->addWidget(hintLabel_);
    lay->addWidget(btnBox);

    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(regBtn, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
}

QString LoginDialog::login() const {
    return loginEdit_ ? loginEdit_->text().trimmed() : QString();
}

void LoginDialog::onLoginClicked() {
    QString err;
    if (!AuthStore::verifyLogin(loginEdit_->text(), passwordEdit_->text(), &err)) {
        QMessageBox::warning(this, tr("Помилка входу"), err);
        return;
    }
    accept();
}

void LoginDialog::onRegisterClicked() {
    if (passwordEdit_->text() != password2Edit_->text()) {
        QMessageBox::warning(this, tr("Реєстрація"), tr("Паролі не збігаються."));
        return;
    }
    QString err;
    if (!AuthStore::registerUser(loginEdit_->text(), passwordEdit_->text(), &err)) {
        QMessageBox::warning(this, tr("Реєстрація"), err);
        return;
    }
    QMessageBox::information(this, tr("Реєстрація"), tr("Обліковий запис створено. Ви увійшли."));
    accept();
}
