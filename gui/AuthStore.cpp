#include "AuthStore.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QDateTime>

namespace {

QString usersJsonPath() {
    return QDir(QDir::currentPath()).filePath(QStringLiteral("data/_auth/users.json"));
}

bool ensureAuthDirImpl() {
    const QString p = QDir(QDir::currentPath()).filePath(QStringLiteral("data/_auth"));
    return QDir().mkpath(p);
}

QByteArray hashPassword(const QString& password, const QByteArray& saltBytes) {
    return QCryptographicHash::hash(saltBytes + password.toUtf8(), QCryptographicHash::Sha256).toHex();
}

bool loadUsersArray(QJsonArray& out) {
    const QString path = usersJsonPath();
    QFile f(path);
    if (!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        out = QJsonArray();
        return true;
    }
    const QByteArray data = f.readAll();
    f.close();
    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (!doc.isObject()) return false;
    const QJsonObject root = doc.object();
    if (!root.contains(QLatin1String("users")) || !root[QLatin1String("users")].isArray()) {
        out = QJsonArray();
        return true;
    }
    out = root[QLatin1String("users")].toArray();
    return true;
}

bool saveUsersArray(const QJsonArray& users) {
    if (!ensureAuthDirImpl()) return false;
    QJsonObject root;
    root[QLatin1String("users")] = users;
    const QJsonDocument doc(root);
    QFile f(usersJsonPath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) return false;
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
    return true;
}

bool validateLoginString(const QString& login, QString* err) {
    const QString t = login.trimmed();
    if (t.size() < 2 || t.size() > 64) {
        if (err) *err = QStringLiteral("Логін: від 2 до 64 символів.");
        return false;
    }
    for (const QChar& c : t) {
        if (!c.isLetterOrNumber() && c != QLatin1Char('_') && c != QLatin1Char('.') && c != QLatin1Char('@')
            && c != QLatin1Char('-')) {
            if (err) *err = QStringLiteral("Логін: лише літери, цифри та ._@-");
            return false;
        }
    }
    return true;
}

bool validateEmailString(const QString& email, QString* err) {
    const QString e = email.trimmed();
    if (e.isEmpty()) return true;
    if (e.size() < 5 || e.size() > 128 || !e.contains(QLatin1Char('@')) || e.startsWith(QLatin1Char('@'))
        || e.endsWith(QLatin1Char('@'))) {
        if (err) *err = QStringLiteral("Некоректний формат email.");
        return false;
    }
    return true;
}

QString normalizeEmail(const QString& email) {
    return email.trimmed().toLower();
}

bool emailTaken(const QJsonArray& users, const QString& emailNorm, const QString& exceptLogin = QString()) {
    if (emailNorm.isEmpty()) return false;
    for (const QJsonValue& v : users) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        if (!exceptLogin.isEmpty()
            && o.value(QLatin1String("login")).toString().compare(exceptLogin, Qt::CaseInsensitive) == 0) {
            continue;
        }
        if (normalizeEmail(o.value(QLatin1String("email")).toString()) == emailNorm) return true;
    }
    return false;
}

QString findLoginByEmail(const QJsonArray& users, const QString& emailNorm) {
    for (const QJsonValue& v : users) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        if (normalizeEmail(o.value(QLatin1String("email")).toString()) == emailNorm) {
            return o.value(QLatin1String("login")).toString();
        }
    }
    return {};
}

bool passwordMatchesUser(const QJsonObject& o, const QString& password, QString* err) {
    const QString saltHex = o.value(QLatin1String("salt_hex")).toString();
    const QString storedHash = o.value(QLatin1String("hash_hex")).toString();
    const QByteArray salt = QByteArray::fromHex(saltHex.toLatin1());
    if (salt.isEmpty()) {
        if (err) *err = QStringLiteral("Пошкоджені дані користувача.");
        return false;
    }
    const QString computed = QString::fromLatin1(hashPassword(password, salt));
    if (computed.compare(storedHash, Qt::CaseInsensitive) == 0) return true;
    if (err) *err = QStringLiteral("Невірний пароль.");
    return false;
}

} // namespace

