#include "EzData.hpp"

#include <Arduino.h>
#include <stdint.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


EzData::EzData(const char *dev_token, const char *key) {
    _device_token = dev_token;
    _key = key;
}


EzData::EzData(const String dev_token, const char *key) {
    _device_token = dev_token;
    _key = key;
}

EzData::EzData(const String dev_token, const String key) {
    _device_token = dev_token;
    _key = key;
}

EzData::EzData(const char *dev_token, const String key) {
    _device_token = dev_token;
    _key = key;
}

EzData::EzData(const char *data_token) {
    _device_token = data_token;
    _public = true;
}

EzData::EzData(const String data_token) {
    _device_token = data_token;
    _public = true;
}


EzData::~EzData() {}


void EzData::setDeviceToken(const String &dev_token) {
    _device_token = dev_token;
    log_d("%s", _device_token.c_str());
}

template<typename T, typename std::enable_if<std::is_integral<T>::value, T> :: type* = nullptr>
bool EzData::set(T value) {
    if (_public) {
        return false;
    }
    char buf[256] = { 0 };
    snprintf(
        buf,
        sizeof(buf) - 1,
        "{\"dataType\": \"int\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %d}",
        _key.c_str(),
        value
    );
    return _set((uint8_t *)buf, strlen(buf));
}

bool EzData::set(const float value) {
    if (_public) {
        return false;
    }
    char buf[256] = { 0 };
    snprintf(
        buf,
        sizeof(buf) - 1,
        "{\"dataType\": \"double\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %f}",
        _key.c_str(),
        value
    );
    return _set((uint8_t *)buf, strlen(buf));
}

bool EzData::set(const double value) {
    if (_public) {
        return false;
    }
    char buf[256] = { 0 };
    snprintf(
        buf,
        sizeof(buf) - 1,
        "{\"dataType\": \"double\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %f}",
        _key.c_str(),
        value
    );
    return _set((uint8_t *)buf, strlen(buf));
}

bool EzData::set(const String value) {
    if (_public) {
        return false;
    }
    char *buf = (char *)malloc(256 + value.length());
    memset(buf, 0, 256 + value.length());
    snprintf(
        buf,
        256 + value.length() - 1,
        "{\"dataType\": \"string\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": \"%s\"}",
        _key.c_str(),
        value.c_str()
    );
    log_d("%s", buf);
    bool ret = _set((uint8_t *)buf, strlen(buf));
    free(buf);
    return ret;
}

bool EzData::set(const char *value, size_t len) {
    if (_public) {
        return false;
    }
    char *buf = (char *)malloc(256 + len);
    memset(buf, 0, 256 + len);
    snprintf(
        buf,
        256 + len - 1,
        "{\"dataType\": \"string\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": \"%s\"}",
        _key.c_str(),
        value
    );
    
    log_d("%s", buf);
    bool ret = _set((uint8_t *)buf, strlen(buf));
    free(buf);
    return ret;
}

bool EzData::_set(uint8_t *payload, size_t size) {
    bool ret = false;
    HTTPClient http;
    String url = String("http://ezdata2.m5stack.com/api/v2/") + _device_token + String("/add");
    DynamicJsonDocument doc(1024);

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST((uint8_t *)payload, size);
    if (httpCode == HTTP_CODE_OK) {
        DeserializationError error = deserializeJson(doc, http.getString());
        ret = false;
        if (error) {
            log_e("deserializeJson() failed: %s", error.c_str());
            goto out;
        }
        int code = doc["code"].as<int>();
        if (code == 200) {
            ret = true;
        } else {
            ret = false;
        }
    } else {
        ret = false;
    }
out:
    http.end();
    return ret;
}

template <typename T>
bool EzData::get(T &retValue) {
    if (_public) {
        return false;
    }
    return _get(retValue);
}

template <typename T>
bool EzData::_get(T &retValue) {
    bool ret = false;
    HTTPClient http;
    DynamicJsonDocument doc(1024);
    String url = String("http://ezdata2.m5stack.com/api/v2/")
                + _device_token
                + String("/dataByKey/")
                + _key;

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String rsp = http.getString();
        log_e("%s", rsp.c_str());
        DeserializationError error = deserializeJson(doc, rsp);
        if (error) {
            log_e("deserializeJson() failed: %s", error.c_str());
            goto out;
        }
        retValue = doc["data"]["value"].as<T>();
        ret = true;
    }
    http.end();
    return ret;
out:
    http.end();
    return ret;
}

bool EzData::del() {
    bool ret = false;
    HTTPClient http;
    String url = String("http://ezdata2.m5stack.com/api/v2/")
                + _device_token
                + String("/delete/")
                + _key;

    http.begin(url);
    int httpCode = http.sendRequest("DELETE");
    if (httpCode == HTTP_CODE_OK) {
        ret = true;
    } else {
        ret = false;
    }
    http.end();
    return ret;
}


bool registeredDevice(const String &mac, String &loginName, String &password, String &devToken) {
    bool ret = false;
    HTTPClient http;
    char buf[256] = { 0 };
    String url = "http://ezdata2.m5stack.com/api/v2/AirQ/registered";
    DynamicJsonDocument doc(1024);

    snprintf(buf, sizeof(buf) - 1, "{\"mac\": \"%s\"}", mac.c_str());

    log_d("%s", buf);

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST((uint8_t *)buf, strlen(buf));
    if (httpCode == HTTP_CODE_OK) {
        String rsp = http.getString();
        log_d("rsp: %s", rsp.c_str());
        DeserializationError error = deserializeJson(doc, rsp);
        ret = false;
        if (error) {
            log_e("deserializeJson() failed: %s", error.c_str());
            goto out;
        }
        int code = doc["code"].as<int>();
        if (code == 200) {
            ret = true;
            loginName = doc["data"]["loginName"].as<String>();
            password = doc["data"]["password"].as<String>();
            devToken = doc["data"]["deviceToken"].as<String>();
        } else {
            ret = false;
        }
    } else {
        ret = false;
    }
out:
    http.end();
    return ret;
}

bool login(const String &loginName, const String &password, String &deviceToken) {
    bool ret = false;
    HTTPClient http;
    char buf[256] = { 0 };
    String url = "http://ezdata2.m5stack.com/api/v2/AirQ/doLogin";
    DynamicJsonDocument doc(1024);

    snprintf(buf, sizeof(buf) - 1, "{\"loginName\": \"%s\", \"password\": \"%s\"}", loginName.c_str(), password.c_str());

    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST((uint8_t *)buf, strlen(buf));
    if (httpCode == HTTP_CODE_OK) {
        String rsp = http.getString();
        log_d("rsp: %s", rsp.c_str());
        DeserializationError error = deserializeJson(doc, rsp);
        ret = false;
        if (error) {
            log_e("deserializeJson() failed: %s", error.c_str());
            goto out;
        }
        int code = doc["code"].as<int>();
        if (code == 200) {
            ret = true;
            deviceToken = doc["data"]["deviceToken"].as<String>();
            log_d("%s", deviceToken.c_str());
        } else {
            ret = false;
        }
    } else {
        ret = false;
    }
out:
    http.end();
    return ret;
}