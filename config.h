#pragma once

#define USER_BTN_A 0
#define USER_BTN_B 8

#define POWER_HOLD 46
#define BUZZER_PIN 9

#define GROVE_SDA 13
#define GROVE_SCL 15

#define I2C1_SDA_PIN 11
#define I2C1_SCL_PIN 12

#define BAT_ADC_PIN 14

#define SEN55_POWER_EN 10

#define USER_BUTTON_POWER 42

#define EPD_MOSI 6
#define EPD_MISO -1
#define EPD_SCLK 5
#define EPD_DC   3
#define EPD_FREQ 40000000

#define EPD_CS   4
#define EPD_RST  2
#define EPD_BUSY 1

#define APP_VERSION "0.1.0"

#define FORMAT_FILESYSTEM false
#define FILESYSTEM LittleFS

/* Number of retries for ezdata failed data upload
 *
 * ezdata上传数据失败的重试次数
 */
#define EZDATA_UPLOAD_RETRY_COUNT 3

/* AirQ shutdown timeout in wake state, unit is seconds
 *
 * AirQ在唤醒状态下的关机超时时间，单位是秒
 */
#define AIRQ_SHUTDOWN_TIMEOUT 10

/* WiFi connection timeout, unit is seconds
 *
 * WiFi连接超时时间，单位是秒
 */
#define WIFI_CONNECT_TIMEOUT 10
