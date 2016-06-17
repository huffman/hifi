#include "Trace.h"

#include <QCoreApplication>
#include <QThread>
#include <SharedUtil.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QFile>

QString eventTypeToString(EventType type) {
    switch (type) {
        case DurationBegin:
            return "B";
        case DurationEnd:
            return "E";
    };
    return "invalid";
}

void Tracer::writeToFile(QString path) {
    if (_enabled) {
        return;
    }

    //open file

    QJsonArray traceEvents;
    for (const auto& event : _events) {
        QJsonObject ev {
            { "name", event.name },
            { "cat", "" },
            { "ph", eventTypeToString(event.type) },
            { "ts", event.timestamp },
            { "pid", event.processID },
            { "tid", event.threadID },
            { "args", QJsonObject::fromVariantMap(event.args) }
        };
        traceEvents.append(ev);
    }


    QJsonObject root {
        { "traceEvents", traceEvents }
    };
    QJsonDocument document { root };

    QFile file("F:///trace.json");
    if (file.open(QIODevice::WriteOnly)) {
        auto data = document.toJson(QJsonDocument::Compact);
        file.write(data);
    }
}

void Tracer::traceEvent(QString name, EventType type, uint64_t timestamp, int64_t processID, int64_t threadID,
                QVariantMap args) {
    if (!_enabled) {
        return;
    }

    TraceEvent event {
        name,
        type,
        timestamp,
        processID,
        threadID,
        args
    };
    _events.push_back(event);
}

void Tracer::traceEvent(QString name, EventType type, QVariantMap args) {
    auto timestamp = usecTimestampNow();
    traceEvent(name, type, timestamp, QCoreApplication::applicationPid(), int64_t(QThread::currentThreadId()));
}

void Tracer::durationBegin(QString name) {
    traceEvent(name, DurationBegin);
}

void Tracer::durationEnd(QString name) {
    traceEvent(name, DurationEnd);
}
