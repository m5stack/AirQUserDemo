#include <Arduino.h>

#include <Wire.h>
#include <LittleFS.h>
#include <M5Unified.h>
#include <lgfx/v1/panel/Panel_GDEW0154M09.hpp>
#include <WiFi.h>
#include <sntp.h>
#include <freertos/queue.h>

#include "I2C_BM8563.h"
#include <SensirionI2CScd4x.h>
#include <SensirionI2CSen5x.h>
#include <OneButton.h>
#include <cJSON.h>
#include <Preferences.h>

#include <EzData.hpp>
#include "config.h"
#include "misc.h"
#include "DataBase.hpp"
#include "MainAppView.hpp"
#include "Sensor.hpp"
#include "AppWeb.hpp"


class AirQ_GFX : public lgfx::LGFX_Device {
    lgfx::Panel_GDEW0154M09 _panel_instance;
    lgfx::Bus_SPI           _spi_bus_instance;

   public:
    AirQ_GFX(void) {
        {
            auto cfg = _spi_bus_instance.config();

            cfg.pin_mosi   = EPD_MOSI;
            cfg.pin_miso   = EPD_MISO;
            cfg.pin_sclk   = EPD_SCLK;
            cfg.pin_dc     = EPD_DC;
            cfg.freq_write = EPD_FREQ;

            _spi_bus_instance.config(cfg);
            _panel_instance.setBus(&_spi_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();

            cfg.invert       = false;
            cfg.pin_cs       = EPD_CS;
            cfg.pin_rst      = EPD_RST;
            cfg.pin_busy     = EPD_BUSY;
            cfg.panel_width  = 200;
            cfg.panel_height = 200;
            cfg.offset_x     = 0;
            cfg.offset_y     = 0;

            _panel_instance.config(cfg);
        }
        setPanel(&_panel_instance);
    }
    bool begin(void) { return init_impl(true , false); };
};


typedef enum WakeupType_t {
    E_WAKEUP_TYPE_UNKNOWN = 0,
    E_WAKEUP_TYPE_RTC,
    E_WAKEUP_TYPE_USER,
    E_WAKEUP_TYPE_USB,
} WakeupType_t;

typedef enum RunMode_t {
    E_RUN_MODE_FACTORY = 0,
    E_RUN_MODE_MAIN,
    E_RUN_MODE_SETTING,
    E_RUN_MODE_EZDATA,
} RunMode_t;


typedef enum SettingState_t {
    E_SETTING_STATE_INIT = 0,
    E_SETTING_STATE_AP,
    E_SETTING_STATE_WEB,
    E_SETTING_STATE_DONE,
} SettingState_t;


typedef enum ButtonID_t {
    E_BUTTON_NONE,
    E_BUTTON_A,
    E_BUTTON_B,
    E_BUTTON_POWER,
} ButtonID_t;


typedef struct ButtonEvent_t {
    ButtonID_t id;
} ButtonEvent_t;

typedef enum EzDataState_t {
    E_EZDATA_STATE_INIT = 0,
    E_EZDATA_STATE_SUCCESS,
    E_EZDATA_STATE_FAILURE,
} EzDataState_t;


WakeupType_t wakeupType = E_WAKEUP_TYPE_UNKNOWN;

RunMode_t runMode = E_RUN_MODE_MAIN;

AirQ_GFX lcd;
M5Canvas mainCanvas(&lcd);
StatusView statusView(&lcd, &mainCanvas);

SensirionI2CScd4x scd4x;
SensirionI2CSen5x sen5x;
I2C_BM8563 bm8563(I2C_BM8563_DEFAULT_ADDRESS, Wire);
Sensor sensor(scd4x, sen5x, bm8563);

EzData ezdataHanlder(db.ezdata2.devToken, "raw");
bool ezdataStatus = false;
EzDataState_t ezdataState = E_EZDATA_STATE_INIT;

Preferences preferences;
uint32_t successCounter = 0;
uint32_t failCounter = 0;

TaskHandle_t countdownTaskHandler;

OneButton btnA = OneButton(
    USER_BTN_A,  // Input pin for the button
    true,        // Button is active LOW
    true         // Enable internal pull-up resistor
);

OneButton btnB = OneButton(
    USER_BTN_B,  // Input pin for the button
    true,        // Button is active LOW
    true         // Enable internal pull-up resistor
);

OneButton btnPower = OneButton(
    USER_BUTTON_POWER,  // Input pin for the button
    true,        // Button is active LOW
    false         // Enable internal pull-up resistor
);

QueueHandle_t buttonEventQueue;

String mac;
String apSSID;

void listDirectory(fs::FS &fs, const char * dirname, uint8_t levels);


void setup() {
    Serial.begin(115200);

    log_i("Project name: AirQ demo");
    log_i("Build: %s %s", __DATE__, __TIME__);
    log_i("Version: %s", APP_VERSION);

    log_i("Turn on main power");
    pinMode(POWER_HOLD, OUTPUT);
    digitalWrite(POWER_HOLD, HIGH);

    log_i("Turn on SEN55 power");
    pinMode(SEN55_POWER_EN, OUTPUT);
    digitalWrite(SEN55_POWER_EN, LOW);

    log_i("LittleFS init");
    if (FORMAT_FILESYSTEM) {
        FILESYSTEM.format();
    }
    FILESYSTEM.begin();
    listDirectory(FILESYSTEM, "/", 1);
    db.loadFromFile();
    db.dump();

    /** Start Beep */
    if (db.buzzer.onoff == true) {
        ledcAttachPin(BUZZER_PIN, 0);
    } else {
        ledcDetachPin(BUZZER_PIN);
    }
    BUTTON_TONE();

    log_i("NVS init");
    preferences.begin("airq", false);
    successCounter = preferences.getUInt("OK", 0);
    failCounter = preferences.getUInt("NG", 0);

    log_i("Screen init");
    lcd.begin();
    lcd.setEpdMode(epd_mode_t::epd_text);
    // lcd.sleep();

    statusView.begin();

    wifiAPSTASetup();

    log_i("I2C init");
    pinMode(GROVE_SDA, OUTPUT);
    pinMode(GROVE_SCL, OUTPUT);
    Wire.begin(I2C1_SDA_PIN, I2C1_SCL_PIN);

    log_i("RTC(BM8563) init");
    bm8563.begin();
    bm8563.clearIRQ();

    wakeupType = getDeviceWakeupType();

    log_i("NTP init");
    sntp_servermode_dhcp(1);
    configTzTime(
        db.ntp.tz.c_str(),
        db.ntp.ntpServer0.c_str(),
        db.ntp.ntpServer1.c_str(),
        "pool.ntp.org"
    );

    log_i("SCD40 sensor init");
    char errorMessage[256];
    scd4x.begin(Wire);
    /** stop potentially previously started measurement */
    uint16_t error = scd4x.stopPeriodicMeasurement();
    if (error) {
        errorToString(error, errorMessage, 256);
        log_w("Error trying to execute stopPeriodicMeasurement(): %s", errorMessage);
    }
    /** Start Measurement */
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        errorToString(error, errorMessage, 256);
        log_w("Error trying to execute startPeriodicMeasurement(): %s", errorMessage);
    }
    log_i("Waiting for first measurement... (5 sec)");

