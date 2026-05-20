#include "OfflineAssistant.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_map>
#include <vector>

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

static bool isStopWord(const std::string& w) {
    static const std::vector<std::string> stops = {
        "the", "a", "an", "in", "on", "of", "for", "how", "many", "much", "what", "is", "are",
        "calories", "calorie", "kcal", "nutrition", "food", "скільки", "калорій", "калорії",
        "калорія", "у", "в", "на", "про", "що", "як", "макро", "білок", "жир", "вуглеводи"};
    return std::find(stops.begin(), stops.end(), w) != stops.end();
}

static std::string englishFoodHint(const std::string& token) {
    static const std::unordered_map<std::string, std::string> map = {
        {"apple", "яблук"},       {"banana", "банан"},     {"chicken", "кур"},
        {"rice", "рис"},          {"egg", "яйц"},        {"milk", "молок"},
        {"bread", "хліб"},        {"potato", "картоп"},  {"tomato", "помід"},
        {"cucumber", "огір"},     {"beef", "ялов"},      {"pork", "свин"},
        {"salmon", "лосос"},      {"tuna", "тун"},       {"oat", "вівся"},
        {"yogurt", "йогур"},      {"cheese", "сир"},     {"broccoli", "брокол"},
        {"carrot", "моркв"},      {"orange", "апельс"},  {"pear", "груш"},
        {"honey", "мед"},         {"avocado", "авокад"},
    };
    const auto it = map.find(token);
    return it == map.end() ? std::string() : it->second;
}

static std::vector<std::string> searchTokensFromQuery(const std::string& qLower) {
    std::vector<std::string> tokens;
    std::string cur;
    for (char c : qLower) {
        if (std::isalnum(static_cast<unsigned char>(c)) || (c & 0x80)) {
            cur.push_back(c);
        } else if (!cur.empty()) {
            if (cur.size() >= 2 && !isStopWord(cur)) {
                tokens.push_back(cur);
            }
            cur.clear();
        }
    }
    if (!cur.empty() && cur.size() >= 2 && !isStopWord(cur)) {
        tokens.push_back(cur);
    }
    return tokens;
}

static std::string tryAnswerFoodQuestion(const std::string& qLower, const FoodDatabase& foodDb) {
    std::vector<std::string> tried;
    auto trySearch = [&](const std::string& term) -> std::vector<Food> {
        if (term.size() < 2) {
            return {};
        }
        if (std::find(tried.begin(), tried.end(), term) != tried.end()) {
            return {};
        }
        tried.push_back(term);
        return foodDb.searchFoodsParallel(term, 0);
    };

    std::vector<Food> hits;
    for (const auto& token : searchTokensFromQuery(qLower)) {
        auto part = trySearch(token);
        if (part.empty()) {
            const std::string hint = englishFoodHint(token);
            if (!hint.empty()) {
                part = trySearch(hint);
            }
        }
        for (const auto& f : part) {
            if (std::find_if(hits.begin(), hits.end(), [&](const Food& x) {
                    return x.getName() == f.getName();
                }) == hits.end()) {
                hits.push_back(f);
            }
        }
        if (hits.size() >= 5) {
            break;
        }
    }

    if (hits.empty()) {
        hits = trySearch(qLower);
    }

    if (hits.empty()) {
        return {};
    }

    std::ostringstream out;
    out << "Знайдено в базі продуктів (на 100 г):\n";
    const size_t limit = std::min<size_t>(hits.size(), 5);
    for (size_t i = 0; i < limit; ++i) {
        const Food& f = hits[i];
        out << "• " << f.getName() << ": " << f.getCalories() << " ккал, Б " << f.getProtein()
            << " г, Ж " << f.getFat() << " г, В " << f.getCarbs() << " г\n";
    }
    if (hits.size() > limit) {
        out << "… ще " << (hits.size() - limit) << " варіант(ів). Уточніть назву.\n";
    }
    return out.str();
}

