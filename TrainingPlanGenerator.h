#ifndef CALORIECALC_TRAININGPLANGENERATOR_H
#define CALORIECALC_TRAININGPLANGENERATOR_H

#include "TrainingSession.h"
#include <string>
#include <vector>

struct TrainingPreferences {
    // e.g. "cutting" | "bulk" | "maintenance"
    std::string goal;
};

class TrainingPlanGenerator {
public:
    // Generates at most one session per day (can be extended later).
    TrainingSession generateSessionForDay(int dayIndex0to6, const TrainingPreferences& prefs) const;

    // Helper to generate the whole 7-day week.
    std::vector<TrainingSession> generateWeek(const TrainingPreferences& prefs) const;
};

#endif //CALORIECALC_TRAININGPLANGENERATOR_H