    /** Init SEN55 */
    log_i("SEN55 sensor init");
    sen5x.begin(Wire);
    error = sen5x.deviceReset();
    if (error) {
        errorToString(error, errorMessage, 256);
        log_w("Error trying to execute deviceReset(): %s", errorMessage);
    }
    float tempOffset = 0.0;
    error = sen5x.setTemperatureOffsetSimple(tempOffset);
    if (error) {
        errorToString(error, errorMessage, 256);
        log_w("Error trying to execute setTemperatureOffsetSimple(): %s", errorMessage);
    } else {
        log_i("Temperature Offset set to %f deg. Celsius (SEN54/SEN55 only)", tempOffset);
    }
    /** Start Measurement */
    error = sen5x.startMeasurement();
    if (error) {
        errorToString(error, errorMessage, 256);
        log_w("Error trying to execute startMeasurement(): %s", errorMessage);
    }

    /** fixme: 超时处理 */
    bool isDataReady = false;
    do {
        error = scd4x.getDataReadyFlag(isDataReady);
        if (error) {
            errorToString(error, errorMessage, 256);
            log_w("Error trying to execute getDataReadyFlag(): %s", errorMessage);
            return;
        }
    } while (!isDataReady);

    if (db.factoryState || wakeupType == E_WAKEUP_TYPE_USER) {
        runMode = E_RUN_MODE_FACTORY;
    }

