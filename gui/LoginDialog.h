#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QLabel;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent = nullptr);

    QString login() const;

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QLineEdit* loginEdit_{};
    QLineEdit* passwordEdit_{};
    QLineEdit* password2Edit_{};
    QLabel* hintLabel_{};
};
