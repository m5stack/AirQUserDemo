#include "AppWeb.hpp"

#include <WebServer.h>
#include <cJSON.h>
#include <LittleFS.h>
#include <WString.h>

#include <SensirionI2CScd4x.h>
#include <SensirionI2CSen5x.h>

#include "config.h"
#include "DataBase.hpp"

WebServer server(80);
static TaskHandle_t webTaskHandler;
bool webServerState = false;

extern SensirionI2CScd4x scd4x;
extern SensirionI2CSen5x sen5x;

static void postWiFiConnect();
static void getWiFiStatus();
static void getWiFiList();
static void postEzDataConfig();
static void getStatus();
static void getInfo();
static void getConfig();
static void postConfig();
static void postAPControl();
static void webTask(void *);

static bool getSCD40MeasurementResult(SensirionI2CScd4x& scd4x, uint16_t& co2, float& temperature, float& humidity);

void appWebServer(void) {
    if (webServerState == true) {
        return;
    }
    server.serveStatic("/", FILESYSTEM, "/www/index.html");
    // onServeStatic("/www");

    server.on("/api/v1/wifi_connect", HTTP_POST, postWiFiConnect);
    server.on("/api/v1/wifi_status", HTTP_GET, getWiFiStatus);
    server.on("/api/v1/wifi_list", HTTP_GET, getWiFiList);
    server.on("/api/v1/ezdata_config", HTTP_POST, postEzDataConfig);
    server.on("/api/v1/status", HTTP_GET, getStatus);
    server.on("/api/v1/info", HTTP_GET, getInfo);
    server.on("/api/v1/config", HTTP_GET, getConfig);
    server.on("/api/v1/config", HTTP_POST, postConfig);
    server.on("/api/v1/ap_control", HTTP_POST, postAPControl);
    // called when the url is not defined here
    server.onNotFound([]() {
        server.send(404, "text/plain", "FileNotFound");
    });

    server.begin();
    webServerState = true;
    log_i("HTTP server started");
    xTaskCreatePinnedToCore(webTask, "webTask", 8192, NULL, 5, &webTaskHandler, APP_CPU_NUM);
}


void appWebServerClose(void) {
    server.close();
    webServerState = false;
    vTaskDelete(webTaskHandler);
}



static void webTask(void *) {
    for (;;) {
        server.handleClient();
        delay(10);
    }
    vTaskDelete(NULL);
}


static void postWiFiConnect() {
    cJSON *reqObject = NULL;
    cJSON *wifiObject = NULL;
    bool flag = false;
    char *str = NULL;

    String content = server.arg("plain");
    log_d("POST /api/v1/wifi_connect content: '%s'", content.c_str());
    reqObject = cJSON_Parse(content.c_str());
    if (reqObject == NULL) {
        log_w("JSON parse error");
        log_w("payload: '%s'", server.arg("plain").c_str());
        return;
    }

    wifiObject = cJSON_GetObjectItem(reqObject, "wifi");
    if (wifiObject) {
        cJSON *ssidObject = cJSON_GetObjectItem(wifiObject, "ssid");
        if (ssidObject != NULL) {
            db.wifi.ssid = ssidObject->valuestring;
            flag = true;
        }
        cJSON *pskObject = cJSON_GetObjectItem(wifiObject, "password");
        if (ssidObject != NULL) {
            db.wifi.password = pskObject->valuestring;
            flag = true;
        }
    }

    db.factoryState = false;
    db.saveToFile();

    if (flag || WiFi.isConnected() != true) {
        WiFi.disconnect();
        delay(200);
        WiFi.begin(db.wifi.ssid.c_str(), db.wifi.password.c_str());
        log_i("WiFi connect ...");
        db.isConfigState = true;
    }

    server.send(200, "application/json", content);
    log_d("POST /api/v1/wifi_connect response: '%s'", content.c_str());
    return;
}


static void getWiFiStatus() {
    cJSON *rspObject = NULL;
    char *str = NULL;

    rspObject = cJSON_CreateObject();
    if (rspObject == NULL) {
        return;
    }

    String mac = WiFi.softAPmacAddress();
    if (WiFi.status() == WL_CONNECTED) {
        cJSON_AddBoolToObject(rspObject, "status", true);
    } else {
        cJSON_AddBoolToObject(rspObject, "status", false);
    }
    cJSON_AddStringToObject(rspObject, "mac", mac.c_str());
    cJSON_AddStringToObject(rspObject, "ip", WiFi.localIP().toString().c_str());
    cJSON_AddBoolToObject(rspObject, "psk_status", db.pskStatus);

    str = cJSON_Print(rspObject);
    log_d("GET /api/v1/wifi_status response: '%s'", str);
    server.send(200, "application/json", str);

    free(str);
    cJSON_Delete(rspObject);
    return;
}


