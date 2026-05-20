#include "LoginDialog.h"
#include "AuthStore.h"
#include "LegalDocuments.h"
#include "AppStyle.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("CalorieCalc — вхід"));
    setModal(true);
    setWindowFlags(windowFlags() | Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    resize(520, 560);
    setMinimumSize(480, 520);
    AppStyle::styleDialog(this);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 20);
    root->setSpacing(16);

    auto* brand = new QFrame(this);
    brand->setObjectName(QStringLiteral("appHeader"));
    auto* brandLay = new QVBoxLayout(brand);
    brandLay->setContentsMargins(20, 18, 20, 18);
    auto* title = new QLabel(QStringLiteral("CalorieCalc"), brand);
    title->setObjectName(QStringLiteral("appTitle"));
    auto* subtitle = new QLabel(tr("Персональний облік харчування та тренувань"), brand);
    subtitle->setObjectName(QStringLiteral("appSubtitle"));
    brandLay->addWidget(title);
    brandLay->addWidget(subtitle);
    root->addWidget(brand);

    auto* card = new QFrame(this);
    card->setObjectName(QStringLiteral("loginCard"));
    auto* cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(20, 20, 20, 20);
    cardLay->setSpacing(12);

    auto* form = new QFormLayout();
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignLeft);

    loginEdit_ = new QLineEdit(card);
    loginEdit_->setPlaceholderText(tr("Логін (або email для входу)"));
    emailEdit_ = new QLineEdit(card);
    emailEdit_->setPlaceholderText(tr("Email (для реєстрації)"));
    passwordEdit_ = new QLineEdit(card);
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setPlaceholderText(tr("Пароль"));
    password2Edit_ = new QLineEdit(card);
    password2Edit_->setEchoMode(QLineEdit::Password);
    password2Edit_->setPlaceholderText(tr("Повтор пароля (реєстрація)"));

    form->addRow(tr("Логін / email"), loginEdit_);
    form->addRow(tr("Email"), emailEdit_);
    form->addRow(tr("Пароль"), passwordEdit_);
    form->addRow(tr("Повтор"), password2Edit_);
    cardLay->addLayout(form);

    hintLabel_ = new QLabel(
        tr("Створіть обліковий запис або увійдіть за логіном чи email. "
           "Вхід через Google або Apple створює локальний профіль на цьому пристрої."),
        card);
    hintLabel_->setObjectName(QStringLiteral("mutedHint"));
    hintLabel_->setWordWrap(true);
    cardLay->addWidget(hintLabel_);

    consentCheck_ = new QCheckBox(
        tr("Погоджуюся з Умовами та Політикою (обов’язково для реєстрації / OAuth)."),
        card);
    cardLay->addWidget(consentCheck_);

    auto* legalRow = new QHBoxLayout();
    termsBtn_ = new QPushButton(tr("Умови"), card);
    privacyBtn_ = new QPushButton(tr("Конфіденційність"), card);
    legalRow->addWidget(termsBtn_);
    legalRow->addWidget(privacyBtn_);
    legalRow->addStretch();
    cardLay->addLayout(legalRow);

    root->addWidget(card, 1);

    auto* oauthRow = new QHBoxLayout();
    auto* googleBtn = new QPushButton(tr("Google"), this);
    auto* appleBtn = new QPushButton(tr("Apple"), this);
    oauthRow->addWidget(googleBtn);
    oauthRow->addWidget(appleBtn);
    oauthRow->addStretch();
    root->addLayout(oauthRow);

    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    auto* loginBtn = new QPushButton(tr("Увійти"), this);
    auto* regBtn = new QPushButton(tr("Зареєструватися"), this);
    btnBox->addButton(loginBtn, QDialogButtonBox::AcceptRole);
    btnBox->addButton(regBtn, QDialogButtonBox::ActionRole);
    AppStyle::markPrimary(loginBtn);
    AppStyle::markAccent(regBtn);
    root->addWidget(btnBox);

    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(loginBtn, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(regBtn, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(googleBtn, &QPushButton::clicked, this, &LoginDialog::onGoogleClicked);
    connect(appleBtn, &QPushButton::clicked, this, &LoginDialog::onAppleClicked);
    connect(termsBtn_, &QPushButton::clicked, this, [this] { showLegalDocument(this, LegalDocumentKind::TermsOfService); });
    connect(privacyBtn_, &QPushButton::clicked, this, [this] { showLegalDocument(this, LegalDocumentKind::PrivacyPolicy); });
}

QString LoginDialog::login() const {
    return canonicalLogin_;
}

void LoginDialog::onLoginClicked() {
    QString err;
    QString canonical;
    if (!AuthStore::verifyLogin(loginEdit_->text(), passwordEdit_->text(), &err, &canonical)) {
        QMessageBox::warning(this, tr("Помилка входу"), err);
        return;
    }
    canonicalLogin_ = canonical.isEmpty() ? loginEdit_->text().trimmed() : canonical;
    accept();
}

void LoginDialog::onRegisterClicked() {
    if (passwordEdit_->text() != password2Edit_->text()) {
        QMessageBox::warning(this, tr("Реєстрація"), tr("Паролі не збігаються."));
        return;
    }
    QString err;
    if (!AuthStore::registerUser(loginEdit_->text(),
                                 emailEdit_->text(),
                                 passwordEdit_->text(),
                                 consentCheck_->isChecked(),
                                 &err)) {
        QMessageBox::warning(this, tr("Реєстрація"), err);
        return;
    }
    canonicalLogin_ = loginEdit_->text().trimmed();
    QMessageBox::information(this, tr("Реєстрація"), tr("Ласкаво просимо! Обліковий запис створено."));
    accept();
}

void LoginDialog::onGoogleClicked() {
    QString err;
    QString canonical;
    if (!AuthStore::signInWithGoogle(consentCheck_->isChecked(), &canonical, &err)) {
        QMessageBox::warning(this, tr("Google"), err);
        return;
    }
    canonicalLogin_ = canonical;
    accept();
}

void LoginDialog::onAppleClicked() {
    QString err;
    QString canonical;
    if (!AuthStore::signInWithApple(consentCheck_->isChecked(), &canonical, &err)) {
        QMessageBox::warning(this, tr("Apple"), err);
        return;
    }
    canonicalLogin_ = canonical;
    accept();
}