QString AuthStore::storageFolderForLogin(const QString& login) {
    const QByteArray h = QCryptographicHash::hash(login.trimmed().toUtf8(), QCryptographicHash::Sha256);
    return QStringLiteral("u_%1").arg(QString::fromLatin1(h.toHex().left(16)));
}

bool AuthStore::registerUser(const QString& login,
                             const QString& emailOptional,
                             const QString& password,
                             bool legalDocumentsAccepted,
                             QString* errorOut) {
    if (!legalDocumentsAccepted) {
        if (errorOut) {
            *errorOut = QStringLiteral(
                "Підтвердіть згоду з Умовами використання та Політикою конфіденційності (прапорець при реєстрації).");
        }
        return false;
    }
    if (!validateLoginString(login, errorOut)) return false;
    if (!validateEmailString(emailOptional, errorOut)) return false;
    if (password.size() < 4) {
        if (errorOut) *errorOut = QStringLiteral("Пароль: мінімум 4 символи.");
        return false;
    }
    if (!ensureAuthDirImpl()) {
        if (errorOut) *errorOut = QStringLiteral("Не вдалося створити каталог для облікових записів.");
        return false;
    }

    QJsonArray users;
    if (!loadUsersArray(users)) {
        if (errorOut) *errorOut = QStringLiteral("Помилка читання файлу користувачів.");
        return false;
    }

    const QString key = login.trimmed();
    const QString emailNorm = normalizeEmail(emailOptional);
    if (!emailNorm.isEmpty() && emailTaken(users, emailNorm)) {
        if (errorOut) *errorOut = QStringLiteral("Користувач з таким email вже зареєстрований.");
        return false;
    }

    for (const QJsonValue& v : users) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        if (o.value(QLatin1String("login")).toString().compare(key, Qt::CaseInsensitive) == 0) {
            if (errorOut) *errorOut = QStringLiteral("Користувач з таким логіном вже існує.");
            return false;
        }
    }

    QByteArray salt(16, 0);
    for (int i = 0; i < salt.size(); ++i) {
        salt[i] = char(QRandomGenerator::global()->bounded(256));
    }
    const QString saltHex = QString::fromLatin1(salt.toHex());
    const QString hashHex = QString::fromLatin1(hashPassword(password, salt));

    QJsonObject u;
    u[QLatin1String("login")] = key;
    u[QLatin1String("salt_hex")] = saltHex;
    u[QLatin1String("hash_hex")] = hashHex;
    u[QLatin1String("folder")] = storageFolderForLogin(key);
    u[QLatin1String("consent_iso")] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    if (!emailNorm.isEmpty()) u[QLatin1String("email")] = emailNorm;
    users.append(u);

    if (!saveUsersArray(users)) {
        if (errorOut) *errorOut = QStringLiteral("Не вдалося зберегти обліковий запис.");
        return false;
    }
    return true;
}

bool AuthStore::verifyLogin(const QString& loginOrEmail,
                            const QString& password,
                            QString* errorOut,
                            QString* canonicalLoginOut) {
    if (password.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Введіть пароль.");
        return false;
    }

    QJsonArray users;
    if (!loadUsersArray(users)) {
        if (errorOut) *errorOut = QStringLiteral("Помилка читання файлу користувачів.");
        return false;
    }

    const QString input = loginOrEmail.trimmed();
    QString key = input;
    if (input.contains(QLatin1Char('@'))) {
        const QString byEmail = findLoginByEmail(users, normalizeEmail(input));
        if (byEmail.isEmpty()) {
            if (errorOut) *errorOut = QStringLiteral("Користувача з таким email не знайдено.");
            return false;
        }
        key = byEmail;
    } else if (!validateLoginString(input, errorOut)) {
        return false;
    }

    for (const QJsonValue& v : users) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        if (o.value(QLatin1String("login")).toString().compare(key, Qt::CaseInsensitive) != 0) continue;

        if (passwordMatchesUser(o, password, errorOut)) {
            if (canonicalLoginOut) *canonicalLoginOut = o.value(QLatin1String("login")).toString();
            return true;
        }
        return false;
    }
    if (errorOut) *errorOut = QStringLiteral("Користувача не знайдено. Спочатку зареєструйтесь.");
    return false;
}