    btnA.attachClick(btnAClickEvent);
    btnB.attachClick(btnBClickEvent);
    btnPower.attachClick(btnPowerClickEvent);
    buttonEventQueue = xQueueCreate(16, sizeof(ButtonEvent_t));
    xTaskCreatePinnedToCore(buttonTask, "Button Task", 4096, NULL, 5, NULL, APP_CPU_NUM);
    // if (runMode != E_RUN_MODE_FACTORY) {
        // xTaskCreatePinnedToCore(countdownTask, "Shutdown Task", 4096, NULL, 4, &countdownTaskHandler, APP_CPU_NUM);
    // }
}


void loop() {

    ButtonEvent_t buttonEvent = { .id = E_BUTTON_NONE };
    xQueueReceive(buttonEventQueue, &buttonEvent, (TickType_t)10);

    switch (runMode) {
        case E_RUN_MODE_FACTORY: {
            factoryApp(&buttonEvent);
        }
        break;

        case E_RUN_MODE_MAIN: {
            mainApp(&buttonEvent);
        }
        break;

        case E_RUN_MODE_SETTING: {
            settingApp(&buttonEvent);
        }
        break;

        case E_RUN_MODE_EZDATA: {
            ezdataApp(&buttonEvent);
        }
        break;

        default: break;
    }
    ezdataServiceTask();
    countdownServiceTask();
    shutdownServiceTask(&buttonEvent);
    buttonEvent.id = E_BUTTON_NONE;
    delay(10);
}


void factoryApp(ButtonEvent_t *buttonEvent) {
    static bool refesh = true;
    static int64_t lastCountDownUpdate = esp_timer_get_time() / 1000;
    static int64_t lastCountDown = 5;

    int64_t currentMillisecond = esp_timer_get_time() / 1000;

    if (buttonEvent->id == E_BUTTON_B) {
        runMode = E_RUN_MODE_SETTING;
        return ;
    }

    if (refesh) {
        refesh = false;
        lcd.clear(TFT_BLACK);
        lcd.waitDisplay();
        lcd.clear(TFT_WHITE);
        lcd.waitDisplay();
        lcd.drawJpgFile(FILESYSTEM, "/init.jpg", 0, 0);
        lcd.drawString(String(lastCountDown), 86, 173, &fonts::FreeSansBold12pt7b);
        lcd.waitDisplay();
    }

    if (currentMillisecond - lastCountDownUpdate > 1000) {
        lastCountDown--;
        lcd.drawString(String(lastCountDown), 86, 173, &fonts::FreeSansBold12pt7b);
        lcd.waitDisplay();
        if (lastCountDown == 0) {
            lastCountDown = 5;
            runMode = E_RUN_MODE_MAIN;
        }
        lastCountDownUpdate = currentMillisecond;
    }

}


