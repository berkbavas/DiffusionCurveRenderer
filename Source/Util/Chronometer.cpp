#include "Chronometer.h"

#include "Logger.h"

#include <format>

DiffusionCurveRenderer::Chronometer::Chronometer(const std::string& name)
    : mName(name)
{
}

DiffusionCurveRenderer::Chronometer::~Chronometer()
{
    std::scoped_lock lock(MUTEX);

    // Update
    auto& stats = STATS_OF_INSTANCES[mName];
    const auto now = std::chrono::system_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - mStartTime);

    stats.callTime = duration;
    stats.lastCallTime = duration;
    stats.totalCallTime += duration;
    stats.numberOfCalls += 1;

    if (stats.longestCallTime < duration)
    {
        stats.longestCallTime = duration;
    }
}

DiffusionCurveRenderer::Stats DiffusionCurveRenderer::Chronometer::QueryAverageStats(const std::string& name)
{
    std::scoped_lock lock(MUTEX);
    Stats& stats = STATS_OF_INSTANCES[name];

    stats.numberOfQueries++;

    if (stats.numberOfCallsAtLastQuery == stats.numberOfCalls)
        stats.callTime = std::chrono::microseconds(0);

    stats.numberOfCallsAtLastQuery = stats.numberOfCalls;

    return stats;
}

std::string DiffusionCurveRenderer::Chronometer::Print(const std::string& name)
{
    const auto stats = QueryAverageStats(name);

    return std::format("{:<40}: {:<5.3} ms,   {:<5.3} ms,   {:<5.3} ms",
                       name,
                       stats.callTime.count() / 1000.0f,
                       stats.lastCallTime.count() / 1000.0f,
                       stats.longestCallTime.count() / 1000.0f);
}

std::mutex DiffusionCurveRenderer::Chronometer::MUTEX = std::mutex();

std::map<std::string, DiffusionCurveRenderer::Stats> DiffusionCurveRenderer::Chronometer::STATS_OF_INSTANCES{};