#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class TraceScriptingInterface : QObject {
    Q_OBJECT
public:
    bool start(QStringList categoryBlacklist, QStringList categoryWhitelist);
    bool stop(QString filename);

//private:
//    QStringList _categoryBlacklist;
//    QStringList _categoryWhitelist;
};