#ifndef CALORIECALC_XMLSAVESTRATEGY_H
#define CALORIECALC_XMLSAVESTRATEGY_H

#include "ISaveStrategy.h"
#include <string>

/**
 * @brief XML стратегія збереження
 * 
 * Зберігає дані у XML форматі для сумісності з іншими системами
 */
class XmlSaveStrategy : public ISaveStrategy {
public:
    bool save(const Diary& diary, const std::string& filename) const override;
    bool load(Diary& diary, const std::string& filename) const override;
    std::string getExtension() const override { return ".xml"; }
};

#endif //CALORIECALC_XMLSAVESTRATEGY_H

