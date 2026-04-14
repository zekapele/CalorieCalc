#ifndef CALORIECALC_TRAININGDIARY_H
#define CALORIECALC_TRAININGDIARY_H

#include "TrainingSession.h"
#include <vector>
#include <string>

// Stores all workout sessions for a single day.
class TrainingDiary {
public:
    void addSession(const TrainingSession& session);
    void removeSession(int index);
    std::vector<TrainingSession> getAllSessions() const { return sessions_; }

    // Convenience: total training time in minutes.
    int getTotalDurationMin() const;

    bool isEmpty() const { return sessions_.empty(); }
    void clear() { sessions_.clear(); }

private:
    std::vector<TrainingSession> sessions_;
};

#endif //CALORIECALC_TRAININGDIARY_H