static void getWiFiList() {
    cJSON *rspObject = NULL;
    cJSON *apListObject = NULL;
    int n = 0;
    char *str = NULL;

    rspObject = cJSON_CreateObject();
    if (rspObject == NULL) {
        return ;
    }
    apListObject = cJSON_CreateArray();
    if (apListObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "ap_list", apListObject);

    log_i("WiFi Scan start");
    n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i) {
        log_i("%s", WiFi.SSID(i).c_str());
        cJSON_AddItemToArray(apListObject, cJSON_CreateString(WiFi.SSID(i).c_str()));
    }
    log_i("WiFi Scan done");
    WiFi.scanDelete();
    str = cJSON_Print(rspObject);
    server.send(200, "application/json", str);
    log_d("GET /api/v1/wifi_list response: '%s'", str);
OUT:
    free(str);
    cJSON_Delete(rspObject);
    return ;
}


static void postEzDataConfig() {
    cJSON *reqObject = NULL;
    cJSON *ezdataObject = NULL;
    cJSON *rspObject = NULL;

    String content = server.arg("plain");
    log_d("POST /api/v1/ezdata_config content: '%s'", server.arg("plain").c_str());
    reqObject = cJSON_Parse(content.c_str());
    if (reqObject == NULL) {
        log_w("JSON parse error");
        log_w("payload: '%s'", server.arg("plain").c_str());
        return;
    }

    ezdataObject = cJSON_GetObjectItem(reqObject, "ezdata2");
    if (ezdataObject) {
        cJSON *tokenObject = cJSON_GetObjectItem(ezdataObject, "dev_token");
        db.ezdata2.devToken = tokenObject->valuestring;
        db.factoryState = false;
    }

    server.send(201, "application/json", server.arg("plain"));
    log_d("POST /api/v1/ezdata_config response: '%s'", server.arg("plain").c_str());

    db.saveToFile();

    cJSON_Delete(reqObject);
    cJSON_Delete(rspObject);
    return;
}


static void getStatus() {
    cJSON *rspObject = NULL;
    cJSON *sen55Object = NULL;
    cJSON *scd40Object = NULL;
    char *str = NULL;

    rspObject = cJSON_CreateObject();
    if (rspObject == NULL) {
        goto OUT1;
    }

    sen55Object = cJSON_CreateObject();
    if (sen55Object == NULL) {
        goto OUT;
    }
    float massConcentrationPm1p0;
    float massConcentrationPm2p5;
    float massConcentrationPm4p0;
    float massConcentrationPm10p0;
    float ambientHumidity;
    float ambientTemperature;
    float vocIndex;
    float noxIndex;
    sen5x.readMeasuredValues(
        massConcentrationPm1p0,
        massConcentrationPm2p5,
        massConcentrationPm4p0,
        massConcentrationPm10p0,
        ambientHumidity,
        ambientTemperature,
        vocIndex,
        noxIndex
    );
    cJSON_AddItemToObject(rspObject, "sen55", sen55Object);
    cJSON_AddNumberToObject(sen55Object, "pm1.0", massConcentrationPm1p0);
    cJSON_AddNumberToObject(sen55Object, "pm2.5", massConcentrationPm2p5);
    cJSON_AddNumberToObject(sen55Object, "pm4.0", massConcentrationPm4p0);
    cJSON_AddNumberToObject(sen55Object, "pm10.0", massConcentrationPm10p0);
    cJSON_AddNumberToObject(sen55Object, "humidity", ambientHumidity);
    cJSON_AddNumberToObject(sen55Object, "temperature", ambientTemperature);
    cJSON_AddNumberToObject(sen55Object, "voc", vocIndex);
    cJSON_AddNumberToObject(sen55Object, "nox", noxIndex);

    scd40Object = cJSON_CreateObject();
    if (scd40Object == NULL) {
        goto OUT;
    }
    uint16_t co2;
    float humidity;
    float temperature;
    getSCD40MeasurementResult(scd4x, co2, humidity, temperature);
    cJSON_AddItemToObject(rspObject, "scd40", scd40Object);
    cJSON_AddNumberToObject(scd40Object, "co2", co2);
    cJSON_AddNumberToObject(scd40Object, "humidity", humidity);
    cJSON_AddNumberToObject(scd40Object, "temperature", temperature);

    struct tm timeinfo;
    getLocalTime(&timeinfo, 1000);
    cJSON_AddStringToObject(rspObject, "time", asctime((const struct tm *)&timeinfo));
    cJSON_AddNumberToObject(rspObject, "interval", db.rtc.sleepInterval);

    str = cJSON_Print(rspObject);
    server.send(200, "application/json", str);
    log_d("GET /api/v1/status response: '%s'", str);
OUT:
    free(str);
    cJSON_Delete(rspObject);
OUT1:
    return;
}


