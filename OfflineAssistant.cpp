#include "OfflineAssistant.h"

#include <algorithm>
#include <sstream>

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string OfflineAssistant::getRecommendation(const std::string& query,
                                                const Diary& diary,
                                                const TrainingDiary& training,
                                                const TrainingPreferences& prefs) const {
    const std::string q = toLower(query);

    std::ostringstream out;

    const double remaining = diary.getRemainingCalories();
    const double goal = diary.getCalorieGoal();
    const int waterMl = diary.getWaterMl();
    const int waterGoal = diary.getWaterGoalMl();
    const double weight = diary.getWeightKg();
    const int trainingMin = training.getTotalDurationMin();

    if (q.find("калор") != std::string::npos || q.find("kcal") != std::string::npos) {
        out << "Калорії на сьогодні: ціль " << goal << " ккал. " 
            << "Залишок: " << remaining << " ккал.\n";
        if (remaining < 0) {
            out << "Ви вже перевищили ціль. Спробуйте в наступному прийомі їжі зменшити порцію або додати овочі/білок.\n";
        } else if (remaining > 200) {
            out << "Є запас. Можете планувати легкий прийом їжі з достатнім білком і клітковиною.\n";
        } else {
            out << "Ви близько до цілі. Слідкуйте за порціями та макроелементами.\n";
        }
        return out.str();
    }

    if (q.find("вода") != std::string::npos || q.find("water") != std::string::npos) {
        out << "Вода: " << waterMl << " / " << waterGoal << " мл.\n";
        if (waterGoal > 0 && waterMl < waterGoal * 0.5) {
            out << "Рекомендація: випийте 250-500 мл протягом найближчої години.\n";
        } else if (waterGoal > 0 && waterMl < waterGoal) {
            out << "Рекомендація: добийте ціль невеликими порціями протягом дня.\n";
        } else {
            out << "Ціль води майже/виконана. Підтримуйте регулярний питний режим.\n";
        }
        return out.str();
    }

    if (q.find("трен") != std::string::npos || q.find("workout") != std::string::npos || q.find("вправа") != std::string::npos) {
        out << "Сьогодні в тренуваннях: " << trainingMin << " хв.\n";
        if (trainingMin <= 0) {
            out << "Спробуйте 20-40 хв легкого кардіо або мобільність (розігрів + легка активність).\n";
        } else {
            out << "Ви вже тренувались. Додайте 5-10 хв розминки/заминки і контролюйте відновлення.\n";
        }

        if (prefs.goal == "cutting") {
            out << "Ціль (схуднення): тримайте помірний дефіцит калорій та пріоритет на білок.\n";
        } else if (prefs.goal == "bulk") {
            out << "Ціль (набір): зверніть увагу на сон і достатню кількість білка/калорій.\n";
        } else {
            out << "Ціль (підтримка): 1-2 силові + легке кардіо 1-2 рази на тиждень зазвичай працюють стабільно.\n";
        }
        if (weight > 0) out << "Вага: " << weight << " кг.\n";
        return out.str();
    }

    // Fallback generic advice using the available data.
    out << "Я можу порадити по харчуванню та тренуванням. Спробуйте запит типу \"калорії\", \"вода\" або \"тренування\".\n";
    out << "Поточні дані: залишок калорій " << remaining << " ккал, вода " << waterMl << "/" << waterGoal << " мл.\n";
    out << "Тренування сьогодні: " << trainingMin << " хв.\n";
    return out.str();
}