void mainApp(ButtonEvent_t *buttonEvent) {
    static bool refesh = true;
    static int64_t lastMillisecond = esp_timer_get_time() / 1000;
    static int64_t lastCountDownUpdate = lastMillisecond;
    static int64_t lastCountDown = db.rtc.sleepInterval;
    static bool runingEzdataUpload = false;
    static int ezdataUploadCount = EZDATA_UPLOAD_RETRY_COUNT;

    int64_t currentMillisecond = esp_timer_get_time() / 1000;

    if (buttonEvent->id == E_BUTTON_A || buttonEvent->id == E_BUTTON_B) {
        runMode = buttonEvent->id == E_BUTTON_A ? E_RUN_MODE_EZDATA : E_RUN_MODE_SETTING;
        refesh = true;
        return ;
    }

    if (refesh) {
        refesh = false;
        log_d("refesh");
        sensor.getSCD40MeasurementResult();
        sensor.getSEN55MeasurementResult();
        sensor.getBatteryVoltageRaw();
        sensor.getTimeString();
        sensor.getDateString();

        statusView.updateSCD40(
            sensor.scd40.co2,
            sensor.scd40.temperature,
            sensor.scd40.humidity
        );
        statusView.updatePower(sensor.battery.raw);
        statusView.updateCountdown(db.rtc.sleepInterval);
        statusView.updateSEN55(
            sensor.sen55.massConcentrationPm1p0,
            sensor.sen55.massConcentrationPm2p5,
            sensor.sen55.massConcentrationPm4p0,
            sensor.sen55.massConcentrationPm10p0,
            sensor.sen55.ambientHumidity,
            sensor.sen55.ambientTemperature,
            sensor.sen55.vocIndex,
            sensor.sen55.noxIndex
        );
        statusView.updateTime(sensor.time.time, sensor.time.date);

        statusView.load();
        lastMillisecond = currentMillisecond;
        if (lastCountDown == db.rtc.sleepInterval && (WiFi.isConnected() && db.ezdata2.devToken)) {
            ezdataUploadCount = EZDATA_UPLOAD_RETRY_COUNT;
            runingEzdataUpload = true;
        }
    }

    if (currentMillisecond - lastCountDownUpdate > 1000) {
        lastCountDown--;
        statusView.displayCountdown(lastCountDown);
        if (lastCountDown == 0) {
            lastCountDown = db.rtc.sleepInterval;
            refesh = true;
        }
        lastCountDownUpdate = currentMillisecond;
    }

    if (WiFi.isConnected() && runingEzdataUpload && ezdataUploadCount-- > 0) {
        ezdataHanlder.setDeviceToken(db.ezdata2.devToken);
        BUTTON_TONE();
        if (uploadSensorRawData(ezdataHanlder)) {
            successCounter += 1;
            preferences.putUInt("OK", successCounter);
            String msg = "OK:" + String(successCounter);
            statusView.displayNetworkStatus("Upload", msg.c_str());
            SUCCESS_TONE();
            runingEzdataUpload = false;
            ezdataState = E_EZDATA_STATE_SUCCESS;
            ezdataStatus = true;
        } else {
            failCounter += 1;
            preferences.putUInt("NG", failCounter);
            String msg = "NG:" + String(failCounter);
            statusView.displayNetworkStatus("Upload", msg.c_str());
            FAIL_TONE();
        }
    }

}


void ezdataApp(ButtonEvent_t *buttonEvent) {
    static bool refresh = true;
    static String devToken = db.ezdata2.devToken;

    String url = "https://airq.m5stack.com/" + db.ezdata2.devToken + "?data=raw";

    if (buttonEvent->id == E_BUTTON_A) {
        runMode = E_RUN_MODE_MAIN;
        refresh = true;
        return ;
    }

    if (refresh || devToken != db.ezdata2.devToken) {
        lcd.clear(TFT_BLACK);
        lcd.waitDisplay();
        lcd.clear(TFT_WHITE);
        lcd.waitDisplay();
        lcd.drawJpgFile(FILESYSTEM, "/ezdata.jpg", 0, 0);
        lcd.waitDisplay();
        lcd.qrcode(url, 35, 35, 130);
        lcd.waitDisplay();
        refresh = false;
    }
}


