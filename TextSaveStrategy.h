#ifndef CALORIECALC_TEXTSAVESTRATEGY_H
#define CALORIECALC_TEXTSAVESTRATEGY_H

#include "ISaveStrategy.h"
#include <string>

// Альтернативна реалізація стратегії збереження (текстовий формат)
// Демонстрація Strategy pattern
class TextSaveStrategy : public ISaveStrategy {
public:
    bool save(const Diary& diary, const std::string& filename) const override;
    bool load(Diary& diary, const std::string& filename) const override;
    std::string getExtension() const override { return ".txt"; }
};

#endif //CALORIECALC_TEXTSAVESTRATEGY_H


