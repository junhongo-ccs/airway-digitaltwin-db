/**
 * @file logger.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-03-03
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#include "logger.h"

#include <stdarg.h>

#include <chrono>
#include <iomanip>
#include <sstream>

namespace spatialid {

std::mutex Logger::mutex_;
std::string Logger::filename_;
#ifdef _DEBUG
LogLevel Logger::log_level_ = LogLevel::kDebug;
#else
LogLevel Logger::log_level_ = LogLevel::kInfo;
#endif

// インスタンスを取得する
Logger &Logger::GetInstance() {
    std::lock_guard<std::mutex> lock(mutex_);

    static Logger logger_;
    return logger_;
}

// ログを出力する
void Logger::Write(LogLevel log_level, const char *file_name,
                   const char *func_name, int line, const char *format, ...) {
    if (!Logger::IsOutput(log_level)) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    WriteHeader(log_level, file_name, func_name, line);

    va_list ap;
    va_start(ap, format);
    std::vfprintf(fp_, format, ap);
    va_end(ap);

    std::fprintf(fp_, "\n");
}

// ログを出力する(std::vector<std::string>)
void Logger::WriteVectorString(LogLevel log_level, const char *file_name,
                               const char *func_name, int line,
                               const std::vector<std::string> &list,
                               const char *format, ...) {
    if (!Logger::IsOutput(log_level)) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    WriteHeader(log_level, file_name, func_name, line);

    va_list ap;
    va_start(ap, format);
    std::vfprintf(fp_, format, ap);
    va_end(ap);

    std::stringstream ss;
    ss << "(";
    std::vector<std::string>::const_iterator it = list.begin();
    if (it != list.end()) {
        ss << *it;
        ++it;
    }
    for (; it != list.end(); ++it) {
        ss << "," << *it;
    }
    ss << ")";
    std::fprintf(fp_, "%s\n", ss.str().c_str());
}

// ログを出力する(std::set<std::string>)
void Logger::WriteSetString(LogLevel log_level, const char *file_name,
                            const char *func_name, int line,
                            const std::set<std::string> &list,
                            const char *format, ...) {
    if (!Logger::IsOutput(log_level)) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    WriteHeader(log_level, file_name, func_name, line);

    va_list ap;
    va_start(ap, format);
    std::vfprintf(fp_, format, ap);
    va_end(ap);

    std::stringstream ss;
    ss << "(";
    std::set<std::string>::const_iterator it = list.begin();
    if (it != list.end()) {
        ss << *it;
        ++it;
    }
    for (; it != list.end(); ++it) {
        ss << "," << *it;
    }
    ss << ")";
    std::fprintf(fp_, "%s\n", ss.str().c_str());
}

// ログを出力する(std::unordered_set<std::string>)
void Logger::WriteUnorderedSetString(
    LogLevel log_level, const char *file_name, const char *func_name, int line,
    const std::unordered_set<std::string> &list, const char *format, ...) {
    if (!Logger::IsOutput(log_level)) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    WriteHeader(log_level, file_name, func_name, line);

    va_list ap;
    va_start(ap, format);
    std::vfprintf(fp_, format, ap);
    va_end(ap);

    std::stringstream ss;
    ss << "(";
    std::unordered_set<std::string>::const_iterator it = list.begin();
    if (it != list.end()) {
        ss << *it;
        ++it;
    }
    for (; it != list.end(); ++it) {
        ss << "," << *it;
    }
    ss << ")";
    std::fprintf(fp_, "%s\n", ss.str().c_str());
}

// ログを出力する(x,y,z)
void Logger::WritePointString(LogLevel log_level, const char *file_name,
                              const char *func_name, int line, double x,
                              double y, double z, const char *format, ...) {
    if (!Logger::IsOutput(log_level)) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    WriteHeader(log_level, file_name, func_name, line);

    va_list ap;
    va_start(ap, format);
    std::vfprintf(fp_, format, ap);
    va_end(ap);

    std::stringstream ss;
    ss << "(";
    ss << std::fixed << std::setprecision(10) << x << ",";
    ss << std::fixed << std::setprecision(10) << y << ",";
    ss << std::fixed << std::setprecision(10) << z << ")";
    std::fprintf(fp_, "%s\n", ss.str().c_str());
}

// ログレベルに応じた文字列を取得する
std::string Logger::ToString(LogLevel log_level) {
    switch (log_level) {
        case LogLevel::kDebug:
            return "D";
        case LogLevel::kError:
            return "E";
        case LogLevel::kInfo:
            return "I";
        case LogLevel::kWarn:
            return "W";
        case LogLevel::kVerbose:
            return "V";
        default:
            return " ";
    }
}

void Logger::WriteHeader(LogLevel log_level, const char *file_name,
                         const char *func_name, int line) {
    auto p = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(p);

    time_t current_time = time(nullptr);
    struct tm current_tm;
#ifdef _MSC_VER
    localtime_s(&current_tm, &current_time);
#else
    localtime_r(&current_time, &current_tm);
#endif

    std::stringstream ss;
    ss << std::put_time(&current_tm, "%Y/%m/%d %H:%M:%S") << " ["
       << Logger::ToString(log_level) << "][" << file_name << "][" << func_name
       << "][" << line << "] ";
    std::fprintf(fp_, "%s", ss.str().c_str());
}

}  // namespace spatialid