void settingApp(ButtonEvent_t *buttonEvent) {
    static bool refresh = true;
    static SettingState_t settingState = E_SETTING_STATE_INIT;
    static int64_t lastMillisecond = esp_timer_get_time() / 1000;

    String apQrcode = "WIFI:T:nopass;S:" + apSSID + ";P:;H:false;;";

    if (buttonEvent->id == E_BUTTON_A) {
        if (settingState == E_SETTING_STATE_AP || settingState == E_SETTING_STATE_WEB) {
            WiFi.softAPdisconnect();
            appWebServerClose();
            if (WiFi.isConnected() != true) {
                WiFi.disconnect();
                delay(200);
                WiFi.begin(db.wifi.ssid.c_str(), db.wifi.password.c_str());
            }
        }
        runMode = E_RUN_MODE_MAIN;
        settingState = E_SETTING_STATE_INIT;
        refresh = true;
        return;
    }

    if (settingState == E_SETTING_STATE_INIT && buttonEvent->id == E_BUTTON_B) {
        wifiStartAP();
        appWebServer();
        settingState = E_SETTING_STATE_AP;
        ezdataStatus = false;
        refresh = true;
    }

    if (settingState == E_SETTING_STATE_AP && WiFi.softAPgetStationNum() > 0) {
        settingState = E_SETTING_STATE_WEB;
        refresh = true;
    }

    if (settingState == E_SETTING_STATE_WEB && WiFi.softAPgetStationNum() == 0) {
        settingState = E_SETTING_STATE_AP;
        refresh = true;
    }

    if (settingState == E_SETTING_STATE_WEB && ezdataStatus == true) {
        settingState = E_SETTING_STATE_DONE;
        lastMillisecond = esp_timer_get_time() / 1000;
        refresh = true;
    }

    if (settingState == E_SETTING_STATE_DONE && esp_timer_get_time() / 1000 - lastMillisecond > 1000) {
        runMode = E_RUN_MODE_MAIN;
        settingState = E_SETTING_STATE_INIT;
        refresh = true;
        WiFi.softAPdisconnect();
        appWebServerClose();
        return ;
    }

    if (refresh) {
        switch (settingState)
        {
            case E_SETTING_STATE_INIT:
                lcd.clear(TFT_BLACK);
                lcd.waitDisplay();
                lcd.clear(TFT_WHITE);
                lcd.waitDisplay();
                lcd.drawJpgFile(FILESYSTEM, "/settings.jpg", 0, 0);
                lcd.waitDisplay();
            break;

            case E_SETTING_STATE_AP:
                apQrcode = "WIFI:T:nopass;S:" + apSSID + ";P:;H:false;;";
                lcd.clear(TFT_BLACK);
                lcd.waitDisplay();
                lcd.clear(TFT_WHITE);
                lcd.waitDisplay();
                lcd.drawJpgFile(FILESYSTEM, "/ap.jpg", 0, 0);
                lcd.qrcode(apQrcode, 35, 35, 130);
                lcd.drawString(apSSID, 70, 175, &fonts::FreeSansBold9pt7b);
                lcd.waitDisplay();
            break;

            case E_SETTING_STATE_WEB:
                lcd.clear(TFT_BLACK);
                lcd.waitDisplay();
                lcd.clear(TFT_WHITE);
                lcd.waitDisplay();
                lcd.drawJpgFile(FILESYSTEM, "/web.jpg", 0, 0);
                lcd.qrcode("http://192.168.4.1", 35, 35, 130);
                lcd.waitDisplay();
            break;

            case E_SETTING_STATE_DONE:
                lcd.clear(TFT_BLACK);
                lcd.waitDisplay();
                lcd.clear(TFT_WHITE);
                lcd.waitDisplay();
                lcd.drawJpgFile(FILESYSTEM, "/done.jpg", 0, 0);
                lcd.waitDisplay();

                lastMillisecond = esp_timer_get_time() / 1000;
            break;

            default:
            break;
        }
        refresh = false;
    }

}


