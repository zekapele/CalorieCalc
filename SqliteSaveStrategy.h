#ifndef CALORIECALC_SQLITESAVESTRATEGY_H
#define CALORIECALC_SQLITESAVESTRATEGY_H

#ifdef HAVE_SQLITE3
#include "ISaveStrategy.h"
#include <string>

/**
 * @brief SQLite стратегія збереження
 * 
 * Зберігає дані у SQLite базі даних для швидкого пошуку
 * та структурованого зберігання.
 */
class SqliteSaveStrategy : public ISaveStrategy {
public:
    SqliteSaveStrategy();
    ~SqliteSaveStrategy();
    
    bool save(const Diary& diary, const std::string& filename) const override;
    bool load(Diary& diary, const std::string& filename) const override;
    std::string getExtension() const override { return ".db"; }

private:
    bool createTables(const std::string& dbPath) const;
};

#endif // HAVE_SQLITE3
#endif //CALORIECALC_SQLITESAVESTRATEGY_H
