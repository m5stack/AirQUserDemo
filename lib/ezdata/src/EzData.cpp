#include "EzData.hpp"


// EzData::EzData(const char *dev_token, const char *key) {
//     _device_token = dev_token;
//     _key = key;
// }

// EzData::EzData(const String dev_token, const char *key) {
//     _device_token = dev_token;
//     _key = key;
// }

// EzData::EzData(const String dev_token, const String key) {
//     _device_token = dev_token;
//     _key = key;
// }

// EzData::EzData(const char *dev_token, const String key) {
//     _device_token = dev_token;
//     _key = key;
// }

// EzData::EzData(const char *data_token) {
//     _device_token = data_token;
//     _public = true;
// }

// EzData::EzData(const String data_token) {
//     _device_token = data_token;
//     _public = true;
// }

// EzData::~EzData()
// {
// }

// bool EzData::_set(uint8_t *payload, size_t size) {
//     bool ret = false;
//     HTTPClient http;
//     String url = String("http://ezdata2.m5stack.com/api/v2/") + _device_token + String("/add");
//     http.begin(url);
//     http.addHeader("Content-Type", "application/json");
//     int httpCode = http.POST((uint8_t *)payload, size);
//     if (httpCode == HTTP_CODE_OK) {
//         ret = true;
//     } else {
//         ret = false;
//     }
//     http.end();
//     return ret;
// }

// bool EzData::_set(String payload) {
//     return _set((uint8_t *)payload.c_str(), payload.length());
// }

// bool EzData::set(const int8_t value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"int\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %d}",
//         _key.c_str(),
//         value
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

// bool EzData::set(const uint8_t value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"int\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %d}",
//         _key.c_str(),
//         value
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

// bool EzData::set(const int16_t value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"int\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %d}",
//         _key.c_str(),
//         value
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

// bool EzData::set(const uint16_t value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"int\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %d}",
//         _key.c_str(),
//         value
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

// bool EzData::set(const int32_t value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"int\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %d}",
//         _key.c_str(),
//         value
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

// bool EzData::set(const uint32_t value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"int\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %d}",
//         _key.c_str(),
//         value
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

// bool EzData::set(const int64_t value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"int\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %d}",
//         _key.c_str(),
//         value
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

// bool EzData::set(const uint64_t value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"int\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %d}",
//         _key.c_str(),
//         value
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

// bool EzData::set(const float value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"double\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %f}",
//         _key.c_str(),
//         value
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

// bool EzData::set(const double value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"double\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": %f}",
//         _key.c_str(),
//         value
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

// bool EzData::set(const String value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"string\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": \"%s\"}",
//         _key.c_str(),
//         value.c_str()
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

// bool EzData::set(const char *value) {
//     char buf[256] = { 0 };
//     snprintf(
//         buf,
//         sizeof(buf) - 1,
//         "{\"dataType\": \"string\", \"name\": \"%s\", \"permissions\": \"1\", \"value\": \"%s\"}",
//         _key.c_str(),
//         value
//     );
//     return _set((uint8_t *)buf, strlen(buf));
// }

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
        DeserializationError error = deserializeJson(doc, http.getString());
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
        DeserializationError error = deserializeJson(doc, http.getString());
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