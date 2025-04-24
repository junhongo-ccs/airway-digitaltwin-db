/**
 * @file logger.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-03
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <cstdio>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

namespace spatialid {

/**
 * @brief ログレベルの種類
 *
 */
enum class LogLevel : int {
    kNone = 0,  // ログなし
    kError,     // エラーログ
    kWarn,      // 警告ログ
    kInfo,      // 情報ログ
    kDebug,     // デバッグログ
    kVerbose,   // 詳細ログ
};

// ログ出力先設定マクロ
#define LOG_FILENAME(filename) Logger::SetFilename(filename)

// ログレベル設定マクロ
#define LOG_LEVEL_NONE() Logger::SetLogLevel(LogLevel::kNone)
#define LOG_LEVEL_ERROR() Logger::SetLogLevel(LogLevel::kError)
#define LOG_LEVEL_WARN() Logger::SetLogLevel(LogLevel::kWarn)
#define LOG_LEVEL_INFO() Logger::SetLogLevel(LogLevel::kInfo)
#define LOG_LEVEL_DEBUG() Logger::SetLogLevel(LogLevel::kDebug)
#define LOG_LEVEL_VERBOSE() Logger::SetLogLevel(LogLevel::kVerbose)

#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

// エラーログ出力マクロ
#define LOG_ERROR(format, ...)                                             \
    if (Logger::IsOutput(LogLevel::kError))                                \
    Logger::GetInstance().Write(LogLevel::kError, __FILE_NAME__, __func__, \
                                __LINE__, format, ##__VA_ARGS__)

// 警告ログ出力マクロ
#define LOG_WARN(format, ...)                                             \
    if (Logger::IsOutput(LogLevel::kWarn))                                \
    Logger::GetInstance().Write(LogLevel::kWarn, __FILE_NAME__, __func__, \
                                __LINE__, format, ##__VA_ARGS__)

// 情報ログ出力マクロ
#define LOG_INFO(format, ...)                                             \
    if (Logger::IsOutput(LogLevel::kInfo))                                \
    Logger::GetInstance().Write(LogLevel::kInfo, __FILE_NAME__, __func__, \
                                __LINE__, format, ##__VA_ARGS__)

// デバッグログ出力マクロ
#define LOG_DEBUG(format, ...)                                             \
    if (Logger::IsOutput(LogLevel::kInfo))                                 \
    Logger::GetInstance().Write(LogLevel::kDebug, __FILE_NAME__, __func__, \
                                __LINE__, format, ##__VA_ARGS__)
#define LOG_DEBUG_VECTOR_STRING(list, format, ...)                            \
    if (Logger::IsOutput(LogLevel::kInfo))                                    \
    Logger::GetInstance().WriteVectorString(LogLevel::kDebug, __FILE_NAME__,  \
                                            __func__, __LINE__, list, format, \
                                            ##__VA_ARGS__)
#define LOG_DEBUG_SET_STRING(list, format, ...)                            \
    if (Logger::IsOutput(LogLevel::kInfo))                                 \
    Logger::GetInstance().WriteSetString(LogLevel::kDebug, __FILE_NAME__,  \
                                         __func__, __LINE__, list, format, \
                                         ##__VA_ARGS__)
#define LOG_DEBUG_UNORDERED_SET_STRING(list, format, ...)                  \
    if (Logger::IsOutput(LogLevel::kInfo))                                 \
    Logger::GetInstance().WriteUnorderedSetString(                         \
        LogLevel::kDebug, __FILE_NAME__, __func__, __LINE__, list, format, \
        ##__VA_ARGS__)
#define LOG_DEBUG_POINT(x, y, z, format, ...)                               \
    if (Logger::IsOutput(LogLevel::kInfo))                                  \
    Logger::GetInstance().WritePointString(LogLevel::kDebug, __FILE_NAME__, \
                                           __func__, __LINE__, x, y, z,     \
                                           format, ##__VA_ARGS__)

// 詳細ログ出力マクロ
#define LOG_VERBOSE(format, ...)                                             \
    if (Logger::IsOutput(LogLevel::kVerbose))                                \
    Logger::GetInstance().Write(LogLevel::kVerbose, __FILE_NAME__, __func__, \
                                __LINE__, format, ##__VA_ARGS__)

/**
 * @brief ログ出力クラス
 *
 */
class Logger {
   private:
    static std::mutex mutex_;
    static std::string filename_;
    static LogLevel log_level_;
    std::FILE *fp_;

   protected:
    /**
     * @brief Construct a new Logger object
     *
     */
    Logger() : fp_(nullptr) {
#ifdef _MSC_VER
        if (fopen_s(&fp_, filename_.c_str(), "a") != 0) {
            fp_ = nullptr;
        }
#else
        fp_ = std::fopen(filename_.c_str(), "a");
#endif
        if (!fp_) {
            fp_ = stdout;
        }
    }

    /**
     * @brief Destroy the Logger object
     *
     */
    virtual ~Logger() {
        if (fp_ != stdout) {
            std::fclose(fp_);
        }
    }

   public:
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger &operator=(Logger &&) = delete;

    /**
     * @brief Get the Instance object
     *
     * @return Logger&
     */
    static Logger &GetInstance();

    /**
     * @brief ログファイル名を設定する
     *
     * @param filename
     */
    static void SetFilename(const std::string &filename) {
        filename_ = filename;
    };

    /**
     * @brief ログレベルを設定する
     *
     * @param log_level
     */
    static void SetLogLevel(LogLevel log_level) { log_level_ = log_level; }

    /**
     * @brief ログを出力するか
     *
     * @param log_level
     * @return true
     * @return false
     */
    static bool IsOutput(LogLevel log_level) {
        return !(static_cast<int>(log_level_) < static_cast<int>(log_level));
    }

    /**
     * @brief ログを出力する
     *
     * @param log_level
     * @param file_name
     * @param func_name
     * @param line
     * @param format
     * @param ...
     */
    void Write(LogLevel log_level, const char *file_name, const char *func_name,
               int line, const char *format, ...);

    /**
     * @brief ログを出力する(std::vector<std::string>)
     *
     * @param log_level
     * @param file_name
     * @param func_name
     * @param line
     * @param list
     * @param format
     * @param ...
     */
    void WriteVectorString(LogLevel log_level, const char *file_name,
                           const char *func_name, int line,
                           const std::vector<std::string> &list,
                           const char *format, ...);

    /**
     * @brief ログを出力する(std::set<std::string>)
     *
     * @param log_level
     * @param file_name
     * @param func_name
     * @param line
     * @param list
     * @param format
     * @param ...
     */
    void WriteSetString(LogLevel log_level, const char *file_name,
                        const char *func_name, int line,
                        const std::set<std::string> &list, const char *format,
                        ...);

    /**
     * @brief ログを出力する(std::unordered_set<std::string>)
     *
     * @param log_level
     * @param file_name
     * @param func_name
     * @param line
     * @param list
     * @param format
     * @param ...
     */
    void WriteUnorderedSetString(LogLevel log_level, const char *file_name,
                                 const char *func_name, int line,
                                 const std::unordered_set<std::string> &list,
                                 const char *format, ...);

    /**
     * @brief ログを出力する(x,y,z)
     *
     * @param log_level
     * @param file_name
     * @param func_name
     * @param line
     * @param x
     * @param y
     * @param z
     * @param format
     * @param ...
     */
    void WritePointString(LogLevel log_level, const char *file_name,
                          const char *func_name, int line, double x, double y,
                          double z, const char *format, ...);

   private:
    /**
     * @brief ログレベルに応じた文字列を取得する
     *
     * @param log_level
     * @return std::string
     */
    static std::string ToString(LogLevel log_level);

    /**
     * @brief ログヘッダーを出力する
     *
     * @param log_level
     * @param file_name
     * @param func_name
     * @param line
     */
    void WriteHeader(LogLevel log_level, const char *file_name,
                     const char *func_name, int line);
};

}  // namespace spatialid

#endif  // __LOGGER_H__
