
#include "Logger.h"

void DiffusionCurveRenderer::Logger::Log(LogLevel LogLevel, const std::string& Message)
{
    thread_local const auto TL_THREAD_ID = mLastThreadId.fetch_add(1);

    const auto Log = std::format("{:.12} [{:}] [ {:5} ] {:}", GetTimeString(), TL_THREAD_ID, GetLogLevelString(LogLevel), Message);

    switch (LogLevel)
    {
    case LogLevel::TRACE:
    case LogLevel::DEBUG:

    case LogLevel::INFO:
        std::cout << Log << std::endl;
        break;
    case LogLevel::WARNING:
    case LogLevel::FATAL:
        std::cerr << Log << std::endl;
        break;
    default:
        break;
    }
}

void DiffusionCurveRenderer::Logger::SetLogLevel(LogLevel LogLevel)
{
    mLogLevel = LogLevel;
}

DiffusionCurveRenderer::LogLevel DiffusionCurveRenderer::Logger::GetLogLevel()
{
    return mLogLevel;
}

std::string DiffusionCurveRenderer::Logger::GetTimeString()
{
    const auto Zone = std::chrono::current_zone();
    const auto Now = std::chrono::system_clock::now();
    const auto Local = std::chrono::zoned_time(Zone, Now);

    return std::format("{:%T}", Local);
}

void DiffusionCurveRenderer::Logger::QtMessageOutputCallback(QtMsgType Type, const QMessageLogContext& Context, const QString& Message)
{
    switch (Type)
    {
    case QtDebugMsg:
        LOG_DEBUG("{}", Message.toStdString());
        break;
    case QtInfoMsg:
        LOG_INFO("{}", Message.toStdString());
        break;
    case QtWarningMsg:
        LOG_WARN("{}", Message.toStdString());
        break;
    case QtCriticalMsg:
        LOG_FATAL("{}", Message.toStdString());
        break;
    case QtFatalMsg:
        LOG_FATAL("{}", Message.toStdString());
        break;
    }
}

std::string DiffusionCurveRenderer::Logger::GetLogLevelString(LogLevel LogLevel)
{
    switch (LogLevel)
    {
    case LogLevel::ALL:
        return "ALL";
    case LogLevel::NONE:
        return "NONE";
    case LogLevel::TRACE:
        return "TRACE";
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARNING:
        return "WARN";
    case LogLevel::FATAL:
        return "FATAL";
    default:
        return "N/A";
    }
}

bool DiffusionCurveRenderer::Logger::IsLogEnabledFor(LogLevel LogLevel)
{
    return mLogLevel <= LogLevel;
}

DiffusionCurveRenderer::LogLevel DiffusionCurveRenderer::Logger::mLogLevel = DiffusionCurveRenderer::LogLevel::ALL;

std::atomic_uint32_t DiffusionCurveRenderer::Logger::mLastThreadId = 0;