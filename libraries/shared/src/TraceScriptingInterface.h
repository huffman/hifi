#pragma once

class TraceScriptingInterface : QObject {
    Q_OBJECT
public:
    TraceScriptingInterface();

    bool start(QStringList blacklistCategories, QStringList whitelistCategories);
    bool stop(QString filename);
}
