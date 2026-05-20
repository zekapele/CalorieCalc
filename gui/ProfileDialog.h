#pragma once

#include <QDialog>

class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QLineEdit;

class ProfileDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProfileDialog(const QString& login, QWidget* parent = nullptr);

    bool accountWasDeleted() const { return accountDeleted_; }

    void setProfileValues(double weightKg, int ageYears, double heightCm, int activityIndex);
    double weightKg() const;
    int ageYears() const;
    double heightCm() const;
    int activityIndex() const;

private slots:
    void onSaveClicked();
    void onDeleteAccountClicked();

private:
    QString login_;
    bool accountDeleted_{false};
    QDoubleSpinBox* weightSpin_{};
    QSpinBox* ageSpin_{};
    QDoubleSpinBox* heightSpin_{};
    QComboBox* activityCombo_{};
    QLineEdit* currentPasswordEdit_{};
    QLineEdit* newPasswordEdit_{};
    QLineEdit* newPassword2Edit_{};
};

