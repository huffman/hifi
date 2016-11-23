#pragma once

#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <mutex>
#include "DependencyManager.h"
#include "Trace.h"

using EditStatFunction = std::function<QVariant(QVariant currentValue)>;

class StatTracker : public Dependency {
public:
    StatTracker();
    QVariant getStat(QString name);
    void editStat(QString name, EditStatFunction fn);
    void incrementStat(QString name);
    void decrementStat(QString name);
private:
    std::mutex _statsLock;
    QVariantMap _stats;
};

class CounterStat {
public:
    CounterStat(QString name) : _name(name) {
        DependencyManager::get<StatTracker>()->incrementStat(_name);
        //trace::COUNTER("processing", "stats", {
            //{ "active", DependencyManager::get<StatTracker>()->getStat("ResourceProcessing").toInt() }
        //});
    }    
    ~CounterStat() {
        DependencyManager::get<StatTracker>()->decrementStat(_name);
    }    
private:
    QString _name;
};