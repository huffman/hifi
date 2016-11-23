#include "StatTracker.h"

StatTracker::StatTracker() {
    
}

QVariant StatTracker::getStat(QString name) {
    std::lock_guard<std::mutex> lock(_statsLock);
    return _stats[name];
}

void StatTracker::editStat(QString name, EditStatFunction fn) {
    std::lock_guard<std::mutex> lock(_statsLock);
    _stats[name] = fn(_stats[name]);
}

void StatTracker::incrementStat(QString name) {
    std::lock_guard<std::mutex> lock(_statsLock);
    QVariant stat = _stats[name];
    _stats[name] = _stats[name].toInt() + 1;
}

void StatTracker::decrementStat(QString name) {
    std::lock_guard<std::mutex> lock(_statsLock);
    _stats[name] = _stats[name].toInt() - 1;
}