bool AuthStore::signInWithGoogle(bool legalDocumentsAccepted, QString* canonicalLoginOut, QString* errorOut) {
    if (!legalDocumentsAccepted) {
        if (errorOut) {
            *errorOut = QStringLiteral("Підтвердіть згоду з Умовами та Політикою перед входом через Google.");
        }
        return false;
    }
    if (!ensureAuthDirImpl()) {
        if (errorOut) *errorOut = QStringLiteral("Не вдалося створити каталог для облікових записів.");
        return false;
    }
    QJsonArray users;
    if (!loadUsersArray(users)) {
        if (errorOut) *errorOut = QStringLiteral("Помилка читання файлу користувачів.");
        return false;
    }
    const QString provider = QStringLiteral("google");
    for (const QJsonValue& v : users) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        if (o.value(QLatin1String("oauth_provider")).toString() == provider) {
            if (canonicalLoginOut) *canonicalLoginOut = o.value(QLatin1String("login")).toString();
            return true;
        }
    }
    const QString login = QStringLiteral("oauth_google");
    QJsonObject u;
    u[QLatin1String("login")] = login;
    u[QLatin1String("oauth_provider")] = provider;
    u[QLatin1String("folder")] = storageFolderForLogin(login);
    u[QLatin1String("consent_iso")] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    u[QLatin1String("email")] = QStringLiteral("google@caloriecalc.local");
    users.append(u);
    if (!saveUsersArray(users)) {
        if (errorOut) *errorOut = QStringLiteral("Не вдалося створити обліковий запис Google.");
        return false;
    }
    if (canonicalLoginOut) *canonicalLoginOut = login;
    return true;
}

bool AuthStore::signInWithApple(bool legalDocumentsAccepted, QString* canonicalLoginOut, QString* errorOut) {
    if (!legalDocumentsAccepted) {
        if (errorOut) {
            *errorOut = QStringLiteral("Підтвердіть згоду з Умовами та Політикою перед входом через Apple.");
        }
        return false;
    }
    if (!ensureAuthDirImpl()) {
        if (errorOut) *errorOut = QStringLiteral("Не вдалося створити каталог для облікових записів.");
        return false;
    }
    QJsonArray users;
    if (!loadUsersArray(users)) {
        if (errorOut) *errorOut = QStringLiteral("Помилка читання файлу користувачів.");
        return false;
    }
    const QString provider = QStringLiteral("apple");
    for (const QJsonValue& v : users) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        if (o.value(QLatin1String("oauth_provider")).toString() == provider) {
            if (canonicalLoginOut) *canonicalLoginOut = o.value(QLatin1String("login")).toString();
            return true;
        }
    }
    const QString login = QStringLiteral("oauth_apple");
    QJsonObject u;
    u[QLatin1String("login")] = login;
    u[QLatin1String("oauth_provider")] = provider;
    u[QLatin1String("folder")] = storageFolderForLogin(login);
    u[QLatin1String("consent_iso")] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    u[QLatin1String("email")] = QStringLiteral("apple@caloriecalc.local");
    users.append(u);
    if (!saveUsersArray(users)) {
        if (errorOut) *errorOut = QStringLiteral("Не вдалося створити обліковий запис Apple.");
        return false;
    }
    if (canonicalLoginOut) *canonicalLoginOut = login;
    return true;
}

QString AuthStore::emailForLogin(const QString& canonicalLogin) {
    QJsonArray users;
    if (!loadUsersArray(users)) return {};
    const QString key = canonicalLogin.trimmed();
    for (const QJsonValue& v : users) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        if (o.value(QLatin1String("login")).toString().compare(key, Qt::CaseInsensitive) == 0) {
            return o.value(QLatin1String("email")).toString();
        }
    }
    return {};
}

