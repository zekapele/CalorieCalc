#include "TrainingDiary.h"

void TrainingDiary::addSession(const TrainingSession& session) {
    sessions_.push_back(session);
}

void TrainingDiary::removeSession(int index) {
    if (index >= 0 && index < static_cast<int>(sessions_.size())) {
        sessions_.erase(sessions_.begin() + index);
    }
}

void TrainingDiary::replaceSession(int index, const TrainingSession& session) {
    if (index >= 0 && index < static_cast<int>(sessions_.size())) {
        sessions_[static_cast<size_t>(index)] = session;
    }
}

int TrainingDiary::getTotalDurationMin() const {
    int total = 0;
    for (const auto& s : sessions_) {
        total += s.getDurationMin();
    }
    return total;
}

