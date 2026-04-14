#include "TrainingSession.h"

TrainingSession::TrainingSession(std::string type, int durationMin, std::string timeHHmm, Status status, std::string notes)
    : type_(std::move(type)),
      durationMin_(durationMin),
      timeHHmm_(std::move(timeHHmm)),
      status_(status),
      notes_(std::move(notes)) {
}

std::string TrainingSession::statusToString(Status s) {
    switch (s) {
        case Status::Planned: return "planned";
        case Status::Completed: return "completed";
    }
    return "planned";
}

TrainingSession::Status TrainingSession::statusFromString(const std::string& s) {
    if (s == "completed") return Status::Completed;
    return Status::Planned;
}

