#pragma once

#include "config.h"

#if EZDATA_HTTP_DEBUG

#include <Arduino.h>
#include <WString.h>

namespace EzDataLog {

inline void parseUrl(const String &url, String &host, String &path) {
    String hostPath = url;
    if (hostPath.startsWith("http://")) {
        hostPath = hostPath.substring(7);
    } else if (hostPath.startsWith("https://")) {
        hostPath = hostPath.substring(8);
    }
    int slashIdx = hostPath.indexOf('/');
    host = slashIdx >= 0 ? hostPath.substring(0, slashIdx) : hostPath;
    path = slashIdx >= 0 ? hostPath.substring(slashIdx) : "/";
}

inline void logRequest(const char *method, const String &url,
                       const char *contentType = nullptr,
                       const uint8_t *body = nullptr, size_t bodyLen = 0) {
    String host, path;
    parseUrl(url, host, path);

    log_i("[EzData] --> %s %s HTTP/1.1", method, path.c_str());
    log_i("[EzData]     Host: %s", host.c_str());
    if (contentType != nullptr && contentType[0] != '\0') {
        log_i("[EzData]     Content-Type: %s", contentType);
        log_i("[EzData]     Content-Length: %u", (unsigned)bodyLen);
    }
    if (body != nullptr && bodyLen > 0) {
        log_i("[EzData]     %.*s", (int)bodyLen, (const char *)body);
    }
}

inline void logResponse(const char *body) {
    if (body == nullptr) {
        return;
    }
    log_i("[EzData] <-- %s", body);
}

inline void logResponse(const String &body) {
    logResponse(body.c_str());
}

} // namespace EzDataLog

#define EZDATA_LOG_HTTP_REQUEST(method, url, ...) \
    EzDataLog::logRequest(method, url, ##__VA_ARGS__)

#define EZDATA_LOG_HTTP_RESPONSE(body) \
    EzDataLog::logResponse(body)

#else

#define EZDATA_LOG_HTTP_REQUEST(method, url, ...) ((void)0)
#define EZDATA_LOG_HTTP_RESPONSE(body)            ((void)0)

#endif
