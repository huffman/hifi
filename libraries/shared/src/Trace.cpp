#include "Trace.h"

#include <QCoreApplication>
#include <QThread>
#include "SharedUtil.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QFile>

static const std::map<EventType, QString> eventTypeToStringMap {
    { DurationBegin, "B" },
    { DurationEnd, "E" },

    { Complete, "X" },
    { Instant, "i" },
    { Counter, "C" },

    { AsyncNestableStart, "b" },
    { AsyncNestableInstant, "n" },
    { AsyncNestableEnd, "e" },

    { FlowStart, "s" },
    { FlowStep, "t" },
    { FlowEnd, "f" },

    { Sample, "P" },

    { ObjectCreated, "N" },
    { ObjectSnapshot, "O" },
    { ObjectDestroyed, "D" },

    { Metadata, "M" },

    { MemoryDumpGlobal, "V" },
    { MemoryDumpProcess, "v" },

    { Mark, "R" },

    { ClockSync, "c" },

    { ContextEnter, "(" },
    { ContextLeave, ")" }
};

QString eventTypeToString(EventType type) {
    auto it = eventTypeToStringMap.find(type);
    if (it != eventTypeToStringMap.end()) {
        return it->second;
    }
    return "not implemented";
}

void Tracer::writeToFile(QString path) {
    if (_enabled) {
        return;
    }

    std::lock_guard<std::mutex> guard(_eventsMutex);
    QJsonArray traceEvents;
    for (const auto& event : _events) {
        QJsonObject ev {
            { "name", event.name },
            { "cat", event.category },
            { "ph", eventTypeToString(event.type) },
            { "ts", event.timestamp },
            { "pid", event.processID },
            { "tid", event.threadID },
            { "args", QJsonObject::fromVariantMap(event.args) }
        };
        auto it = event.extra.begin();
        for (; it != event.extra.end(); it++) {
            ev[it.key()] = QJsonValue::fromVariant(it.value());
        }
        if (!event.id.isEmpty()) {
            ev["id"] = event.id;
        }
        traceEvents.append(ev);
    }


    QJsonObject root {
        { "traceEvents", traceEvents }
    };
    QJsonDocument document { root };

//    QFile file("F:///trace.json");
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        auto data = document.toJson(QJsonDocument::Compact);
        file.write(data);
    }
}

void Tracer::traceEvent(QString name, EventType type, uint64_t timestamp, int64_t processID, int64_t threadID, QString id,
                QString category, QVariantMap args, QVariantMap extra) {
    if (!_enabled) {
        return;
    }

    static const QStringList categoryBlacklist = {
        "assetRequest"
    };

    if (categoryBlacklist.contains(category)) {
        return;
    }

    TraceEvent event {
        id,
        name,
        type,
        timestamp,
        processID,
        threadID,
        category,
        args,
        extra
    };

    std::lock_guard<std::mutex> guard(_eventsMutex);
    _events.push_back(event);
}

void Tracer::traceEvent(QString name, EventType type, QString category, QString id, QVariantMap args, QVariantMap extra) {
    auto timestamp = usecTimestampNow();
    traceEvent(name, type, timestamp, QCoreApplication::applicationPid(), int64_t(QThread::currentThreadId()), id, category, args, extra);
}

void trace::traceEvent(QString name, EventType type, QString category, QVariantMap args, QVariantMap extra) {
    Tracer::getInstance()->traceEvent(name, type, category, "-1", args, extra);
}
void trace::traceEvent(QString name, EventType type, QString id, QString category, QVariantMap args, QVariantMap extra) {
    Tracer::getInstance()->traceEvent(name, type, category, id, args, extra);
}

void trace::ASYNC_BEGIN(QString name, QString category, QString id, QVariantMap args) {
    trace::traceEvent(name, AsyncNestableStart, id, category, args);
}
void trace::ASYNC_BEGIN(QString name, QString category, int id, QVariantMap args) {
    trace::traceEvent(name, AsyncNestableStart, QString::number(id), category, args);
}
void trace::ASYNC_END(QString name, QString category, QString id, QVariantMap args) {
    trace::traceEvent(name, AsyncNestableEnd, id, category, args);
}
void trace::ASYNC_END(QString name, QString category, int id, QVariantMap args) {
    trace::traceEvent(name, AsyncNestableEnd, QString::number(id), category, args);
}
void trace::ASYNC_INSTANT(QString name, QString category, QString id, QVariantMap args) {
    trace::traceEvent(name, AsyncNestableInstant, id, category, args);
}


void trace::FLOW_BEGIN(QString name, QString category, QString id, QVariantMap args) {
    trace::traceEvent(name, FlowStart, id, category, args);
}
void trace::FLOW_END(QString name, QString category, QString id, QVariantMap args) {
    trace::traceEvent(name, FlowEnd, id, category, args);
}

void trace::DURATION_BEGIN(QString name, QString category, QVariantMap args) {
    trace::traceEvent(name, DurationBegin, "", category, args);
}
void trace::DURATION_END(QString name, QString category, QVariantMap args) {
    trace::traceEvent(name, DurationEnd, "", category, args);
}

void Tracer::durationBegin(QString name) {
    traceEvent(name, DurationBegin);
}

void Tracer::durationEnd(QString name) {
    traceEvent(name, DurationEnd);
}
