#pragma once

#include <QtCore/QObject>
#include <QtCore/QFile>

#include <memory>
#include <vector>

class LogHandler : public QObject
{
    Q_OBJECT
public:
    LogHandler();
    ~LogHandler();

signals:
    void messageAdded(const QString &msg);
    void messageAddedPrivate(const QString &msg);

private:
    static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    void appendLogMessage(const QString &msg);

private:
    std::unique_ptr<QFile> _log;
};
