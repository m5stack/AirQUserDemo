#include <EzData.hpp>
#include <Arduino.h>
#include <WiFi.h>
#include <List.hpp>
#include <typeinfo>

const char* ssid     = "M5-R&D"; // Change this to your WiFi SSID
const char* password = "echo\"password\">/dev/null"; // Change this to your WiFi password

EzData ezdata("b4d22c0bb4174dcb9655a1a3a09f23c4", "test");
// List list;

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    int32_t timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 10000) {
        timeout += 500;
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    test_int();
    test_float();
    test_double();
    test_str();
}

void loop() {
    delay(100);
}

void test_int(void) {
    int value = 123;
    if (ezdata.set(value)) {
        Serial.println("Set success!");
    } else {
        Serial.println("Set error!");
    }
    int ret;
    if (ezdata.get(ret)) {
        Serial.print("ret: ");
        Serial.println(ret);
        if (ret == value) {
            Serial.println("Get success!");
        } else {
            Serial.println("Get error!");
        }
    } else {
        Serial.println("Get error!");
    }
}

void test_float(void) {
    float value = 12.3;
    if (ezdata.set(value)) {
        Serial.println("Set success!");
    } else {
        Serial.println("Set error!");
    }
    float ret;
    if (ezdata.get(ret)) {
        Serial.print("ret: ");
        Serial.println(ret);
        if (ret == value) {
            Serial.println("Get success!");
        } else {
            Serial.println("Get error!");
        }
    } else {
        Serial.println("Get error!");
    }
}

void test_double(void) {
    double value = 12.34;
    if (ezdata.set(value)) {
        Serial.println("Set success!");
    } else {
        Serial.println("Set error!");
    }
    double ret;
    if (ezdata.get(ret)) {
        Serial.print("ret: ");
        Serial.println(ret);
        if (ret == value) {
            Serial.println("Get success!");
        } else {
            Serial.println("Get error!");
        }
    } else {
        Serial.println("Get error!");
    }
}

void test_str(void) {
    String value = "1234";
    if (ezdata.set(value)) {
        Serial.println("Set success!");
    } else {
        Serial.println("Set error!");
    }
    String ret;
    if (ezdata.get(ret)) {
        Serial.print("ret: ");
        Serial.println(ret);
        if (ret == value) {
            Serial.println("Get success!");
        } else {
            Serial.println("Get error!");
        }
    } else {
        Serial.println("Get error!");
    }
}