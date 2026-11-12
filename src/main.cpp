#ifndef PIO_UNIT_TESTING

#include "Arduino.h"
#include "Log.h"
#include "comms/EthClient.h"
#include "comms/LinuxSerialClient.h"
#include "comms/SerialClient.h"
#include "comms/UARTClient.h"
#include "graphics/DeviceScreen.h"

#if defined(ARCH_PORTDUINO)
#include "NativeOptions.h"
#include <thread>
#include <utility>
#define FSBegin() true
#else
#include "LittleFS.h"
#define FSCom LittleFS
#define FSBegin() LittleFS.begin(true)
#endif

#if defined(I2C_SDA) || defined(I2C_SDA1)
#include "Wire.h"
#endif

// device-ui's SpiBus reads the SD card through this HSPI instance when the board
// wires the card to SPI1 (SDCARD_USE_SPI1). Defined here, mirroring firmware FSCommon.cpp.
#if defined(ARCH_ESP32) && defined(HAS_SDCARD) && defined(SDCARD_USE_SPI1)
#include <SPI.h>
SPIClass SPI_HSPI(HSPI);
#endif

// this is pulled in by the device-ui library
const char *firmware_version = "2.8.3";

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
    ~DummyClient() {};
};
#endif

IClientBase *client = nullptr;
DeviceScreen *screen = nullptr;
esp_log_level_t logLevel = esp_log_level_t::ESP_LOG_DEBUG;

#ifdef ARCH_PORTDUINO
std::string ttydev;
std::string remoteHost;
native_ui::OptionValues displayOptions;

enum { OPT_KEYBOARD = 1000, OPT_POINTER, OPT_KEYBOARD_NAVIGATION, OPT_CURSOR, OPT_SIDEBAR_SCROLLBAR };

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    switch (key) {
    case 'p':
        ttydev = arg;
        break;
    case 'h':
        remoteHost = arg;
        break;
    case 's':
        displayOptions.size = arg;
        break;
    case OPT_KEYBOARD:
        displayOptions.keyboard = arg;
        break;
    case OPT_POINTER:
        displayOptions.pointer = arg;
        break;
    case OPT_KEYBOARD_NAVIGATION:
        displayOptions.keyboardNavigation = arg;
        break;
    case OPT_CURSOR:
        displayOptions.cursor = arg;
        break;
    case OPT_SIDEBAR_SCROLLBAR:
        displayOptions.sidebarScrollbar = arg;
        break;
    case 'v':
        logLevel = esp_log_level_t::ESP_LOG_VERBOSE;
        break;
    case ARGP_KEY_ARG:
        return 0;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

void portduinoCustomInit(void)
{
    static struct argp_option options[] = {
        {"port", 'p', "PORT", 0, "The tty device name to connect to."},
        {"host", 'h', "HOSTNAME", 0, "The remote host or IP to connect to."},
        {"size", 's', "WIDTHxHEIGHT", 0, "The display size (default 480x480)"},
        {"keyboard", OPT_KEYBOARD, "DEVICE", 0, "The Linux keyboard input device"},
        {"pointer", OPT_POINTER, "DEVICE", 0, "The Linux pointer input device"},
        {"keyboard-navigation", OPT_KEYBOARD_NAVIGATION, "MODE", 0,
         "Configured Linux keyboard navigation: contextual or raw (default contextual)"},
        {"cursor", OPT_CURSOR, "MODE", 0, "Framebuffer/configured pointer cursor: auto, always, or never (default auto)"},
        {"sidebar-scrollbar", OPT_SIDEBAR_SCROLLBAR, "MODE", 0, "Sidebar scrollbar: auto, on, or off (default auto)"},
        {"verbose", 'v', 0, 0, "Set log level to full trace"},
        {0}};
    static void *childArguments;
    static char doc[] = "Standalone MUI native build.";
    static char args_doc[] = "...";
    static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};
    const struct argp_child child = {&argp, OPTION_ARG_OPTIONAL, 0, 0};
    portduinoAddArguments(child, childArguments);
}

void portduinoSetup(void)
{
    const char *tty = getenv("MUI_TTY");
    const char *hostname = getenv("MUI_SERVER");

    if (!ttydev.empty())
        client = new LinuxSerialClient(ttydev.c_str());
    else if (tty != nullptr)
        client = new LinuxSerialClient(tty);
    else if (!remoteHost.empty())
        client = new EthClient(remoteHost.c_str());
    else if (hostname != nullptr)
        client = new EthClient(hostname);
    else
        client = new EthClient();

#ifdef USE_FRAMEBUFFER
    constexpr bool framebuffer = true;
    constexpr auto device = DisplayDriverConfig::device_t::FB;
#else
    constexpr bool framebuffer = false;
    constexpr auto device = DisplayDriverConfig::device_t::X11;
#endif

    native_ui::OptionValues environment;
    const auto readEnvironment = [](const char *name) -> std::optional<std::string> {
        const char *value = getenv(name);
        return value && *value ? std::optional<std::string>(value) : std::nullopt;
    };
    environment.size = readEnvironment("MUI_SIZE");
    environment.keyboard = readEnvironment("MUI_KEYBOARD");
    environment.pointer = readEnvironment("MUI_POINTER");
    environment.keyboardNavigation = readEnvironment("MUI_KEYBOARD_NAVIGATION");
    environment.cursor = readEnvironment("MUI_CURSOR");
    environment.sidebarScrollbar = readEnvironment("MUI_SIDEBAR_SCROLLBAR");
    native_ui::DisplayOptions options;
    std::string error;
    if (!native_ui::resolveDisplayOptions(displayOptions, environment, framebuffer, options, error)) {
        fprintf(stderr, "Invalid native display option: %s\n", error.c_str());
        exit(EXIT_FAILURE);
    }

    DisplayDriverConfig config(device, options.width, options.height);
    config.input(std::move(options.input));
    config.navigation(std::move(options.navigation));
    screen = &DeviceScreen::create(std::move(config));
}
#endif

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
    logger.setDebugLevel(logLevel);
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

#ifdef ARCH_ESP32
#ifdef USE_DUMMY_SERIAL
    client = new DummyClient();
#else
    client = new UARTClient();
#endif
    screen = &DeviceScreen::create();
#endif

    screen->init(client);

#if defined(ARCH_ESP32) && defined(HAS_SDCARD) && defined(SDCARD_USE_SPI1) && defined(SPI_SCK) && defined(SPI_MISO) &&           \
    defined(SPI_MOSI)
    // The WT32-SC01 Plus declares SPI_SCK/MISO/MOSI as static const constants rather than
    // macros, so device-ui's macro-guarded SDHandler.begin() never runs and SD.begin() falls
    // back to unattached ESP32-S3 HSPI defaults. Attach the card pins here instead.
    SPI_HSPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
#endif

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
    }
}
#else
#error "Unsupported architecture"
#endif

#endif
