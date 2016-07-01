#ifndef hifi_Trace_h
#define hifi_Trace_h

#include <QString>
#include <QVariantMap>
#include <QVariantMap>
#include <cstdint>

#include "DependencyManager.h"
#include <mutex>

#define TRACE_ENABLED

enum EventType {
    DurationBegin = 0,
    DurationEnd,

    Complete,
    Instant,
    Counter,

    AsyncNestableStart,
    AsyncNestableInstant,
    AsyncNestableEnd,

    FlowStart,
    FlowStep,
    FlowEnd,

    Sample,

    ObjectCreated,
    ObjectSnapshot,
    ObjectDestroyed,

    Metadata,

    MemoryDumpGlobal,
    MemoryDumpProcess,

    Mark,

    ClockSync,

    ContextEnter,
    ContextLeave
};

struct TraceEvent {
    QString id;
    QString name;
    EventType type;
    int64_t timestamp;
    int64_t processID;
    int64_t threadID;
    QString category;
    QVariantMap args;
    QVariantMap extra;
};

class Tracer {
public:
    static Tracer* getInstance() {
        static Tracer tracer;
        return &tracer;
    };
    void traceEvent(QString name, EventType type, uint64_t timestamp, int64_t processID, int64_t threadID, QString id = "",
                    QString category = "", QVariantMap args = {}, QVariantMap extra = {});
    void traceEvent(QString name, EventType type, QString category = "", QString id = "", QVariantMap args = {}, QVariantMap extra = {});
    void durationBegin(QString name);
    void durationEnd(QString name);

    void setEnabled(bool enabled) { _enabled = enabled;  }
    void writeToFile(QString path);

private:
    bool _enabled { true };
    std::vector<TraceEvent> _events;
    std::mutex _eventsMutex;
};

namespace trace {

    enum Category {
        Resource,
        AssetRequest
    };

    void traceEvent(QString name, EventType type, QString category = "", QVariantMap args = {}, QVariantMap extra = {});
    void traceEvent(QString name, EventType type, QString id, QString category = "", QVariantMap args = {}, QVariantMap extra = {});

    void ASYNC_BEGIN(QString name, QString category, QString id, QVariantMap args = {});
    void ASYNC_BEGIN(QString name, QString category, int id, QVariantMap args = {});
    void ASYNC_END(QString name, QString category, QString id, QVariantMap args = {});
    void ASYNC_END(QString name, QString category, int id, QVariantMap args = {});
    void ASYNC_INSTANT(QString name, QString category, QString id, QVariantMap args = {});

    void FLOW_BEGIN(QString name, QString category, QString id, QVariantMap args = {});
    void FLOW_END(QString name, QString category, QString id, QVariantMap args = {});

    void DURATION_BEGIN(QString name, QString category, QVariantMap args = {});
    void DURATION_END(QString name, QString category, QVariantMap args = {});

    inline void COUNTER(QString name, QString category, QVariantMap args, QVariantMap extra = {}) {
#ifdef TRACE_ENABLED
        trace::traceEvent(name, Counter, category, args, extra);
#endif
    }

    inline void INSTANT(QString name, QString scope = "t", QVariantMap args = {}, QVariantMap extra = {}) {
#ifdef TRACE_ENABLED
        extra["s"] = scope;
        trace::traceEvent(name, Instant, "", args, extra);
#endif
    }

    class Duration {
    public:
        Duration(QString name, QString category = "", QVariantMap args = {})
            : _name(name), _category(category) {
            Tracer::getInstance()->traceEvent(_name, DurationBegin, _category, "" ,args);
        }
        ~Duration() {
            Tracer::getInstance()->traceEvent(_name, DurationEnd, _category);
        }
    private:
        QString _name;
        QString _category;
    };

    static const QString nameAssetRequest = "AssetRequest";
    static const QString nameAssetData = "AssetData";
    static const QString nameAssetInfo = "AssetInfo";
    static const QString nameAssetEndedEarly = "AssetEndedEarly";

    static const QString cResource = "Resource";
    static const QString cDomainLoading = "DomainLoading";
}


#endif // hifi_Trace_h
