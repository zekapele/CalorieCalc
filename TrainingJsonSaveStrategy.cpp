#include "TrainingJsonSaveStrategy.h"

#include "TrainingSession.h"

#include <fstream>
#include <iomanip>
#include <string>

static std::string extractValueAfterColon(const std::string& line) {
    auto pos = line.find(':');
    if (pos == std::string::npos) return {};
    return line.substr(pos + 1);
}

static std::string trimQuotesAndSpaces(std::string s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(s.begin());
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == ',' || s.back() == '\r' || s.back() == '\n'))
        s.pop_back();
    if (!s.empty() && s.front() == '"') s.erase(s.begin());
    if (!s.empty() && s.back() == '"') s.pop_back();
    return s;
}

bool TrainingJsonSaveStrategy::save(const TrainingDiary& diary, const std::string& filename) const {
    // 'filename' is without extension; we append "_training.json"
    const std::string full = filename + getExtension();
    std::ofstream file(full);
    if (!file.is_open()) return false;

    file << std::fixed << std::setprecision(2);
    file << "{\n";
    file << "  \"sessions\": [\n";

    auto sessions = diary.getAllSessions();
    for (size_t i = 0; i < sessions.size(); ++i) {
        const auto& s = sessions[i];
        file << "    {\n";
        file << "      \"type\": \"" << s.getType() << "\",\n";
        file << "      \"duration_min\": " << s.getDurationMin() << ",\n";
        file << "      \"time_hhmm\": \"" << s.getTimeHHmm() << "\",\n";
        file << "      \"status\": \"" << TrainingSession::statusToString(s.getStatus()) << "\",\n";
        file << "      \"notes\": \"" << s.getNotes() << "\"\n";
        file << "    }";
        if (i + 1 < sessions.size()) file << ",";
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";
    return true;
}

bool TrainingJsonSaveStrategy::load(TrainingDiary& diary, const std::string& filename) const {
    const std::string full = filename + getExtension();
    std::ifstream file(full);
    if (!file.is_open()) return false;

    diary.clear();

    std::string line;
    std::string type;
    std::string timeHHmm;
    std::string notes;
    int duration = 0;
    TrainingSession::Status status = TrainingSession::Status::Planned;

    bool haveType = false;
    bool haveTime = false;
    bool haveNotes = false;
    bool haveDuration = false;
    bool haveStatus = false;

    while (std::getline(file, line)) {
        if (line.find("\"type\"") != std::string::npos) {
            type = trimQuotesAndSpaces(extractValueAfterColon(line));
            haveType = true;
        } else if (line.find("\"duration_min\"") != std::string::npos) {
            auto value = extractValueAfterColon(line);
            duration = static_cast<int>(std::stod(value));
            haveDuration = true;
        } else if (line.find("\"time_hhmm\"") != std::string::npos) {
            timeHHmm = trimQuotesAndSpaces(extractValueAfterColon(line));
            haveTime = true;
        } else if (line.find("\"status\"") != std::string::npos) {
            auto statusStr = trimQuotesAndSpaces(extractValueAfterColon(line));
            status = TrainingSession::statusFromString(statusStr);
            haveStatus = true;
        } else if (line.find("\"notes\"") != std::string::npos) {
            notes = trimQuotesAndSpaces(extractValueAfterColon(line));
            haveNotes = true;
        }

        // When we have a full object - commit it on the first safe boundary.
        // In our format, all fields appear before the closing '}' of the session.
        if (haveType && haveTime && haveNotes && haveDuration && haveStatus) {
            diary.addSession(TrainingSession(type, duration, timeHHmm, status, notes));

            // Reset for next session.
            type.clear();
            timeHHmm.clear();
            notes.clear();
            duration = 0;
            status = TrainingSession::Status::Planned;

            haveType = haveTime = haveNotes = haveDuration = haveStatus = false;
        }
    }
    return true;
}

