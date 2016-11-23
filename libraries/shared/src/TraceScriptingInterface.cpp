#include "TraceScriptingInterface.h"
#include "Trace.h"

bool TraceScriptingInterface::start(QStringList categoryBlacklist, QStringList categoryWhitelist) {
    std::vector<QString> blacklist;
    std::vector<QString> whitelist;
    for (auto& cat : categoryBlacklist) {
        blacklist.push_back(cat);
    }

    for (auto& cat : categoryWhitelist) {
        whitelist.push_back(cat);
    }

    tracing::Tracer::getInstance()->startTracing(blacklist, whitelist);

    return true;
}

bool TraceScriptingInterface::stop(QString filename) {
    tracing::Tracer::getInstance()->stopTracingAndWriteToFile(filename);

    return true;
}
