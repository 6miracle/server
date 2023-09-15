#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>
#include <ctime>
#include <string>

#define CURRENT_THREAD() \
    ({   \
    std::stringstream ss; \
    ss << std::this_thread::get_id(); \
    ss.str(); \
    }) 


// 文件设置
#define MAX_COUNT 10000
static FILE *fp;
static int count = 0;

#define BLACK  "\e[0;30m"
#define RED    "\e[0;31m"
#define GREEN  "\e[0;32m"
#define YELLOW "\e[0;33m"
#define BLUE   "\e[0;34m"
#define WHITE  "\e[0;37m"
#define BLANK  "\e[0m"
// Log levels.
#define LOG_LEVEL_OFF       1000
#define LOG_LEVEL_ERROR     500
#define LOG_LEVEL_WARN      400
#define LOG_LEVEL_INFO      300
#define LOG_LEVEL_DEBUG     200
#define LOG_LEVEL_TRACE     100
#define LOG_LEVEL_ALL       0
#define LOG_LOG_TIME_FORMAT "%Y-%m-%d %H:%M:%S"

// 设置默认level
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

// 简单模式
#define LOG(...)                                    \
    fprintf(stdout, "[%s:%d]", __FILE__, __LINE__); \
    ::fprintf(stdout, __VA_ARGS__);                 \
    ::fprintf(stdout, "\n")

// 复杂模式
// ERROR
#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(...)                                              \
    OutputLogHeader(                                                \
        __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, stdout); \
    ::fprintf(stdout, __VA_ARGS__);                                 \
    ::fprintf(stdout, "\n");                                        \
    ::fflush(stdout)
// 文件
#define FOG_ERROR(...)                                                      \
    OutputFile(LOG_LEVEL_ERROR);                                            \
    OutputLogHeader(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_ERROR, fp); \
    ::fprintf(fp, __VA_ARGS__);                                             \
    ::fprintf(fp, "\n");                                                    \
    ::fflush(fp)
#else
#define LOG_ERROR(...) ((void)0)
#define FOG_ERROR(...) ((void)0)
#endif

// WARN
#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(...)                                                          \
    OutputLogHeader(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_WARN, stdout); \
    ::fprintf(stdout, __VA_ARGS__);                                            \
    ::fprintf(stdout, "\n");                                                   \
    ::fflush(stdout)
#define FOG_WARN(...)                                                      \
    OutputFile(LOG_LEVEL_WARN);                                            \
    OutputLogHeader(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_WARN, fp); \
    ::fprintf(fp, __VA_ARGS__);                                            \
    ::fprintf(fp, "\n");                                                   \
    ::fflush(fp)
#else
#define LOG_WARN(...) ((void)0)
#define FOG_WARN(...) ((void)0)
#endif

// INFO
#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(...)                                                          \
    OutputLogHeader(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_INFO, stdout); \
    ::fprintf(stdout, __VA_ARGS__);                                            \
    ::fprintf(stdout, "\n");                                                   \
    ::fflush(stdout)
#define FOG_INFO(...)                                                      \
    OutputFile(LOG_LEVEL_INFO);                                            \
    OutputLogHeader(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_INFO, fp); \
    ::fprintf(fp, __VA_ARGS__);                                            \
    ::fprintf(fp, "\n");                                                   \
    ::fflush(fp)
#else
#define LOG_INFO(...) ((void)0)
#define FOG_INFO(...) ((void)0)
#endif

// DEBUG
#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_DEBUG(...)                                              \
    OutputLogHeader(                                                \
        __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, stdout); \
    ::fprintf(stdout, __VA_ARGS__);                                 \
    ::fprintf(stdout, "\n");                                        \
    ::fflush(stdout)
#define FOG_DEBUG(...)                                                      \
    OutputFile(LOG_LEVEL_DEBUG);                                            \
    OutputLogHeader(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_DEBUG, fp); \
    ::fprintf(fp, __VA_ARGS__);                                             \
    ::fprintf(fp, "\n");                                                    \
    ::fflush(fp)
#else
#define LOG_DEBUG(...) ((void)0)
#define FOG_DEBUG(...) ((void)0)
#endif

// TRACE
#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define LOG_TRACE(...)                                              \
    OutputLogHeader(                                                \
        __FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_TRACE, stdout); \
    ::fprintf(stdout, __VA_ARGS__);                                 \
    ::fprintf(stdout, "\n");                                        \
    ::fflush(stdout)
#define FOG_TRACE(...)                                                      \
    OutputFile(LOG_LEVEL_ERROR);                                            \
    OutputLogHeader(__FILE__, __FUNCTION__, __LINE__, LOG_LEVEL_TRACE, fp); \
    ::fprintf(fp, __VA_ARGS__);                                             \
    ::fprintf(fp, "\n");                                                    \
    ::fflush(fp)
#else
#define LOG_TRACE(...) ((void)0)
#define FOG_TRACE(...) ((void)0)
#endif


inline void OutputLogHeader(
    const char *file, const char *func, int line, int level, FILE *out, ...) {
    time_t t = ::time(nullptr);
    tm *curTime = localtime(&t);
    char time_str[32];
    ::strftime(time_str, 32, LOG_LOG_TIME_FORMAT, curTime);
    const char *type;
    const char *color;
    const char *end = BLANK;
    switch (level) {
        case LOG_LEVEL_ERROR:
            type = "ERROR";
            color = RED;
            break;
        case LOG_LEVEL_WARN:
            type = "WARN";
            color = YELLOW;
            break;
        case LOG_LEVEL_INFO:
            type = "INFO";
            color = BLUE;
            break;
        case LOG_LEVEL_DEBUG:
            type = "DEBUG";
            color = GREEN;
            break;
        default:
            type = "UNKWN";
    }
    if (out != stdout) {
        color = "";
        end = "";
    }
    ::fprintf(
        out, "%s[%-5s][%s %s:%d:%s] %s", color, type, time_str, file, line,
        func, end);
}

inline void OutputFile(int level) {
    // 设置count和文件指针
    if (count >= MAX_COUNT) {
        time_t t = ::time(nullptr);
        tm *curTime = localtime(&t);
        char time_str[32];
        ::strftime(time_str, 32, LOG_LOG_TIME_FORMAT, curTime);
        rename("current", time_str); // 可能新名字已存在
        fp = nullptr;
        count = 0;
    }
    if (fp == nullptr) { fp = fopen("current", "a"); }
    ++count;
}
#endif
