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
};


extern DataBase db;