static void getInfo() {
    cJSON *rspObject = NULL;
    cJSON *sysObject = NULL;
    cJSON *archObject = NULL;
    cJSON *memObject = NULL;
    cJSON *fsObject = NULL;
    cJSON *apObject = NULL;
    cJSON *staObject = NULL;
    char *str = NULL;

    rspObject = cJSON_CreateObject();
    if (rspObject == NULL) {
        goto OUT1;
    }

    sysObject = cJSON_CreateObject();
    if (sysObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "sys", sysObject);
    cJSON_AddStringToObject(sysObject, "model", "M5STACK AirQ");
    cJSON_AddStringToObject(sysObject, "fw", APP_VERSION);
    cJSON_AddStringToObject(sysObject, "sdk", ESP.getSdkVersion());
    archObject = cJSON_CreateObject();
    if (archObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "arch", archObject);
    cJSON_AddStringToObject(archObject, "mfr", "Espressif");
    cJSON_AddStringToObject(archObject, "model", ESP.getChipModel());
    cJSON_AddNumberToObject(archObject, "revision", ESP.getChipRevision());
    if (!strncmp(ESP.getChipModel(), "ESP32-S3", strlen("ESP32-S3"))) {
        cJSON_AddStringToObject(archObject, "cpu", "XTensa® dual-core LX7");
    } else if (!strncmp(ESP.getChipModel(), "ESP32-S2", strlen("ESP32-S2"))) {
        cJSON_AddStringToObject(archObject, "cpu", "XTensa® single-core LX7");
    } else if (!strncmp(ESP.getChipModel(), "ESP32-C3", strlen("ESP32-C3"))) {
        cJSON_AddStringToObject(archObject, "cpu", "RISC-V");
    } else if (!strncmp(ESP.getChipModel(), "ESP32", strlen("ESP32"))) {
        cJSON_AddStringToObject(archObject, "cpu", "XTensa® dual-core LX6");
    }
    cJSON_AddNumberToObject(archObject, "freq", ESP.getCpuFreqMHz());

    memObject = cJSON_CreateObject();
    if (memObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "mem", memObject);
    cJSON_AddNumberToObject(memObject, "total", ESP.getHeapSize());
    cJSON_AddNumberToObject(memObject, "free", ESP.getFreeHeap());

    fsObject = cJSON_CreateObject();
    if (fsObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "fs", fsObject);
    cJSON_AddNumberToObject(fsObject, "total", FILESYSTEM.totalBytes());
    cJSON_AddNumberToObject(fsObject, "used", FILESYSTEM.usedBytes());
    cJSON_AddNumberToObject(fsObject, "free", FILESYSTEM.totalBytes() - FILESYSTEM.usedBytes());

    apObject = cJSON_CreateObject();
    if (apObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "ap", apObject);
    cJSON_AddStringToObject(apObject, "ssid", WiFi.softAPSSID().c_str());
    cJSON_AddNumberToObject(apObject, "num", WiFi.softAPgetStationNum());

    staObject = cJSON_CreateObject();
    if (staObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(rspObject, "sta", staObject);
    cJSON_AddStringToObject(staObject, "ssid", db.wifi.ssid.c_str());
    cJSON_AddStringToObject(staObject, "status", WiFi.isConnected() ? "connected" : "disconnect");

    cJSON_AddBoolToObject(rspObject, "factory_state", db.factoryState);

    str = cJSON_Print(rspObject);
    server.send(200, "application/json", str);
    log_d("GET /api/v1/info response: '%s'", str);
OUT:
    free(str);
    cJSON_Delete(rspObject);
OUT1:
    return;
}


static void getConfig() {
    File file = FILESYSTEM.open("/db.json", "r");
    server.streamFile(file, "application/json");
    log_d("GET /api/v1/config response: '/db.json'");
    file.close();
    return;
}


static void postConfig() {

    cJSON *reqObject = NULL;
    cJSON *configObject = NULL;
    cJSON *wifiObject = NULL;
    cJSON *rtcObject = NULL;
    cJSON *ntpObject = NULL;
    cJSON *ezdataObject = NULL;
    cJSON *buzzerObject = NULL;
    bool flag = false;

    String content = server.arg("plain");
    log_d("POST /api/v1/config content: '%s'", content.c_str());
    reqObject = cJSON_Parse(content.c_str());
    if (reqObject == NULL) {
        log_w("JSON parse error");
        log_w("payload: '%s'", server.arg("plain").c_str());
        return;
    }

    configObject = cJSON_GetObjectItem(reqObject, "config");
    wifiObject = cJSON_GetObjectItem(configObject, "wifi");
    if (wifiObject) {
        cJSON *ssidObject = cJSON_GetObjectItem(wifiObject, "ssid");
        if (String(ssidObject->valuestring) != db.wifi.ssid && db.wifi.ssid.length() > 0) {
            db.wifi.ssid = ssidObject->valuestring;
            flag = true;
        }
        cJSON *pskObject = cJSON_GetObjectItem(wifiObject, "password");
        if (db.wifi.ssid.length() > 0 && String(pskObject->valuestring) != db.wifi.password) {
            db.wifi.password = pskObject->valuestring;
            flag = true;
        }
    }

    rtcObject = cJSON_GetObjectItem(configObject, "rtc");
    if (rtcObject) {
        cJSON * sleepIntervalObject = cJSON_GetObjectItem(rtcObject, "sleep_interval");
        db.rtc.sleepInterval = sleepIntervalObject->valueint;
    }

    ntpObject = cJSON_GetObjectItem(configObject, "ntp");
    if (ntpObject) {
        cJSON *ntpServer0Object = cJSON_GetObjectItem(ntpObject, "server_0");
        cJSON *ntpServer1Object = cJSON_GetObjectItem(ntpObject, "server_1");
        cJSON *tzObject = cJSON_GetObjectItem(ntpObject, "tz");
        if (String(ntpServer0Object->valuestring) != db.ntp.ntpServer0
            || String(ntpServer1Object->valuestring) != db.ntp.ntpServer1
            || String(tzObject->valuestring) != db.ntp.tz
        ) {
            db.ntp.ntpServer0 = String(ntpServer0Object->valuestring);
            db.ntp.ntpServer1 = String(ntpServer1Object->valuestring);
            db.ntp.tz = String(tzObject->valuestring);
            configTzTime(
                db.ntp.tz.c_str(),
                db.ntp.ntpServer0.c_str(),
                db.ntp.ntpServer1.c_str(),
                "pool.ntp.org"
            );
        }
    }

    ezdataObject = cJSON_GetObjectItem(configObject, "ezdata2");
    if (ezdataObject) {
        cJSON *tokenObject = cJSON_GetObjectItem(ezdataObject, "dev_token");
        db.ezdata2.devToken = tokenObject->valuestring;
        db.factoryState = false;
    }

    buzzerObject = cJSON_GetObjectItem(configObject, "buzzer");
    if (buzzerObject) {
        if (cJSON_IsTrue(cJSON_GetObjectItem(buzzerObject, "mute"))) {
            db.buzzer.onoff = true;
            ledcAttachPin(BUZZER_PIN, 0);
        } else {
            db.buzzer.onoff = false;
            ledcDetachPin(BUZZER_PIN);
        }
    }

    cJSON_AddBoolToObject(reqObject, "factory_state", db.factoryState);

    db.saveToFile();

    server.send(201, "application/json", server.arg("plain"));
    log_d("POST /api/v1/config response: '%s'", content.c_str());

    if (flag || WiFi.isConnected() != true) {
        WiFi.disconnect();
        delay(200);
        WiFi.begin(db.wifi.ssid.c_str(), db.wifi.password.c_str());
    }

    cJSON_Delete(reqObject);
    return;
}


static void postAPControl() {
    cJSON *rspObject = NULL;
    char *str = NULL;

    rspObject = cJSON_CreateObject();
    if (rspObject == NULL) {
        return;
    }

    bool status = WiFi.softAPdisconnect();

    cJSON_AddBoolToObject(rspObject, "status", status);

    str = cJSON_Print(rspObject);
    server.send(200, "application/json", str);

    log_d("POST /api/v1/ap_control response: '%s'", str);

    delay(1000);

    free(str);
    cJSON_Delete(rspObject);
    return;
}


static bool getSCD40MeasurementResult(SensirionI2CScd4x& scd4x, uint16_t& co2, float& temperature, float& humidity)
{
    char _errorMessage[256];

    bool isDataReady = false;
    uint16_t error = scd4x.getDataReadyFlag(isDataReady);
    if (error) {
        errorToString(error, _errorMessage, 256);
        log_w("Error trying to execute getDataReadyFlag(): %s", _errorMessage);
        return false;
    }
    if (!isDataReady) {
        return false;
    }

    error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error) {
        errorToString(error, _errorMessage, 256);
        log_w("Error trying to execute readMeasurement(): %s", _errorMessage);
        return false;
    } else if (co2 == 0) {
        log_w("Invalid sample detected, skipping.");
        return false;
    } else {
        log_i("SCD40 Measurement Result:");
        log_i("  Co2: %d ppm",co2 );
        log_i("  Temperature: %f °C", temperature);
        log_i("  Humidity: %f %RH", humidity);
        return true;
    }
    return false;
}
