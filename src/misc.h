#pragma once

#define BUTTON_TONE() {       \
    ledcWriteTone(0, 3500); \
    delay(50);              \
    ledcWriteTone(0, 0);    \
}


#define SUCCESS_TONE() {       \
    ledcWriteTone(0, 7000); \
    delay(50);              \
    ledcWriteTone(0, 0);    \
}


#define FAIL_TONE() {        \
    ledcWriteTone(0, 4000); \
    delay(100);             \
    ledcWriteTone(0, 0);    \
}