bool AuthStore::deleteAccount(const QString& login, const QString& password, QString* errorOut) {
  QString canonical;
  if (!verifyLogin(login, password, errorOut, &canonical)) return false;
  const QString key = canonical.isEmpty() ? login.trimmed() : canonical;

    QJsonArray users;
    if (!loadUsersArray(users)) {
        if (errorOut) *errorOut = QStringLiteral("Помилка читання файлу користувачів.");
        return false;
    }

    for (int i = 0; i < users.size(); ++i) {
        if (!users[i].isObject()) continue;
        const QJsonObject o = users[i].toObject();
        if (o.value(QLatin1String("login")).toString().compare(key, Qt::CaseInsensitive) != 0) continue;

        const QString folder = o.value(QLatin1String("folder")).toString();
        if (!folder.isEmpty()) {
            const QString dataPath = QDir(QDir::currentPath()).filePath(QStringLiteral("data/%1").arg(folder));
            QDir dir(dataPath);
            if (dir.exists()) {
                if (!dir.removeRecursively()) {
                    if (errorOut) *errorOut = QStringLiteral("Не вдалося повністю видалити локальні дані користувача.");
                    return false;
                }
            }
        }

        users.removeAt(i);
        if (!saveUsersArray(users)) {
            if (errorOut) *errorOut = QStringLiteral("Не вдалося оновити список користувачів після видалення.");
            return false;
        }
        return true;
    }

    if (errorOut) *errorOut = QStringLiteral("Користувача не знайдено.");
    return false;
}

bool AuthStore::changePassword(const QString& login,
                               const QString& currentPassword,
                               const QString& newPassword,
                               QString* errorOut) {
    if (!validateLoginString(login, errorOut)) return false;
    if (currentPassword.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Введіть поточний пароль.");
        return false;
    }
    if (newPassword.size() < 4) {
        if (errorOut) *errorOut = QStringLiteral("Новий пароль: мінімум 4 символи.");
        return false;
    }
    if (newPassword == currentPassword) {
        if (errorOut) *errorOut = QStringLiteral("Новий пароль має відрізнятися від поточного.");
        return false;
    }

    QJsonArray users;
    if (!loadUsersArray(users)) {
        if (errorOut) *errorOut = QStringLiteral("Помилка читання файлу користувачів.");
        return false;
    }

    const QString key = login.trimmed();
    for (int i = 0; i < users.size(); ++i) {
        if (!users[i].isObject()) continue;
        QJsonObject o = users[i].toObject();
        if (o.value(QLatin1String("login")).toString().compare(key, Qt::CaseInsensitive) != 0) continue;

        const QByteArray oldSalt = QByteArray::fromHex(o.value(QLatin1String("salt_hex")).toString().toLatin1());
        const QString oldStoredHash = o.value(QLatin1String("hash_hex")).toString();
        if (oldSalt.isEmpty()) {
            if (errorOut) *errorOut = QStringLiteral("Пошкоджені дані користувача.");
            return false;
        }
        const QString currentHash = QString::fromLatin1(hashPassword(currentPassword, oldSalt));
        if (currentHash.compare(oldStoredHash, Qt::CaseInsensitive) != 0) {
            if (errorOut) *errorOut = QStringLiteral("Поточний пароль невірний.");
            return false;
        }

        QByteArray newSalt(16, 0);
        for (int k = 0; k < newSalt.size(); ++k) {
            newSalt[k] = char(QRandomGenerator::global()->bounded(256));
        }
        o[QLatin1String("salt_hex")] = QString::fromLatin1(newSalt.toHex());
        o[QLatin1String("hash_hex")] = QString::fromLatin1(hashPassword(newPassword, newSalt));
        users[i] = o;

        if (!saveUsersArray(users)) {
            if (errorOut) *errorOut = QStringLiteral("Не вдалося зберегти новий пароль.");
            return false;
        }
        return true;
    }

    if (errorOut) *errorOut = QStringLiteral("Користувача не знайдено.");
    return false;
}
