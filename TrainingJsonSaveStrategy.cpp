#include "TrainingJsonSaveStrategy.h"

#include "TrainingSession.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace {

QJsonObject sessionToJson(const TrainingSession& s) {
    QJsonObject o;
    o[QStringLiteral("type")] = QString::fromStdString(s.getType());
    o[QStringLiteral("duration_min")] = s.getDurationMin();
    o[QStringLiteral("time_hhmm")] = QString::fromStdString(s.getTimeHHmm());
    o[QStringLiteral("status")] = QString::fromStdString(TrainingSession::statusToString(s.getStatus()));
    o[QStringLiteral("notes")] = QString::fromStdString(s.getNotes());
    return o;
}

TrainingSession sessionFromJson(const QJsonObject& o) {
    const QString type = o.value(QStringLiteral("type")).toString(QStringLiteral("Cardio"));
    const int duration = o.value(QStringLiteral("duration_min")).toInt(0);
    const QString time = o.value(QStringLiteral("time_hhmm")).toString(QStringLiteral("07:30"));
    const QString statusStr = o.value(QStringLiteral("status")).toString(QStringLiteral("Planned"));
    const QString notes = o.value(QStringLiteral("notes")).toString();
    return TrainingSession(type.toStdString(),
                           duration,
                           time.toStdString(),
                           TrainingSession::statusFromString(statusStr.toStdString()),
                           notes.toStdString());
}

} // namespace

bool TrainingJsonSaveStrategy::save(const TrainingDiary& diary, const std::string& filename) const {
    const QString full = QString::fromStdString(filename + getExtension());

    QJsonArray sessionsArray;
    for (const auto& s : diary.getAllSessions()) {
        sessionsArray.append(sessionToJson(s));
    }

    QJsonObject root;
    root[QStringLiteral("format_version")] = 1;
    root[QStringLiteral("sessions")] = sessionsArray;

    QFile file(full);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool TrainingJsonSaveStrategy::load(TrainingDiary& diary, const std::string& filename) const {
    const QString full = QString::fromStdString(filename + getExtension());

    QFile file(full);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    diary.clear();
    const QJsonObject root = doc.object();
    const QJsonValue sessionsVal = root.value(QStringLiteral("sessions"));
    if (!sessionsVal.isArray()) {
        return true;
    }

    const QJsonArray sessionsArray = sessionsVal.toArray();
    for (const QJsonValue& item : sessionsArray) {
        if (!item.isObject()) {
            continue;
        }
        diary.addSession(sessionFromJson(item.toObject()));
    }
    return true;
}
