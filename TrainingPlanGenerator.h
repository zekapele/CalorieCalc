#ifndef CALORIECALC_TRAININGPLANGENERATOR_H
#define CALORIECALC_TRAININGPLANGENERATOR_H

#include "TrainingSession.h"
#include <string>
#include <vector>

/**
 * @brief Параметри генерації плану тренувань (ціль, адаптація, профіль).
 */
struct TrainingPreferences {
    // e.g. "cutting" | "bulk" | "maintenance"
    std::string goal;
    /** Адаптивна зміна тривалості сесій: -1 делoad, 0 базово, 1–2 легкий прогресивний overload (по +5 хв/рівень). */
    int adaptationVolume = 0;
    /** Персональні дані профілю; 0 / 0.0 = не враховувати. */
    int ageYears = 0;
    double heightCm = 0.0;
    double weightKg = 0.0;
};

/**
 * @brief Детермінований генератор тренувального плану на тиждень.
 */
class TrainingPlanGenerator {
public:
    /** @brief Кілька сесій на один день (детерміновано за dayIndex і ціллю). */
    std::vector<TrainingSession> generateSessionsForDay(int dayIndex0to6, const TrainingPreferences& prefs) const;

    /** Перша непорожня сесія дня (зворотна сумісність). */
    TrainingSession generateSessionForDay(int dayIndex0to6, const TrainingPreferences& prefs) const;

    /** Усі сесії тижня (7 днів), плоский список. */
    std::vector<TrainingSession> generateWeek(const TrainingPreferences& prefs) const;

    /** Те саме, що generateWeek, але дні 0…6 обчислюються паралельно. */
    std::vector<TrainingSession> generateWeekParallel(const TrainingPreferences& prefs,
                                                    unsigned threadCount = 0) const;
};

#endif //CALORIECALC_TRAININGPLANGENERATOR_H

