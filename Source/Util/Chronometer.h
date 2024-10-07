#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <string>

namespace DiffusionCurveRenderer
{
    using Clock = std::chrono::time_point<std::chrono::system_clock>;

    struct Stats
    {
        uint64_t numberOfCalls{ 0 };
        uint64_t numberOfQueries{ 0 };
        uint64_t numberOfCallsAtLastQuery{ 0 };
        std::chrono::microseconds totalCallTime{ 0 };
        std::chrono::microseconds callTime{ 0 };
        std::chrono::microseconds lastCallTime{ 0 };
        std::chrono::microseconds longestCallTime{ 0 };
    };

    class Chronometer
    {
      public:
        Chronometer(const std::string& name);
        ~Chronometer();

        static Stats QueryAverageStats(const std::string& name);
        static std::string Print(const std::string& name);

      private:
        Clock mStartTime{ std::chrono::system_clock::now() };
        std::string mName;

        static std::mutex MUTEX;
        static std::map<std::string, Stats> STATS_OF_INSTANCES;
    };
}

#define MEASURE_CALL_TIME(NAME) \
    DiffusionCurveRenderer::Chronometer CHORONOMETER__##NAME = DiffusionCurveRenderer::Chronometer(NAME)

#define MEASURE_CALL_TIME_WITH_ARGS(NAME, FORMAT, ...) \
    DiffusionCurveRenderer::Chronometer CHORONOMETER__##NAME = DiffusionCurveRenderer::Chronometer(std::format(FORMAT, __VA_ARGS__))
