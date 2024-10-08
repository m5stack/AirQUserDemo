#pragma once

#include <WString.h>


class DataBase {
public:
    DataBase() {}
    ~DataBase() {}

    void saveToFile();
    void dump();
    void loadFromFile();

public:
    bool factoryState;
    struct {
        String ssid;
        String password;
    } wifi;
    struct {
        int sleepInterval;
    } rtc;
    struct {
        String ntpServer0;
        String ntpServer1;
        String tz;
    } ntp;
    struct {
        String devToken;
        String loginName;
        String password;
    } ezdata2;
    struct {
        bool onoff;
    } buzzer;

    String nickname;

    bool isFactoryTestMode = false;

    // 不需要保存到文件
    bool isConfigState;
    bool pskStatus = true;
};


extern DataBase db;
