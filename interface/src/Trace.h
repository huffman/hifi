#ifndef hifi_Trace_h
#define hifi_Trace_h

#include <QString>
#include <QVariantMap>
#include <QVariantMap>
#include <DependencyManager.h>
#include <cstdint>

enum EventType {
    DurationBegin,
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

    Context
};

struct TraceEvent {
    QString name;
    EventType type;
    int64_t timestamp;
    int64_t processID;
    int64_t threadID;
    QVariantMap args;
};

class Tracer : public Dependency {
public:
    void traceEvent(QString name, EventType type, uint64_t timestamp, int64_t processID, int64_t threadID,
                    QVariantMap args = {});
    void traceEvent(QString name, EventType type, QVariantMap args = {});
    void durationBegin(QString name);
    void durationEnd(QString name);

    void setEnabled(bool enabled) { _enabled = enabled;  }
    void writeToFile(QString path);

private:
    bool _enabled { true };
    std::vector<TraceEvent> _events;
};

namespace profile {

class Duration {
public:
    Duration(QString name) : _name(name) {
        DependencyManager::get<Tracer>()->durationBegin(_name);
    }
    ~Duration() {
        DependencyManager::get<Tracer>()->durationEnd(_name);
    }
private:
    QString _name;
};
    
}


#endif // hifi_Trace_h
