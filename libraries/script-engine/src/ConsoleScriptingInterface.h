#pragma once

#include <QtCore/QObject>
#include <QObject>
#include <QDebug>
#include <QString>
#include <QScriptEngine>

class ConsoleScriptingInterface : public QObject {
    Q_OBJECT
public:
    ConsoleScriptingInterface(QScriptEngine* engine) : QObject(engine), _engine(engine) { }
    Q_INVOKABLE void log(...) {

        print("");

        //emit log(message);
    }

signals:
    void log(QString message);

private:
    void print(QString prefix) {
        auto engine = dynamic_cast<QScriptEngine*>(sender());
        if (!engine) {
            qDebug() << "Error, received log message from non ScriptEngine sender";
            return;
        }

        auto context = engine->currentContext();

        QString message = prefix;
        for (int i = 0; i < context->argumentCount(); i++) {
            if (i > 0) {
                message += " ";
            }
            message += context->argument(i).toString();
        }
        qDebug().noquote() << "script:print()<<" << message;  // noquote() so that \n is treated as newline
    }

    QScriptEngine* _engine;
};
