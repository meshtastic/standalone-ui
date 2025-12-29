#include "Log.h"
#include "stdarg.h"
#include <ctime>
#include <stdio.h>

#define CL_LIGHTRED "\033[01;31m" //!< Light red.
#define CL_RED "\033[22;31m"      //!< Red.
#define CL_YELLOW "\033[01;33m"   //!< Yellow.
#define CL_WHITE "\033[01;37m"    //!< White.
#define CL_DARKGRAY "\033[01;97m" //!< Dark gray.
#define NORMAL "\x1B[0m"          //!< Normal.

#if defined(ARCH_ESP32)
// simply forward all logs into the esp log forwarder
#define VA_START(LVL, ARG, FMT)                                                                                                  \
    esp_log_write(LVL, "TFT", "%s | %s %d [%s] ", lvl[LVL], getTime(), int(millis() / 1000), "TFT");                             \
    va_start(ARG, FMT)
#define LOG_FWD(LVL, TAG, FMT, ARG) esp_log_writev(LVL, TAG, FMT, ARG)
#define LOG_CRLF(LVL, TAG) esp_log_write(LVL, TAG, "\n")
#elif defined(ARCH_PORTDUINO)
// #define _GNU_SOURCE
#include <pthread.h>
#define VA_START(LVL, ARG, FMT)                                                                                                  \
    if (LVL > loglevel) return;                                                                                                  \
    pthread_getname_np(pthread_self(), threadname, sizeof(threadname));                                                          \
    printf("%s | %s %d [%s] ", lvl[LVL], getTime(), int(millis() / 1000), threadname);                                           \
    va_start(ARG, FMT)
#define LOG_FWD(LVL, TAG, FMT, ARG) vprintf(FMT, ARG)
#define LOG_CRLF(LVL, TAG) printf(NORMAL "\n")
#define esp_log_level_set(S,LVL) loglevel = LVL;
static esp_log_level_t loglevel;
#endif

extern "C" unsigned long millis(void);

static time_t rawtime;
struct tm *timeinfo;
char buffer[20];
char threadname[16];
const char *lvl[] = {"NONE", CL_RED "ERROR", CL_YELLOW "WARN ", CL_WHITE "INFO ", "DEBUG", CL_DARKGRAY "TRACE"};
Log logger;

char *getTime()
{
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M:%S ", timeinfo);
    return buffer;
}

void Log::setDebugLevel(esp_log_level_t lvl)
{
    esp_log_level_set("*", lvl);
}

void Log::log_debug(const char *format, ...)
{
    va_list arg;
    VA_START(ESP_LOG_DEBUG, arg, format);
    LOG_FWD(ESP_LOG_DEBUG, "TFT", format, arg);
    va_end(arg);
    LOG_CRLF(ESP_LOG_DEBUG, "TFT");
}

void Log::log_info(const char *format, ...)
{
    va_list arg;
    VA_START(ESP_LOG_INFO, arg, format);
    LOG_FWD(ESP_LOG_INFO, "TFT", format, arg);
    va_end(arg);
    LOG_CRLF(ESP_LOG_INFO, "TFT");
}

void Log::log_warn(const char *format, ...)
{
    va_list arg;
    VA_START(ESP_LOG_WARN, arg, format);
    LOG_FWD(ESP_LOG_WARN, "TFT", format, arg);
    va_end(arg);
    LOG_CRLF(ESP_LOG_WARN, "TFT");
}

void Log::log_error(const char *format, ...)
{
    va_list arg;
    VA_START(ESP_LOG_ERROR, arg, format);
    LOG_FWD(ESP_LOG_ERROR, "TFT", format, arg);
    va_end(arg);
    LOG_CRLF(ESP_LOG_ERROR, "TFT");
}

void Log::log_crit(const char *format, ...)
{
    va_list arg;
    VA_START(ESP_LOG_ERROR, arg, format);
    LOG_FWD(ESP_LOG_ERROR, "TFT", format, arg);
    va_end(arg);
    LOG_CRLF(ESP_LOG_ERROR, "TFT");
}

void Log::log_trace(const char *format, ...)
{
    va_list arg;
    VA_START(ESP_LOG_VERBOSE, arg, format);
    LOG_FWD(ESP_LOG_VERBOSE, "TFT", format, arg);
    va_end(arg);
    LOG_CRLF(ESP_LOG_VERBOSE, "TFT");
}