void ezdataServiceTask() {
    static int64_t lastMillisecond = esp_timer_get_time() / 1000;

    if (
        WiFi.isConnected() == false 
        || ezdataStatus == true 
        || (esp_timer_get_time() / 1000) - lastMillisecond < 1000
    ) {
        return ;
    }

    if (registeredDevice(mac, db.ezdata2.loginName, db.ezdata2.password, db.ezdata2.devToken)) {
        log_w("registeredDevice success");
    } else {
        log_w("registeredDevice error");
        log_i("Login ...");
        db.ezdata2.loginName = "USER_" + mac;
        db.ezdata2.password = "12345678";
        if (login(db.ezdata2.loginName, db.ezdata2.password, db.ezdata2.devToken)) {
            log_w("login success");
        } else {
            log_w("login error");
        }
    }

    ezdataStatus = true;
    lastMillisecond = esp_timer_get_time() / 1000;
    db.saveToFile();

}


void shutdownServiceTask(ButtonEvent_t *buttonEvent) {

    if (buttonEvent->id == E_BUTTON_POWER) {
        shutdown();
    }

}


void buttonTask(void *) {

    for (;;) {
        btnA.tick();
        btnB.tick();
        btnPower.tick();
        delay(10);
    }

    vTaskDelete(NULL);
}


void btnAClickEvent() {
    log_d("btnAClickEvent");

    BUTTON_TONE();

    ButtonEvent_t buttonEvent = { .id = E_BUTTON_A };
    if (xQueueSendToBack(buttonEventQueue, &buttonEvent, ( TickType_t ) 10 ) != pdPASS) {
        log_w("buttonEventQueue send Failed");
    }
}


void btnBClickEvent() {
    log_d("btnBClickEvent");

    BUTTON_TONE();

    ButtonEvent_t buttonEvent = { .id = E_BUTTON_B };
    if (xQueueSendToBack(buttonEventQueue, &buttonEvent, ( TickType_t ) 10 ) != pdPASS) {
        log_w("buttonEventQueue send Failed");
    }
}


void btnPowerClickEvent() {
    log_d("btnBClickEvent");

    BUTTON_TONE();

    ButtonEvent_t buttonEvent = { .id = E_BUTTON_POWER };
    if (xQueueSendToBack(buttonEventQueue, &buttonEvent, ( TickType_t ) 10 ) != pdPASS) {
        log_w("buttonEventQueue send Failed");
    }
}


