#ifndef CALORIECALC_ISAVESTRATEGY_H
#define CALORIECALC_ISAVESTRATEGY_H

#include "Diary.h"
#include <string>

/**
 * @brief Інтерфейс стратегії збереження щоденника (патерн Strategy, DIP).
 *
 * Клієнтський код працює з ISaveStrategy і не залежить від JSON, TXT, XML чи SQLite.
 */
class ISaveStrategy {
public:
    virtual ~ISaveStrategy() = default;

    /**
     * @brief Зберегти щоденник у файл (без розширення — додає getExtension()).
     * @return false при помилці I/O або серіалізації
     */
    virtual bool save(const Diary& diary, const std::string& filename) const = 0;

    /**
     * @brief Завантажити щоденник з файлу.
     * @return false якщо файл відсутній або дані не прочитано
     */
    virtual bool load(Diary& diary, const std::string& filename) const = 0;

    /** @brief Розширення файлу, наприклад ".json" */
    virtual std::string getExtension() const = 0;
};

#endif //CALORIECALC_ISAVESTRATEGY_H


