#ifndef CALORIECALC_ISAVESTRATEGY_H
#define CALORIECALC_ISAVESTRATEGY_H

#include "Diary.h"
#include <string>

// Абстрактний клас (інтерфейс) для стратегії збереження/завантаження
// Демонстрація принципу інверсії залежностей (Dependency Inversion Principle)
class ISaveStrategy {
public:
    virtual ~ISaveStrategy() = default;
    
    // Віртуальні методи - поліморфізм
    virtual bool save(const Diary& diary, const std::string& filename) const = 0;
    virtual bool load(Diary& diary, const std::string& filename) const = 0;
    virtual std::string getExtension() const = 0;
};

#endif //CALORIECALC_ISAVESTRATEGY_H


