#ifndef hifi_Trace_h
#define hifi_Trace_h

#include <QString>
#include <QVariantMap>
#include <QVariantMap>
#include <cstdint>

#include "DependencyManager.h"
#include <mutex>

#define TRACE_ENABLED

namespace tracing {

using TraceTimestamp = uint64_t;

enum EventType : char {
    DurationBegin = 'B',
    DurationEnd = 'E',

    Complete = 'X',
    Instant = 'i',
    Counter = 'C',

    AsyncNestableStart = 'b',
    AsyncNestableInstant = 'n',
    AsyncNestableEnd = 'e',

    FlowStart = 's',
    FlowStep = 't',
    FlowEnd = 'f',

    Sample = 'P',

    ObjectCreated = 'N',
    ObjectSnapshot = 'O',
    ObjectDestroyed = 'D',

    Metadata = 'M',

    MemoryDumpGlobal = 'V',
    MemoryDumpProcess = 'v',

    Mark = 'R',

    ClockSync = 'c',

    ContextEnter = '(',
    ContextLeave = ')'
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

    void traceEvent(QString name, EventType type, QString category = "", QString id = "", QVariantMap args = {}, QVariantMap extra = {});

    void startTracing(std::vector<QString> categoryBlacklist, std::vector<QString> categoryWhitelist);
    void stopTracingAndWriteToFile(QString path);

private:
    void traceEvent(QString name, EventType type, uint64_t timestamp, int64_t processID, int64_t threadID, QString id = "",
        QString category = "", QVariantMap args = {}, QVariantMap extra = {});

    bool _enabled { true };
    std::vector<TraceEvent> _events;
    std::mutex _eventsMutex;
    std::vector<QString> _categoryBlacklist;
    std::vector<QString> _categoryWhitelist;
};

}

#endif // hifi_Trace_h
