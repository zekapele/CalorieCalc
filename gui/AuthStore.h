#ifndef CALORIECALC_AUTHSTORE_H
#define CALORIECALC_AUTHSTORE_H

#include <QString>

// Локальне зберігання користувачів (логін + хеш пароля). Без мережі.
class AuthStore {
public:
    // Унікальна папка для даних користувача під data/<folder>/...
    static QString storageFolderForLogin(const QString& login);

    static bool registerUser(const QString& login,
                             const QString& emailOptional,
                             const QString& password,
                             bool legalDocumentsAccepted,
                             QString* errorOut = nullptr);
    /** Повертає канонічний логін у canonicalLoginOut (якщо не nullptr), зокрема після входу за email. */
    static bool verifyLogin(const QString& loginOrEmail,
                            const QString& password,
                            QString* errorOut = nullptr,
                            QString* canonicalLoginOut = nullptr);
    static QString emailForLogin(const QString& canonicalLogin);
    /** Локальний профіль після вибору Google (без зовнішньої авторизації). */
    static bool signInWithGoogle(bool legalDocumentsAccepted, QString* canonicalLoginOut, QString* errorOut = nullptr);
    static bool signInWithApple(bool legalDocumentsAccepted, QString* canonicalLoginOut, QString* errorOut = nullptr);
    static bool deleteAccount(const QString& login, const QString& password, QString* errorOut = nullptr);
    static bool changePassword(const QString& login,
                               const QString& currentPassword,
                               const QString& newPassword,
                               QString* errorOut = nullptr);
};

#endif
