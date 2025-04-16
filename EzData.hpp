#ifndef _EZDATA_H_
#define _EZDATA_H_

#include <WString.h>
#include <type_traits>


class EzData
{
public:
    EzData(const char *dev_token, const char *key);

    EzData(const String dev_token, const char *key);

    EzData(const String dev_token, const String key);

    EzData(const char *dev_token, const String key);

    EzData(const char *data_token);

    EzData(const String data_token);

    ~EzData();

    void setDeviceToken(const String &dev_token);
    void setServer(const String &server);

    bool set(int value);

    bool set(const float value);

    bool set(const double value);

    bool set(const String value);

    bool set(const char *value, size_t len);

    bool _set(uint8_t *payload, size_t size);

    template <typename T>
    bool _get(T &retValue);

    template <typename T>
    bool get(T &retValue);

    bool del();

private:
    String _server = "ezdata2.m5stack.com";
    bool _public = false;
    String _device_token = "";
    String _key = "";
};

bool registeredDevice(const String &mac, String &loginName, String &password, String &devToken);

bool login(const String &loginName, const String &password, String &deviceToken);


#endif
