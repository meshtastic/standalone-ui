#include "util/ILog.h"

#if defined(ARCH_ESP32)
#include "esp_log.h"
#elif defined(ARCH_PORTDUINO)
typedef enum { ESP_LOG_NONE, ESP_LOG_ERROR, ESP_LOG_WARN, ESP_LOG_INFO, ESP_LOG_DEBUG, ESP_LOG_VERBOSE } esp_log_level_t;
#endif

class Log : public ILog
{
  public:
    Log() : ILog(this) { setDebugLevel(ESP_LOG_DEBUG); }
    void setDebugLevel(esp_log_level_t lvl);

  protected:
    virtual void log_debug(const char *format, ...) override;
    virtual void log_info(const char *format, ...) override;
    virtual void log_warn(const char *format, ...) override;
    virtual void log_error(const char *format, ...) override;
    virtual void log_crit(const char *format, ...) override;
    virtual void log_trace(const char *format, ...) override;
};

extern Log logger;