std::string OfflineAssistant::getRecommendation(const std::string& query,
                                                const Diary& diary,
                                                const TrainingDiary& training,
                                                const TrainingPreferences& prefs,
                                                int activityStreakDays,
                                                const FoodDatabase* foodDb) const {
    const std::string q = toLower(query);

    if (foodDb) {
        const std::string foodAnswer = tryAnswerFoodQuestion(q, *foodDb);
        if (!foodAnswer.empty()) {
            return foodAnswer;
        }
    }

    std::ostringstream out;

    const double remaining = diary.getRemainingCalories();
    const double goal = diary.getCalorieGoal();
    const int waterMl = diary.getWaterMl();
    const int waterGoal = diary.getWaterGoalMl();
    const double weight = diary.getWeightKg();
    const int trainingMin = training.getTotalDurationMin();

    const double proteinGoal = diary.getProteinGoalG();
    const double carbGoal = diary.getCarbGoalG();
    const double fatGoal = diary.getFatGoalG();
    const double proteinEaten = diary.getTotalProtein();
    const double carbEaten = diary.getTotalCarbs();
    const double fatEaten = diary.getTotalFat();

    auto macroHint = [&](const char* name, double eaten, double macroGoalG) {
        if (macroGoalG <= 0.0) return std::string();
        const double left = macroGoalG - eaten;
        std::ostringstream m;
        m << name << ": " << eaten << " / " << macroGoalG << " г";
        if (left > 5) m << " (залишилось ~" << left << " г)";
        else if (left < -5) m << " (перевищення ~" << (-left) << " г)";
        else m << " (близько до цілі)";
        m << "\n";
        return m.str();
    };

    if (q.find("білок") != std::string::npos || q.find("protein") != std::string::npos) {
        out << macroHint("Білок", proteinEaten, proteinGoal);
        if (proteinEaten < proteinGoal * 0.7 && proteinGoal > 0)
            out << "Порада: додайте порцію нежирного білка (йогурт, філе, бобові) у наступний прийом їжі.\n";
        return out.str();
    }

    if (q.find("вуглевод") != std::string::npos || q.find("carb") != std::string::npos) {
        out << macroHint("Вуглеводи", carbEaten, carbGoal);
        if (carbEaten < carbGoal * 0.6 && carbGoal > 0)
            out << "Можна додати складні вуглеводи (крупи, батат) навколо тренування.\n";
        return out.str();
    }

    if (q.find("жир") != std::string::npos || q.find("fat") != std::string::npos) {
        out << macroHint("Жири", fatEaten, fatGoal);
        if (fatEaten < fatGoal * 0.6 && fatGoal > 0)
            out << "Додайте джерела корисних жирів (оріхи, оливкова олія) помірно.\n";
        return out.str();
    }

    if (q.find("макро") != std::string::npos || q.find("macro") != std::string::npos) {
        out << "Макроелементи за день:\n";
        out << macroHint("Білок", proteinEaten, proteinGoal);
        out << macroHint("Жири", fatEaten, fatGoal);
        out << macroHint("Вуглеводи", carbEaten, carbGoal);
        return out.str();
    }

    if (q.find("сері") != std::string::npos || q.find("streak") != std::string::npos ||
        q.find("активності") != std::string::npos) {
        if (activityStreakDays >= 0) {
            out << "Серія активності: " << activityStreakDays << " дн.\n";
            if (activityStreakDays >= 7)
                out << "Чудова регулярність. Плануйте легкий день відновлення, щоб не вигоріти.\n";
            else
                out << "Підтримуйте мінімум один запис харчування або 20+ хв тренувань щодня.\n";
        } else {
            out << "Серія активності недоступна в цьому контексті.\n";
        }
        return out.str();
    }

    if (q.find("підсумок") != std::string::npos || q.find("summary") != std::string::npos) {
        out << "Підсумок дня: калорії залишок " << remaining << " ккал, вода " << waterMl << "/" << waterGoal << " мл.\n";
        out << macroHint("Білок", proteinEaten, proteinGoal);
        out << "Тренування: " << trainingMin << " хв.\n";
        if (activityStreakDays >= 0) out << "Серія активності: " << activityStreakDays << " дн.\n";
        if (prefs.adaptationVolume != 0) {
            out << "Рівень адаптації плану (на основі виконаних сесій): " << prefs.adaptationVolume
                << " (впливає на тривалість автогенерації).\n";
        }
        return out.str();
    }

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
        if (prefs.adaptationVolume != 0) {
            out << "Адаптація плану за дисципліною: рівень " << prefs.adaptationVolume
                << " (коригує тривалість наступних автосесій).\n";
        }
        if (weight > 0) out << "Вага: " << weight << " кг.\n";
        return out.str();
    }

    out << "Не знайшов продукт у базі за цим запитом.\n";
    out << "Спробуйте українську назву (напр. «яблуко») або теми: калорії, вода, тренування, макро, підсумок.\n";
    out << "Поточні дані: залишок калорій " << remaining << " ккал, вода " << waterMl << "/" << waterGoal
        << " мл, тренування " << trainingMin << " хв.\n";
    if (foodDb && foodDb->size() > 0) {
        out << "Для хмарних відповідей задайте CALORIECALC_API_KEY у середовищі перед запуском.\n";
    }
    return out.str();
}

