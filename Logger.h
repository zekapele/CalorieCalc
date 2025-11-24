#ifndef CALORIECALC_LOGGER_H
#define CALORIECALC_LOGGER_H

#include <string>
#include <fstream>
#include <memory>
#include <mutex>

/**
 * @brief Простий логер для запису подій у файл
 * 
 * Підтримує різні рівні логування: DEBUG, INFO, WARNING, ERROR
 */
class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    /**
     * @brief Отримати єдиний екземпляр логера (Singleton)
     * @return Посилання на логер
     */
    static Logger& getInstance();

    /**
     * @brief Ініціалізувати логер з файлом
     * @param filename Ім'я файлу для логування
     * @param maxFileSize Максимальний розмір файлу в байтах (0 = без обмежень)
     * @param maxFiles Максимальна кількість файлів для ротації
     */
    void init(const std::string& filename, size_t maxFileSize = 10 * 1024 * 1024, size_t maxFiles = 5);

    /**
     * @brief Встановити мінімальний рівень логування
     * @param level Мінімальний рівень (повідомлення нижче цього рівня ігноруються)
     */
    void setMinLevel(Level level);

    /**
     * @brief Записати повідомлення
     * @param level Рівень логування
     * @param message Повідомлення
     */
    void log(Level level, const std::string& message);

    /**
     * @brief Закрити файл логування
     */
    void close();

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream logFile_;
    std::mutex mutex_;
    bool initialized_ = false;
    std::string logFilename_;
    size_t maxFileSize_;
    size_t maxFiles_;
    Level minLevel_ = Level::DEBUG;

    std::string levelToString(Level level) const;
    std::string getCurrentTime() const;
    void rotateLogFile();
};

// Макроси для зручності
#define LOG_DEBUG(msg) Logger::getInstance().log(Logger::Level::DEBUG, msg)
#define LOG_INFO(msg) Logger::getInstance().log(Logger::Level::INFO, msg)
#define LOG_WARNING(msg) Logger::getInstance().log(Logger::Level::WARNING, msg)
#define LOG_ERROR(msg) Logger::getInstance().log(Logger::Level::ERROR, msg)

#endif //CALORIECALC_LOGGER_H

