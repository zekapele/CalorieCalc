#ifndef CALORIECALC_CSVIMPORTEXPORT_H
#define CALORIECALC_CSVIMPORTEXPORT_H

#include "Diary.h"
#include <string>
#include <vector>
#include <map>

/**
 * @brief Клас для імпорту та експорту даних у CSV формат
 */
class CsvImportExport {
public:
    /**
     * @brief Експортувати щоденники у CSV
     * @param diaries Мапа профіль -> (дата -> щоденник)
     * @param filename Ім'я файлу для експорту
     * @return true якщо успішно
     */
    static bool exportToCSV(const std::map<std::string, std::map<std::string, Diary>>& diaries, 
                           const std::string& filename);

    /**
     * @brief Імпортувати дані з CSV
     * @param filename Ім'я файлу для імпорту
     * @param diaries Мапа для збереження імпортованих даних
     * @return true якщо успішно
     */
    static bool importFromCSV(const std::string& filename,
                             std::map<std::string, std::map<std::string, Diary>>& diaries);

    /**
     * @brief Експортувати у формат для Excel (TSV)
     * @param diaries Дані для експорту
     * @param filename Ім'я файлу
     * @return true якщо успішно
     */
    static bool exportToTSV(const std::map<std::string, std::map<std::string, Diary>>& diaries,
                            const std::string& filename);
};

#endif //CALORIECALC_CSVIMPORTEXPORT_H

