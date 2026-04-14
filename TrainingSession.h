#ifndef CALORIECALC_TRAININGSESSION_H
#define CALORIECALC_TRAININGSESSION_H

#include <string>

// Represents a single workout session.
class TrainingSession {
public:
    enum class Status {
        Planned,
        Completed
    };

    TrainingSession() = default;
    TrainingSession(std::string type, int durationMin, std::string timeHHmm, Status status, std::string notes);

    const std::string& getType() const { return type_; }
    int getDurationMin() const { return durationMin_; }
    const std::string& getTimeHHmm() const { return timeHHmm_; }
    Status getStatus() const { return status_; }
    const std::string& getNotes() const { return notes_; }

    // Helpers for JSON persistence (string representation).
    static std::string statusToString(Status s);
    static Status statusFromString(const std::string& s);

private:
    std::string type_;
    int durationMin_{};
    std::string timeHHmm_; // e.g. "07:30" (optional but stored)
    Status status_{Status::Planned};
    std::string notes_;
};

#endif //CALORIECALC_TRAININGSESSION_H

