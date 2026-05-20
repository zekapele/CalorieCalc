#include "TrainingPlanGenerator.h"
#include "Parallel.h"

#include <algorithm>
#include <array>
#include <future>

namespace {

/** Корекція тривалості за віком, BMI та серією виконаних сесій. */
int profileDurationMultiplierPercent(const TrainingPreferences& prefs) {
    int pct = 100;
    if (prefs.ageYears > 55) pct -= 10;
    else if (prefs.ageYears > 45) pct -= 5;
    else if (prefs.ageYears > 0 && prefs.ageYears < 22) pct -= 5;

    if (prefs.heightCm > 50.0 && prefs.weightKg > 0.0) {
        const double hM = prefs.heightCm / 100.0;
        const double bmi = prefs.weightKg / (hM * hM);
        if (bmi >= 30.0) pct -= 8;
        else if (bmi < 18.5) pct -= 5;
    }
    return std::clamp(pct, 80, 110);
}

TrainingSession sessionPlanned(const std::string& type,
                               int durationMin,
                               const char* timeHHmm,
                               const TrainingPreferences& prefs) {
    int d = durationMin;
    if (d > 0) {
        const int adapt = std::clamp(prefs.adaptationVolume, -1, 2);
        d = d + adapt * 5;
        d = (d * profileDurationMultiplierPercent(prefs)) / 100;
        d = std::clamp(d, 10, 120);
    }
    return TrainingSession(type, d, std::string(timeHHmm), TrainingSession::Status::Planned, "");
}

} // namespace

std::vector<TrainingSession> TrainingPlanGenerator::generateSessionsForDay(int dayIndex0to6,
                                                                          const TrainingPreferences& prefs) const {
    std::vector<TrainingSession> out;
    const std::string goal = prefs.goal;

    if (goal == "cutting") {
        if (dayIndex0to6 == 0) {
            out.push_back(sessionPlanned("Cardio", 30, "07:30", prefs));
            out.push_back(sessionPlanned("Mobility", 15, "19:00", prefs));
            return out;
        }
        if (dayIndex0to6 == 2) {
            out.push_back(sessionPlanned("Strength", 50, "18:00", prefs));
            return out;
        }
        if (dayIndex0to6 == 3) {
            out.push_back(sessionPlanned("Cardio", 28, "07:15", prefs));
            return out;
        }
        if (dayIndex0to6 == 5) {
            out.push_back(sessionPlanned("Strength", 42, "17:45", prefs));
            return out;
        }
        if (dayIndex0to6 == 6) {
            out.push_back(sessionPlanned("Mobility", 22, "09:00", prefs));
            return out;
        }
        return out;
    }

    if (goal == "bulk") {
        if (dayIndex0to6 == 0) {
            out.push_back(sessionPlanned("Strength", 45, "08:00", prefs));
            out.push_back(sessionPlanned("Cardio", 18, "19:30", prefs));
            return out;
        }
        if (dayIndex0to6 == 1) {
            out.push_back(sessionPlanned("Strength", 50, "18:00", prefs));
            return out;
        }
        if (dayIndex0to6 == 3) {
            out.push_back(sessionPlanned("Strength", 52, "08:15", prefs));
            return out;
        }
        if (dayIndex0to6 == 5) {
            out.push_back(sessionPlanned("Cardio", 22, "07:30", prefs));
            return out;
        }
        if (dayIndex0to6 == 6) {
            out.push_back(sessionPlanned("Mobility", 20, "10:00", prefs));
            return out;
        }
        return out;
    }

    // maintenance
    if (dayIndex0to6 == 0) {
        out.push_back(sessionPlanned("Cardio", 28, "07:20", prefs));
        return out;
    }
    if (dayIndex0to6 == 1) {
        out.push_back(sessionPlanned("Strength", 40, "12:00", prefs));
        out.push_back(sessionPlanned("Mobility", 18, "20:00", prefs));
        return out;
    }
    if (dayIndex0to6 == 3) {
        out.push_back(sessionPlanned("Strength", 48, "18:30", prefs));
        return out;
    }
    if (dayIndex0to6 == 4) {
        out.push_back(sessionPlanned("Cardio", 24, "07:00", prefs));
        return out;
    }
    if (dayIndex0to6 == 6) {
        out.push_back(sessionPlanned("Mobility", 25, "09:30", prefs));
        return out;
    }
    return out;
}

TrainingSession TrainingPlanGenerator::generateSessionForDay(int dayIndex0to6, const TrainingPreferences& prefs) const {
    for (const auto& s : generateSessionsForDay(dayIndex0to6, prefs)) {
        if (s.getDurationMin() > 0 && s.getType() != "Rest") {
            return s;
        }
    }
    return TrainingSession("Rest", 0, "00:00", TrainingSession::Status::Planned, "");
}

std::vector<TrainingSession> TrainingPlanGenerator::generateWeek(const TrainingPreferences& prefs) const {
    std::vector<TrainingSession> out;
    out.reserve(14);
    for (int i = 0; i < 7; ++i) {
        for (const auto& s : generateSessionsForDay(i, prefs)) {
            out.push_back(s);
        }
    }
    return out;
}

std::vector<TrainingSession> TrainingPlanGenerator::generateWeekParallel(const TrainingPreferences& prefs,
                                                                         const unsigned threadCount) const {
    const unsigned threads = Parallel::threadCount(threadCount, 7);
    if (threads <= 1) {
        return generateWeek(prefs);
    }

    std::array<std::vector<TrainingSession>, 7> perDay;
    std::vector<std::future<void>> futures;
    futures.reserve(7);

    for (int day = 0; day < 7; ++day) {
        futures.push_back(std::async(std::launch::async, [this, &prefs, &perDay, day]() {
            perDay[static_cast<size_t>(day)] = generateSessionsForDay(day, prefs);
        }));
    }
    for (auto& f : futures) {
        f.get();
    }

    std::vector<TrainingSession> out;
    out.reserve(14);
    for (int day = 0; day < 7; ++day) {
        for (const auto& s : perDay[static_cast<size_t>(day)]) {
            out.push_back(s);
        }
    }
    return out;
}
