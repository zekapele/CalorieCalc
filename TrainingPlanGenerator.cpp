#include "TrainingPlanGenerator.h"

TrainingSession TrainingPlanGenerator::generateSessionForDay(int dayIndex0to6, const TrainingPreferences& prefs) const {
    // We keep it deterministic to be demonstrable in a classroom.
    // dayIndex0to6: 0 = selected date, 6 = selected date + 6
    const std::string goal = prefs.goal;

    auto mk = [&](const std::string& type, int durationMin) {
        // Default time in day plan (can be edited by user later).
        return TrainingSession(type, durationMin, "07:30", TrainingSession::Status::Planned, "");
    };

    if (goal == "cutting") {
        if (dayIndex0to6 == 0) return mk("Cardio", 35);
        if (dayIndex0to6 == 2) return mk("Strength", 50);
        if (dayIndex0to6 == 3) return mk("Cardio", 30);
        if (dayIndex0to6 == 5) return mk("Strength", 45);
        if (dayIndex0to6 == 6) return mk("Mobility", 25);
        // rest days (no session)
        return TrainingSession("Rest", 0, "00:00", TrainingSession::Status::Planned, "");
    }

    if (goal == "bulk") {
        if (dayIndex0to6 == 0) return mk("Strength", 55);
        if (dayIndex0to6 == 1) return mk("Strength", 50);
        if (dayIndex0to6 == 3) return mk("Strength", 55);
        if (dayIndex0to6 == 5) return mk("Cardio", 25);
        if (dayIndex0to6 == 6) return mk("Mobility", 20);
        return TrainingSession("Rest", 0, "00:00", TrainingSession::Status::Planned, "");
    }

    // maintenance (default)
    if (dayIndex0to6 == 0) return mk("Cardio", 30);
    if (dayIndex0to6 == 1) return mk("Strength", 45);
    if (dayIndex0to6 == 3) return mk("Strength", 50);
    if (dayIndex0to6 == 4) return mk("Cardio", 25);
    if (dayIndex0to6 == 6) return mk("Mobility", 25);
    return TrainingSession("Rest", 0, "00:00", TrainingSession::Status::Planned, "");
}

std::vector<TrainingSession> TrainingPlanGenerator::generateWeek(const TrainingPreferences& prefs) const {
    std::vector<TrainingSession> out;
    out.reserve(7);
    for (int i = 0; i < 7; ++i) {
        out.push_back(generateSessionForDay(i, prefs));
    }
    return out;
}

