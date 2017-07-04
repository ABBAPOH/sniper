#include "loghandler.h"

static LogHandler *_instance = nullptr;
static QtMessageHandler qtMessageHandler = nullptr;

LogHandler::LogHandler()
{
    _instance = this;
    connect(this, &LogHandler::messageAddedPrivate,
            this, &LogHandler::appendLogMessage);
    qtMessageHandler = qInstallMessageHandler(messageOutput);
}

LogHandler::~LogHandler()
{
    _instance = nullptr;
}

LogHandler *LogHandler::instance()
{
    return _instance;
}

QString typeToString(QtMsgType type)
{
    switch (type) {
    case QtMsgType::QtDebugMsg: return QStringLiteral("debug");
    case QtMsgType::QtWarningMsg: return QStringLiteral("warn");
    case QtMsgType::QtCriticalMsg: return QStringLiteral("crit");
    case QtMsgType::QtFatalMsg: return QStringLiteral("fatal");
    case QtMsgType::QtInfoMsg: return QStringLiteral("info");

        break;
    default:
        break;
    }
    return QString();
}

void LogHandler::messageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    qtMessageHandler(type, context, msg);

    const auto message = (context.category == QByteArrayLiteral("default"))
            ? tr("%1: %2").arg(typeToString(type), -5, ' ').arg(msg)
            : tr("%1 %2: %3").arg(typeToString(type), -5, ' ').arg(context.category).arg(msg);
    emit _instance->messageAddedPrivate(message);
}

void LogHandler::appendLogMessage(const QString& msg)
{
    _log.push_back(msg);
    emit messageAdded(msg);
}
