#include "DataBase.hpp"
#include "config.h"

#include <cJSON.h>
#include <LittleFS.h>


void DataBase::saveToFile() {
    cJSON *rootObject = NULL;
    cJSON *configObject = NULL;
    cJSON *wifiObject = NULL;
    cJSON *rtcObject = NULL;
    cJSON *ntpObject = NULL;
    cJSON *ezdataObject = NULL;
    cJSON *buzzerObject = NULL;
    File configfile;
    char *str = NULL;

    rootObject = cJSON_CreateObject();
    if (rootObject == NULL) {
        goto OUT;
    }

    configObject = cJSON_CreateObject();
    if (configObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(rootObject, "config", configObject);

    cJSON_AddBoolToObject(configObject, "factory_state", factoryState);

    wifiObject = cJSON_CreateObject();
    if (wifiObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(configObject, "wifi", wifiObject);
    cJSON_AddStringToObject(wifiObject, "ssid", wifi.ssid.c_str());
    cJSON_AddStringToObject(wifiObject, "password", wifi.password.c_str());

    rtcObject = cJSON_CreateObject();
    if (rtcObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(configObject, "rtc", rtcObject);
    cJSON_AddNumberToObject(rtcObject, "sleep_interval", rtc.sleepInterval);

    ntpObject = cJSON_CreateObject();
    if (ntpObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(configObject, "ntp", ntpObject);
    cJSON_AddStringToObject(ntpObject, "server_0", ntp.ntpServer0.c_str());
    cJSON_AddStringToObject(ntpObject, "server_1", ntp.ntpServer1.c_str());
    cJSON_AddStringToObject(ntpObject, "tz", ntp.tz.c_str());

    ezdataObject = cJSON_CreateObject();
    if (ezdataObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(configObject, "ezdata2", ezdataObject);
    cJSON_AddStringToObject(ezdataObject, "dev_token", ezdata2.devToken.c_str());

    buzzerObject = cJSON_CreateObject();
    if (buzzerObject == NULL) {
        goto OUT;
    }
    cJSON_AddItemToObject(configObject, "buzzer", buzzerObject);
    cJSON_AddBoolToObject(buzzerObject, "mute", buzzer.onoff);

    configfile = FILESYSTEM.open("/db.json", FILE_WRITE);
    str = cJSON_Print(rootObject);
    configfile.write((const uint8_t *)str, strlen(str));
    configfile.close();

OUT:
    free(str);
    cJSON_Delete(rootObject);
    return;
}


void DataBase::dump() {
    log_d("config:");
    log_d("  factory_state: %d", factoryState);

    log_d("  wifi:");
    log_d("    ssid: %s", wifi.ssid.c_str());
    log_d("    password: %s", wifi.password.c_str());

    log_d("  rtc:");
    log_d("    sleep_interval: %d", rtc.sleepInterval);

    log_d("  ntp:");
    log_d("    server_0: %s", ntp.ntpServer0.c_str());
    log_d("    server_1: %s", ntp.ntpServer1.c_str());
    log_d("    tz: %s", ntp.tz.c_str());

    log_d("  ezdata2:");
    log_d("    dev_token: %s", ezdata2.devToken.c_str());

    log_d("  buzzer:");
    log_d("    onoff: %d", buzzer.onoff);
}


void DataBase::loadFromFile(void) {
    log_i("Load DateBase...");

    File dbfile = FILESYSTEM.open("/db.json", "r");
    if (!dbfile) {
        log_i("Error opening file.");
        return;
    }

    char *buffer = (char *)malloc(dbfile.size());
    size_t buffer_len = dbfile.size();

    while (dbfile.available()) {
        dbfile.readBytes(buffer, dbfile.size());
    }
    dbfile.close();

    cJSON *rootObject = cJSON_ParseWithLength(buffer, buffer_len);
    if (rootObject == NULL) {
        log_i("Error opening file.");
        return;
    }

    cJSON *configObject = cJSON_GetObjectItem(rootObject, "config");
    cJSON *wifiObject = cJSON_GetObjectItem(configObject, "wifi");
    cJSON *ssidObject = cJSON_GetObjectItem(wifiObject, "ssid");
    cJSON *pskObject = cJSON_GetObjectItem(wifiObject, "password");
    wifi.ssid = String(ssidObject->valuestring);
    wifi.password = String(pskObject->valuestring);

    cJSON *factoryStateObject = cJSON_GetObjectItem(configObject, "factory_state");
    factoryState = cJSON_IsTrue(factoryStateObject);

    cJSON *rtcObject = cJSON_GetObjectItem(configObject, "rtc");
    cJSON *sleepIntervalObject = cJSON_GetObjectItem(rtcObject, "sleep_interval");
    rtc.sleepInterval = sleepIntervalObject->valueint;

    cJSON *ntpObject = cJSON_GetObjectItem(configObject, "ntp");
    cJSON *server0Object = cJSON_GetObjectItem(ntpObject, "server_0");
    cJSON *server1Object = cJSON_GetObjectItem(ntpObject, "server_1");
    cJSON *tzObject = cJSON_GetObjectItem(ntpObject, "tz");
    ntp.ntpServer0 = String(server0Object->valuestring);
    ntp.ntpServer1 = String(server1Object->valuestring);
    ntp.tz = String(tzObject->valuestring);

    cJSON *ezdataObject = cJSON_GetObjectItem(configObject, "ezdata2");
    cJSON *tokenObject = cJSON_GetObjectItem(ezdataObject, "dev_token");
    ezdata2.devToken = String(tokenObject->valuestring);

    cJSON *buzzerObject = cJSON_GetObjectItem(configObject, "buzzer");
    if (cJSON_IsTrue(cJSON_GetObjectItem(buzzerObject, "mute"))) {
        buzzer.onoff = true;
    } else {
        buzzer.onoff = false;
    }

    cJSON_Delete(rootObject);
    free(buffer);
}


DataBase db;
