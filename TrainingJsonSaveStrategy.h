#ifndef CALORIECALC_TRAININGJSONSAVESTRATEGY_H
#define CALORIECALC_TRAININGJSONSAVESTRATEGY_H

#include "TrainingDiary.h"
#include <string>

// JSON persistence for TrainingDiary (Qt QJsonDocument — коректне екранування рядків).
class TrainingJsonSaveStrategy {
public:
    bool save(const TrainingDiary& diary, const std::string& filename) const;
    bool load(TrainingDiary& diary, const std::string& filename) const;

    // Returned extension (included by caller if needed).
    std::string getExtension() const { return "_training.json"; }
};

#endif //CALORIECALC_TRAININGJSONSAVESTRATEGY_H

