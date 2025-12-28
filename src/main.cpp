#ifndef PIO_UNIT_TESTING

#include "Arduino.h"
#include "Log.h"
#include "comms/EthClient.h"
#include "comms/UARTClient.h"
#include "graphics/DeviceScreen.h"

#if defined(ARCH_PORTDUINO)
#include <thread>
#define FSBegin() true
#else
#include "LittleFS.h"
#define FSCom LittleFS
#define FSBegin() LittleFS.begin(true)
#endif

#if defined(I2C_SDA) || defined(I2C_SDA1)
#include "Wire.h"
#endif

// this is pulled in by the device-ui library
const char *firmware_version = "2.7.17";

#ifdef USE_DUMMY_SERIAL
class DummyClient : public IClientBase
{
  public:
    DummyClient() = default;
    void init(void) override {}
    bool connect(void) override { return true; }
    bool disconnect(void) override { return true; }
    bool isConnected(void) override { return false; }
    bool isStandalone(void) override { return true; }
    bool send(meshtastic_ToRadio &&to) override { return false; }
    meshtastic_FromRadio receive(void) override
    {
        meshtastic_FromRadio dummy{};
        return dummy;
    }
    void setNotifyCallback(NotifyCallback notifyConnectionStatus) override {}
    ~DummyClient(){};
};
#endif

IClientBase *client = nullptr;

DeviceScreen *screen = nullptr;

void setup()
{
#if defined(__APPLE__)
    pthread_setname_np("setup");
#elif defined(__linux__)
    pthread_setname_np(pthread_self(), "setup");
#endif
#ifdef KB_POWERON
    digitalWrite(KB_POWERON, HIGH);
    pinMode(KB_POWERON, OUTPUT);
    delay(200); // wait until keyboard mcu startup finished
#endif

#ifdef PWR_ON_PIN
    pinMode(PWR_ON_PIN, OUTPUT);
    digitalWrite(PWR_ON_PIN, HIGH);
#endif

#ifdef PWR_EN_PIN
    pinMode(PWR_EN_PIN, OUTPUT);
    digitalWrite(PWR_EN_PIN, HIGH);
#endif

#ifndef USE_SERIAL0
#ifdef WAIT_FOR_SERIAL0
    delay(2000);
#endif
    Serial.begin(115200);
#ifdef WAIT_FOR_SERIAL0
    time_t timeout = millis();
    while (!Serial && (millis() - timeout) < 2000)
        ;
#endif
    logger.setDebugLevel(ESP_LOG_DEBUG); // use ESP_LOG_VERBOSE for trace category
#else
    logger.setDebugLevel(ESP_LOG_NONE); // do not log when connected over serial0
#endif

    ILOG_INFO("\n//\\ E S H T /\\ S T / C   U I  -  %s\n", firmware_version);
#ifdef I2C_SDA
    if (!Wire.begin(I2C_SDA, I2C_SCL, 400000))
        ILOG_ERROR("*** Failed to access I2C0(%d, %d)", I2C_SDA, I2C_SCL);
#else
    ILOG_DEBUG("I2C-0 not configured");
#endif
#ifdef I2C_SDA1
    if (!Wire.begin(I2C_SDA1, I2C_SCL1, 400000))
        ILOG_ERROR("*** Failed to access I2C1(%d, %d)", I2C_SDA1, I2C_SCL1);
#else
    ILOG_DEBUG("I2C-1 not configured");
#endif

#ifdef ARDUINO_ARCH_ESP32
    uint64_t chipid;
    chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
    ILOG_DEBUG("  ESP32 Chip ID = %04X %08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
    ILOG_DEBUG("  Flash size: %8d bytes", ESP.getFlashChipSize());
    ILOG_DEBUG("  Heap size : %8d bytes", ESP.getHeapSize());
    ILOG_DEBUG("  Free heap : %8d bytes", ESP.getFreeHeap());
    ILOG_DEBUG("  PSRAM     : %8d bytes", ESP.getFreePsram());
    ILOG_DEBUG("  PSRAM max : %8d bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    ILOG_DEBUG("*****************************************");
#endif

    if (!FSBegin()) {
        ILOG_ERROR("LittleFS mount failed!");
    }

#ifdef ARCH_PORTDUINO
    const char *hostname = getenv("MUI_SERVER");
    if (hostname == nullptr) {
        client = new EthClient();
    } else {
        client = new EthClient(hostname);
    }
    int16_t x = 480;
    int16_t y = 480;
    const char *size = getenv("MUI_SIZE");
    if (size != nullptr) {
        sscanf(size, "%" PRId16 "x%" PRId16, &x, &y);
    }
    if (x < 320 || x > 800)
        x = 480;
    if (y < 240 || y > 800)
        y = 480;
    screen = &DeviceScreen::create(DisplayDriverConfig(DisplayDriverConfig::device_t::X11, x, y));
#else
#ifdef USE_DUMMY_SERIAL
    client = new DummyClient();
#else
    client = new UARTClient();
#endif
    screen = &DeviceScreen::create();
#endif

    screen->init(client);

#ifdef ARDUINO_ARCH_ESP32
    ILOG_DEBUG("Free heap : %8d bytes", ESP.getFreeHeap());
    ILOG_DEBUG("PSRAM     : %8d bytes", ESP.getFreePsram());
#endif

#ifdef ARCH_PORTDUINO
    // create separate thread to handle lvgl X11 GUI simulation
    // otherwise the GUI will slow down the main thread
    extern void tft_task_handler(void *param = nullptr);
    new std::thread([] {
#ifdef __APPLE__
        pthread_setname_np("tft");
#else
        pthread_setname_np(pthread_self(), "tft");
#endif
        tft_task_handler();
    });
#endif

    ILOG_DEBUG("Setup done.");
#if defined(__APPLE__)
    pthread_setname_np("loop");
#elif defined(__linux__)
    pthread_setname_np(pthread_self(), "loop");
#endif
}

#if defined(ARCH_ESP32)
void loop()
{
    screen->task_handler();
    screen->sleep(5);
}

#elif defined(ARCH_PORTDUINO)
void loop()
{
    delay(1000);
    fflush(nullptr);
}

void tft_task_handler(void *)
{
    ILOG_INFO("tft_task_handler started");
    while (true) {
        screen->task_handler();
        screen->sleep(5);
    }
}
#else
#error "Unsupported architecture"
#endif

#endif