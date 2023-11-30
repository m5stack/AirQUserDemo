#pragma once

#include <M5Unified.h>
// #include <lgfx/v1/panel/Panel_GDEW0154M09.hpp>

class StatusView
{
public:
    StatusView();
    StatusView(LGFX_Device *lcd, M5Canvas *canvas);
    ~StatusView();

    void begin();
    void load();
    void updateTime(const char *time, const char *date);
    void updateSCD40(uint16_t co2, float temperature, float humidity);
    void updatePower(uint32_t voltage);
    void updateCountdown(uint32_t seconds);
    void displayCountdown(uint32_t seconds);
    void updateSEN55(
        float massConcentrationPm1p0,
        float massConcentrationPm2p5,
        float massConcentrationPm4p0,
        float massConcentrationPm10p0,
        float ambientHumidity,
        float ambientTemperature,
        float vocIndex,
        float noxIndex
    );
    void updateNetworkStatus(const char *title, const char *msg);
    void displayNetworkStatus(const char *title, const char *msg);
    void updateNickname(String &nickname);
    void displayNickname(String &nickname);
    void disappear();

private:
    LovyanGFX *_lcd;
    M5Canvas *_canvas;

    // time

    const lgfx::IFont* _timeTimeFont = &fonts::FreeSansBold18pt7b;
    const lgfx::IFont* _timeDateFont = &fonts::DejaVu12;

    int32_t _timeBaseCursorX = 2;
    int32_t _timeBaseCursorY = 2;

    M5Canvas *_timeCanvas;
    int32_t _timeCanvasX;
    int32_t _timeCanvasY;
    M5Canvas *_dateCanvas;
    int32_t _dateCanvasX;
    int32_t _dateCanvasY;

    // scd40
    const lgfx::IFont* _scd40TitleFont = &fonts::FreeSansBold9pt7b;
    const lgfx::IFont* _scd40OptionFont = &fonts::DejaVu12;

    int32_t _scd40BaseCursorX = 2;
    int32_t _scd40BaseCursorY = 2 + 50 + 2;

    M5Canvas *_co2Canvas;
    int32_t _co2CanvasX;
    int32_t _co2CanvasY;

    M5Canvas *_tempCanvas;
    int32_t _tempCanvasX;
    int32_t _tempCanvasY;

    M5Canvas *_humiCanvas;
    int32_t _humiCanvasX;
    int32_t _humiCanvasY;

    // power
    const lgfx::IFont* _powerTitleFont = &fonts::FreeSansBold9pt7b;
    const lgfx::IFont* _poweroptionFont = &fonts::DejaVu12;

    int32_t _powerBaseCursorX = 2;
    int32_t _powerBaseCursorY = 2 + 50 + 2 + 69 + 2;

    M5Canvas *_voltageCanvas;
    int32_t _voltageCanvasX;
    int32_t _voltageCanvasY;
    M5Canvas *_chartCanvas;
    M5Canvas *_chartCanvas1;
    int32_t _chartCanvasX;
    int32_t _chartCanvasY;

    // nickname
    M5Canvas *_nicknameCanvas;
    M5Canvas *_nicknameCanvas1;
    int32_t _nicknameCanvasX;
    int32_t _nicknameCanvasY;

    // sen55
    const lgfx::IFont* _sen55TitleFont = &fonts::FreeSansBold9pt7b;
    const lgfx::IFont* _sen55OptionFont = &fonts::DejaVu12;

    int32_t _sen55BaseCursorX = 2 + 97 + 2;
    int32_t _sen55BaseCursorY = 2;

    M5Canvas *_pm1p0Canvas;
    int32_t _pm1p0CanvasX;
    int32_t _pm1p0CanvasY;

    M5Canvas *_pm2p5Canvas;
    int32_t _pm2p5CanvasX;
    int32_t _pm2p5CanvasY;

    M5Canvas *_pm4p0Canvas;
    int32_t _pm4p0CanvasX;
    int32_t _pm4p0CanvasY;

    M5Canvas *_pm10p0Canvas;
    int32_t _pm10p0CanvasX;
    int32_t _pm10p0CanvasY;

    M5Canvas *_sen55TempCanvas;
    int32_t _sen55TempCanvasX;
    int32_t _sen55TempCanvasY;

    M5Canvas *_sen55HumiCanvas;
    int32_t _sen55HumiCanvasX;
    int32_t _sen55HumiCanvasY;

    M5Canvas *_vocCanvas;
    int32_t _vocCanvasX;
    int32_t _vocCanvasY;

    M5Canvas *_noxCanvas;
    int32_t _noxCanvasX;
    int32_t _noxCanvasY;

    // status
    const lgfx::IFont* _statusTitleFont = &fonts::DejaVu18;
    const lgfx::IFont* _statusMsgFont = &fonts::DejaVu18;

    int32_t _statusBaseCursorX = 2 + 97 + 2;
    int32_t _statusBaseCursorY = 2 + 144 + 2;

    M5Canvas *_stautsTitleCanvas;
    M5Canvas *_stautsTitleCanvas1;
    int32_t _statusTitleCanvasX;
    int32_t _statusTitleCanvasY;
    M5Canvas *_stautsMsgCanvas;
    M5Canvas *_stautsMsgCanvas1;
    int32_t _statusMsgCanvasX;
    int32_t _statusMsgCanvasY;

    int32_t _margin = 2;
    int32_t _border = 1;
    int32_t _padding = 2;

    void _updateImpl(M5Canvas *canvas, int32_t x, int32_t y);
    void initTime();
    void initSCD40();
    void initPower();
    void initLOGO();
    void initSEN55();
    void initStatus();
    void splitLongString(String &text, int32_t maxWidth, const lgfx::IFont* font);
    void setNicknameFont(String &text, int32_t maxWidth);
};