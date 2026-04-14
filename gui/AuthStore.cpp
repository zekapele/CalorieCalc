#include "AuthStore.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

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

} // namespace

QString AuthStore::storageFolderForLogin(const QString& login) {
    const QByteArray h = QCryptographicHash::hash(login.trimmed().toUtf8(), QCryptographicHash::Sha256);
    return QStringLiteral("u_%1").arg(QString::fromLatin1(h.toHex().left(16)));
}

bool AuthStore::registerUser(const QString& login, const QString& password, QString* errorOut) {
    if (!validateLoginString(login, errorOut)) return false;
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
    users.append(u);

    if (!saveUsersArray(users)) {
        if (errorOut) *errorOut = QStringLiteral("Не вдалося зберегти обліковий запис.");
        return false;
    }
    return true;
}

bool AuthStore::verifyLogin(const QString& login, const QString& password, QString* errorOut) {
    if (!validateLoginString(login, errorOut)) return false;
    if (password.isEmpty()) {
        if (errorOut) *errorOut = QStringLiteral("Введіть пароль.");
        return false;
    }

    QJsonArray users;
    if (!loadUsersArray(users)) {
        if (errorOut) *errorOut = QStringLiteral("Помилка читання файлу користувачів.");
        return false;
    }

    const QString key = login.trimmed();
    for (const QJsonValue& v : users) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();
        if (o.value(QLatin1String("login")).toString().compare(key, Qt::CaseInsensitive) != 0) continue;

        const QString saltHex = o.value(QLatin1String("salt_hex")).toString();
        const QString storedHash = o.value(QLatin1String("hash_hex")).toString();
        const QByteArray salt = QByteArray::fromHex(saltHex.toLatin1());
        if (salt.isEmpty()) {
            if (errorOut) *errorOut = QStringLiteral("Пошкоджені дані користувача.");
            return false;
        }
        const QString computed = QString::fromLatin1(hashPassword(password, salt));
        if (computed.compare(storedHash, Qt::CaseInsensitive) == 0) return true;
        if (errorOut) *errorOut = QStringLiteral("Невірний пароль.");
        return false;
    }
    if (errorOut) *errorOut = QStringLiteral("Користувача не знайдено. Спочатку зареєструйтесь.");
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
