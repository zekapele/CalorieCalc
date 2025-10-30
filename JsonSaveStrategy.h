#ifndef CALORIECALC_JSONSAVESTRATEGY_H
#define CALORIECALC_JSONSAVESTRATEGY_H

#include "ISaveStrategy.h"
#include <string>

// Конкретна реалізація стратегії збереження (JSON формат)
// Демонстрація наслідування та поліморфізму
class JsonSaveStrategy : public ISaveStrategy {
public:
    bool save(const Diary& diary, const std::string& filename) const override;
    bool load(Diary& diary, const std::string& filename) const override;
    std::string getExtension() const override { return ".json"; }
};

#endif //CALORIECALC_JSONSAVESTRATEGY_H


