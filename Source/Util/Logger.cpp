
#include "Logger.h"

void DiffusionCurveRenderer::Logger::Log(LogLevel level, const std::string& message)
{
    thread_local const auto TL_THREAD_ID = mLastThreadId.fetch_add(1);

    const auto log = std::format("{:.12} [{:}] [ {:5} ] {:}", GetTimeString(), TL_THREAD_ID, GetLogLevelString(level), message);

    switch (level)
    {
        case LogLevel::TRACE:
        case LogLevel::DEBUG:

        case LogLevel::INFO:
            std::cout << log << std::endl;
            break;
        case LogLevel::WARNING:
        case LogLevel::FATAL:
            std::cerr << log << std::endl;
            break;
        default:
            break;
    }
}

void DiffusionCurveRenderer::Logger::SetLogLevel(LogLevel logLevel)
{
    mLogLevel = logLevel;
}

DiffusionCurveRenderer::LogLevel DiffusionCurveRenderer::Logger::GetLogLevel()
{
    return mLogLevel;
}

std::string DiffusionCurveRenderer::Logger::GetTimeString()
{
    const auto zone = std::chrono::current_zone();
    const auto now = std::chrono::system_clock::now();
    const auto local = std::chrono::zoned_time(zone, now);

    return std::format("{:%T}", local);
}

void DiffusionCurveRenderer::Logger::QtMessageOutputCallback(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    switch (type)
    {
        case QtDebugMsg:
            LOG_DEBUG("{}", msg.toStdString());
            break;
        case QtInfoMsg:
            LOG_INFO("{}", msg.toStdString());
            break;
        case QtWarningMsg:
            LOG_WARN("{}", msg.toStdString());
            break;
        case QtCriticalMsg:
            LOG_FATAL("{}", msg.toStdString());
            break;
        case QtFatalMsg:
            LOG_FATAL("{}", msg.toStdString());
            break;
    }
}

bool DiffusionCurveRenderer::Logger::isLogEnabledFor(LogLevel level)
{
    return mLogLevel <= level;
}

DiffusionCurveRenderer::LogLevel DiffusionCurveRenderer::Logger::mLogLevel = DiffusionCurveRenderer::LogLevel::ALL;

std::atomic_uint32_t DiffusionCurveRenderer::Logger::mLastThreadId = 0;