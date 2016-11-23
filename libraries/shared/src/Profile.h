#pragma once

#ifndef HIFI_PROFILE_
#define HIFI_PROFILE_

#include "shared/NsightHelpers.h"
#include "Trace.h"
#include <QCoreApplication>
#include <QThread>

#define CHROME_TRACING

#if defined(NSIGHT_FOUND)
#include "nvToolsExt.h"
#define NSIGHT_TRACING
#endif

#define TRACING_ENABLED 

class Duration {
public:
    Duration(QString category, QString name, uint32_t argbColor = 0xff0000ff, uint64_t payload = 0, QVariantMap args = {})
        : _name(name), _category(category) {
#if defined(CHROME_TRACING)
        args["nv_payload"] = payload;
        tracing::Tracer::getInstance()->traceEvent(_name, tracing::DurationBegin, _category, "", args);
#endif
#if defined(NSIGHT_TRACING)
        nvtxEventAttributes_t eventAttrib {0};
        eventAttrib.version = NVTX_VERSION;
        eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
        eventAttrib.colorType = NVTX_COLOR_ARGB;
        eventAttrib.color = argbColor;
        eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
        eventAttrib.message.ascii = name.toUtf8().data();
        eventAttrib.payload.llValue = payload;
        eventAttrib.payloadType = NVTX_PAYLOAD_TYPE_UNSIGNED_INT64;

        nvtxRangePushEx(&eventAttrib);
#endif
    }
    ~Duration() {
#ifdef CHROME_TRACING
        tracing::Tracer::getInstance()->traceEvent(_name, tracing::DurationEnd, _category);
#endif
#ifdef NSIGHT_TRACING
        nvtxRangePop();
#endif
    }
private:
    QString _name;
    QString _category;
};

inline void asyncBegin(QString name, QString category, QString id, QVariantMap args = {}, QVariantMap extra = {}) {
#ifdef CHROME_TRACING
    tracing::Tracer::getInstance()->traceEvent(name, tracing::AsyncNestableStart, id, category, args, extra);
#endif
}


inline void asyncEnd(QString name, QString category, QString id, QVariantMap args = {}, QVariantMap extra = {}) {
#ifdef CHROME_TRACING
    tracing::Tracer::getInstance()->traceEvent(name, tracing::AsyncNestableEnd, id, category, args, extra);
#endif
}

inline void instant(QString name, QString category, QString scope = "t", QVariantMap args = {}, QVariantMap extra = {}) {
#ifdef CHROME_TRACING
    auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    extra["s"] = scope;
    tracing::Tracer::getInstance()->traceEvent(name, tracing::Instant, "", category, args, extra);
#endif
}

inline void counter(QString name, QString category, QVariantMap args, QVariantMap extra = {}) {
#ifdef CHROME_TRACING
    tracing::Tracer::getInstance()->traceEvent(name, tracing::Counter, "", category, args, extra);
#endif
}


#if defined(CHROME_TRACING) || defined(NSIGHT_TRACING)

    #define PROFILE_RANGE(category, name) Duration profileRangeThis(category, name);
    #define PROFILE_RANGE_EX(category, name, argbColor, payload, ...) Duration profileRangeThis(category, name, argbColor, (uint64_t)payload, ##__VA_ARGS__);

    #define PROFILE_ASYNC_BEGIN(category, name, id, ...) asyncBegin(category, name, id, ##__VA_ARGS__);
    #define PROFILE_ASYNC_END(category, name, id, ...) asyncEnd(category, name, id, ##__VA_ARGS__);

    #define PROFILE_COUNTER(category, name, ...) counter(name, category, ##__VA_ARGS__);

    #define PROFILE_INSTANT(category, name, ...) instant(name, category, ##__VA_ARGS__);

#else

    // EMPTY PROFILE MACROS
    #define PROFILE_RANGE(name)
    #define PROFILE_RANGE_EX(name, argbColor, payload)

    #define PROFILE_ASYNC_BEGIN(category, name, id, ...)
    #define PROFILE_ASYNC_END(category, name, id, ...)

    #define PROFILE_COUNTER(category, name, args, extra)

    #define PROFILE_INSTANT(category, name, args, extra)

#endif


#endif