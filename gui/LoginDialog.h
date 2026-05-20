#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QLabel;
class QCheckBox;
class QPushButton;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent = nullptr);

    /** Канонічний логін для збереження даних (після входу за email або OAuth). */
    QString login() const;

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onGoogleClicked();
    void onAppleClicked();

private:
    QString canonicalLogin_;
    QLineEdit* loginEdit_{};
    QLineEdit* emailEdit_{};
    QLineEdit* passwordEdit_{};
    QLineEdit* password2Edit_{};
    QLabel* hintLabel_{};
    QCheckBox* consentCheck_{};
    QPushButton* termsBtn_{};
    QPushButton* privacyBtn_{};
};
