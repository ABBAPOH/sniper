#pragma once

#include <QtCore/QObject>

#include <vector>

class LogHandler : public QObject
{
    Q_OBJECT
public:
    LogHandler();
    ~LogHandler();

    const std::vector<QString> &log() const { return _log; }

    static LogHandler *instance();

signals:
    void messageAdded(const QString &msg);
    void messageAddedPrivate(const QString &msg);

private:
    static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    void appendLogMessage(const QString &msg);

private:
    std::vector<QString> _log;
};
