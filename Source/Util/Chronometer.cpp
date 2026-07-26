#include "Chronometer.h"

#include "Logger.h"

#include <format>

DiffusionCurveRenderer::Chronometer::Chronometer(const std::string& Name)
    : mName(Name)
{}

DiffusionCurveRenderer::Chronometer::~Chronometer()
{
    std::scoped_lock Lock(sMutex);

    // Update
    auto& Stats = sStats[mName];
    const auto Now = std::chrono::system_clock::now();
    const auto Duration = std::chrono::duration_cast<std::chrono::microseconds>(Now - mStartTime);

    Stats.CallTime = Duration;
    Stats.LastCallTime = Duration;
    Stats.TotalCallTime += Duration;
    Stats.NumberOfCalls += 1;

    if (Stats.LongestCallTime < Duration)
    {
        Stats.LongestCallTime = Duration;
    }
}

DiffusionCurveRenderer::Stats DiffusionCurveRenderer::Chronometer::QueryAverageStats(const std::string& Name)
{
    std::scoped_lock Lock(sMutex);
    Stats& Stats = sStats[Name];

    Stats.NumberOfQueries++;

    if (Stats.NumberOfCallsAtLastQuery == Stats.NumberOfCalls)
    {
        Stats.CallTime = std::chrono::microseconds(0);
    }

    Stats.NumberOfCallsAtLastQuery = Stats.NumberOfCalls;

    return Stats;
}

std::string DiffusionCurveRenderer::Chronometer::Print(const std::string& Name)
{
    const auto& Stats = QueryAverageStats(Name);

    return std::format("{:<40}: {:<5.3} ms,   {:<5.3} ms,   {:<5.3} ms", //
                       Name,
                       Stats.CallTime.count() / 1000.0f,
                       Stats.LastCallTime.count() / 1000.0f,
                       Stats.LongestCallTime.count() / 1000.0f);
}

std::string DiffusionCurveRenderer::Chronometer::PrintAll()
{
    std::string Result;

    for (const auto& [Name, Stats] : sStats)
    {
        Result += Print(Name) + "\n";
    }

    return Result;
}

std::mutex DiffusionCurveRenderer::Chronometer::sMutex{};

std::map<std::string, DiffusionCurveRenderer::Stats> DiffusionCurveRenderer::Chronometer::sStats{};