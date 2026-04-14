#ifndef CALORIECALC_OFFLINEASSISTANT_H
#define CALORIECALC_OFFLINEASSISTANT_H

#include "Diary.h"
#include "TrainingDiary.h"
#include "TrainingPlanGenerator.h"
#include <string>

// Offline fitness/nutrition assistant (no network).
class OfflineAssistant {
public:
    std::string getRecommendation(const std::string& query,
                                   const Diary& diary,
                                   const TrainingDiary& training,
                                   const TrainingPreferences& prefs) const;
};

#endif //CALORIECALC_OFFLINEASSISTANT_H

