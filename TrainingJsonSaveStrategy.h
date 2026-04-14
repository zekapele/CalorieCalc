#ifndef CALORIECALC_TRAININGJSONSAVESTRATEGY_H
#define CALORIECALC_TRAININGJSONSAVESTRATEGY_H

#include "TrainingDiary.h"
#include <string>

// Simple JSON persistence for TrainingDiary (no external JSON dependency).
class TrainingJsonSaveStrategy {
public:
    bool save(const TrainingDiary& diary, const std::string& filename) const;
    bool load(TrainingDiary& diary, const std::string& filename) const;

    // Returned extension (included by caller if needed).
    std::string getExtension() const { return "_training.json"; }
};

#endif //CALORIECALC_TRAININGJSONSAVESTRATEGY_H