void listDirectory(fs::FS &fs, const char * dirname, uint8_t levels) {
    log_i("Listing directory: %s", dirname);

    File root = fs.open(dirname);
    if(!root) {
        log_w("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        log_w(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            log_i("  DIR : %s", file.name());
            if(levels){
                listDirectory(fs, file.path(), levels -1);
            }
        } else {
            log_i("  FILE : %s\tSIZE: %d", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}


void wifiAPSTASetup() {
    log_i("WiFi setup...");

    WiFi.disconnect();
    delay(1000);

    log_i("WiFi: Set mode to WIFI_AP_STA");
    WiFi.mode(WIFI_AP_STA);
    WiFi.onEvent(onWiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(onWiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    if (db.wifi.ssid.length() == 0) {
        log_w("SSID missing");
        statusView.updateNetworkStatus("WIFI", "no ssid");
    } else {
        
    }

    WiFi.begin(db.wifi.ssid.c_str(), db.wifi.password.c_str());

    log_i("Waiting for WiFi");

    mac = WiFi.softAPmacAddress();
    mac.toUpperCase();
    mac.replace(":", "");
    apSSID = "M5-" + mac.substring(6, 12);
}


void wifiStartAP() {
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    delay(200);
    if (WiFi.softAP(apSSID.c_str()) != true) {
        log_i("WiFi: failed to create softAP");
        return;
    }

    log_i("WiFi: softAP has been established");
    log_i("WiFi: please connect to the %s\r\n", apSSID.c_str());
}


void onWiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_i("WiFi connected");
    log_i("IP address: %s", IPAddress(info.got_ip.ip_info.ip.addr).toString().c_str());

    db.factoryState = false;
    db.saveToFile();

    sntp_servermode_dhcp(1);
    configTzTime(
        db.ntp.tz.c_str(),
        db.ntp.ntpServer0.c_str(),
        db.ntp.ntpServer1.c_str(),
        "pool.ntp.org"
    );

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 1000)) {
        I2C_BM8563_TimeTypeDef I2C_BM8563_TimeStruct;
        I2C_BM8563_TimeStruct.hours = timeinfo.tm_hour;
        I2C_BM8563_TimeStruct.minutes = timeinfo.tm_min;
        I2C_BM8563_TimeStruct.seconds = timeinfo.tm_sec;
        bm8563.setTime(&I2C_BM8563_TimeStruct);

        I2C_BM8563_DateTypeDef I2C_BM8563_DateStruct;
        I2C_BM8563_DateStruct.year = 1900 + timeinfo.tm_year;
        I2C_BM8563_DateStruct.month = timeinfo.tm_mon + 1;
        I2C_BM8563_DateStruct.date = timeinfo.tm_mday;
        I2C_BM8563_DateStruct.weekDay = timeinfo.tm_wday;
        bm8563.setDate(&I2C_BM8563_DateStruct);
    }

    // xTaskCreatePinnedToCore(ezdataServiceTask, "EzData Service Task", 4096, NULL, 5, NULL, APP_CPU_NUM);
}


void onWiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_w("WiFi lost connection. Reason: %d", info.wifi_sta_disconnected.reason);

    statusView.updateNetworkStatus("WiFi", "lost");
}


bool uploadSensorRawData(EzData &ezdataHanlder) {
    bool ret = false;
    cJSON *rspObject = NULL;
    cJSON *sen55Object = NULL;
    cJSON *scd40Object = NULL;
    char *buf = NULL;
    String data;

    rspObject = cJSON_CreateObject();
    if (rspObject == NULL) {
        goto OUT1;
    }

    sen55Object = cJSON_CreateObject();
    if (sen55Object == NULL) {
        goto OUT;
    }
    scd40Object = cJSON_CreateObject();
    if (scd40Object == NULL) {
        goto OUT;
    }

    cJSON_AddItemToObject(rspObject, "sen55", sen55Object);
    cJSON_AddNumberToObject(sen55Object, "pm1.0", sensor.sen55.massConcentrationPm1p0);
    cJSON_AddNumberToObject(sen55Object, "pm2.5", sensor.sen55.massConcentrationPm2p5);
    cJSON_AddNumberToObject(sen55Object, "pm4.0", sensor.sen55.massConcentrationPm4p0);
    cJSON_AddNumberToObject(sen55Object, "pm10.0", sensor.sen55.massConcentrationPm10p0);
    cJSON_AddNumberToObject(sen55Object, "humidity", sensor.sen55.ambientHumidity);
    cJSON_AddNumberToObject(sen55Object, "temperature", sensor.sen55.ambientTemperature);
    cJSON_AddNumberToObject(sen55Object, "voc", sensor.sen55.vocIndex);
    cJSON_AddNumberToObject(sen55Object, "nox", sensor.sen55.noxIndex);

    cJSON_AddItemToObject(rspObject, "scd40", scd40Object);
    cJSON_AddNumberToObject(scd40Object, "co2", sensor.scd40.co2);
    cJSON_AddNumberToObject(scd40Object, "humidity", sensor.scd40.humidity);
    cJSON_AddNumberToObject(scd40Object, "temperature", sensor.scd40.temperature);

    buf = cJSON_PrintUnformatted(rspObject);
    data = buf;
    data.replace("\"", "\\\"");
    if (ezdataHanlder.set(data)) {
        log_i("ok");
        ret = true;
    } else {
        log_w("error");
        ret = false;
    }
OUT:
    free(buf);
    cJSON_Delete(rspObject);
OUT1:
    return ret;
}


/**
 * This method of obtaining wake status is not 100% accurate.
 */
WakeupType_t getDeviceWakeupType() {
    WakeupType_t wakeupType = E_WAKEUP_TYPE_UNKNOWN;
    time_t seconds = (time_t)preferences.getLong("sleep_timestamp");
    log_d("bm8563 timestamp: %ld", bm8563ToTime(bm8563));
    log_d("last sleep timestamp: %ld", seconds);
    // if (bm8563ToTime(bm8563) - seconds < (db.rtc.sleepInterval - 3)) {
        pinMode(USER_BUTTON_POWER, INPUT);
        if (digitalRead(USER_BUTTON_POWER) == 0) {
            log_i("Button triggers wake-up");
            wakeupType = E_WAKEUP_TYPE_USER;
        } else {
            log_i("USB wake-up");
            wakeupType = E_WAKEUP_TYPE_USB;
        }
    // } else 
    if (abs(bm8563ToTime(bm8563) - seconds - db.rtc.sleepInterval) < 3) {
        log_i("RTC wake-up");
        wakeupType = E_WAKEUP_TYPE_RTC;
    }
    return wakeupType;
}


time_t bm8563ToTime(I2C_BM8563 &bm8563) {
    I2C_BM8563_TimeTypeDef I2C_BM8563_TimeStruct; 
    bm8563.getTime(&I2C_BM8563_TimeStruct);
    I2C_BM8563_DateTypeDef I2C_BM8563_DateStruct;
    bm8563.getDate(&I2C_BM8563_DateStruct);
    struct tm tm = {
        .tm_sec = I2C_BM8563_TimeStruct.seconds,
        .tm_min = I2C_BM8563_TimeStruct.minutes,
        .tm_hour = I2C_BM8563_TimeStruct.hours,
        .tm_mday = I2C_BM8563_DateStruct.date,
        .tm_mon = I2C_BM8563_DateStruct.month - 1,
        .tm_year = (int)(I2C_BM8563_DateStruct.year - 1900),
    };

    /**
     * The time obtained by BM8563 is the time in the current time zone.
     * Use the mktime function to convert the time into a timestamp, and you
     * need to eliminate the difference caused by the time zone.
     */
    configTzTime(
        "GMT0",
        db.ntp.ntpServer0.c_str(),
        db.ntp.ntpServer1.c_str(),
        "pool.ntp.org"
    );
    time_t time = mktime(&tm);
    configTzTime(
        db.ntp.tz.c_str(),
        db.ntp.ntpServer0.c_str(),
        db.ntp.ntpServer1.c_str(),
        "pool.ntp.org"
    );
    return time;
}


void countdownServiceTask() {
    static uint32_t cur = esp_timer_get_time() / 1000;

    if (runMode == E_RUN_MODE_MAIN) {
        if (esp_timer_get_time() / 1000 - cur > AIRQ_SHUTDOWN_TIMEOUT * 1000) {
            shutdown();
        }
    } else {
        cur = esp_timer_get_time() / 1000;
    }

    // if (ezdataState == E_EZDATA_STATE_SUCCESS) {
    //     shutdown();
    // }

}


void shutdown() {

    time_t timestamp = bm8563ToTime(bm8563);
    log_i("BM8653 timestamp: %ld", timestamp);
    preferences.putLong("sleep_timestamp", timestamp);
    preferences.end();

    // scd4x.powerDown();
    // digitalWrite(SEN55_POWER_EN, HIGH);
    // lcd.powerSaveOn();
    lcd.sleep();
    lcd.waitDisplay();
    // delay(2000);
    if (db.factoryState == false) {
        bm8563.clearIRQ();
        bm8563.SetAlarmIRQ(db.rtc.sleepInterval);
    }

    log_i("shutdown");
    delay(10);
    digitalWrite(POWER_HOLD, LOW);

    lcd.wakeup();
    lcd.waitDisplay();
    preferences.begin("airq", false);
    log_i("USB powered, continue to operate");
    wakeupType = E_WAKEUP_TYPE_USB;
}