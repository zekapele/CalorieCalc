#ifndef CALORIECALC_AUTHSTORE_H
#define CALORIECALC_AUTHSTORE_H

#include <QString>

// Локальне зберігання користувачів (логін + хеш пароля). Без мережі.
class AuthStore {
public:
    // Унікальна папка для даних користувача під data/<folder>/...
    static QString storageFolderForLogin(const QString& login);

    static bool registerUser(const QString& login, const QString& password, QString* errorOut = nullptr);
    static bool verifyLogin(const QString& login, const QString& password, QString* errorOut = nullptr);
    static bool changePassword(const QString& login,
                               const QString& currentPassword,
                               const QString& newPassword,
                               QString* errorOut = nullptr);
};

#endif
