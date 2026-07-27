#include "Chronometer.h"

#include <format>

DiffusionCurveRenderer::Chronometer::Chronometer(const std::string& Name)
    : mName(Name)
{}

DiffusionCurveRenderer::Chronometer::~Chronometer()
{
    const auto Now = std::chrono::high_resolution_clock::now();
    const auto Duration = std::chrono::duration_cast<std::chrono::microseconds>(Now - mStartTime);

    auto& Stats = sStats[mName];

    Stats.LastCallTime = Duration;
    Stats.TotalCallTime += Duration;
    Stats.NumberOfCalls += 1;

    if (Stats.LongestCallTime < Duration)
    {
        Stats.LongestCallTime = Duration;
    }
}

std::string DiffusionCurveRenderer::Chronometer::Print(const std::string& Name)
{
    const auto& Stats = sStats[Name];

    return std::format("{:<45}: {:<5.3} ms,      {:<5.3} ms,      {:<5.3} sec", //
                       Name,
                       Stats.LastCallTime.count() / 1'000.0f,
                       Stats.LongestCallTime.count() / 1'000.0f,
                       Stats.TotalCallTime.count() / 1'000'000.0f);
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

std::map<std::string, DiffusionCurveRenderer::Stats> DiffusionCurveRenderer::Chronometer::sStats{};