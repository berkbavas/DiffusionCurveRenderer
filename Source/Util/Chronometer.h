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
        uint64_t NumberOfCalls{ 0 };
        uint64_t NumberOfQueries{ 0 };
        uint64_t NumberOfCallsAtLastQuery{ 0 };
        std::chrono::microseconds TotalCallTime{ 0 };
        std::chrono::microseconds CallTime{ 0 };
        std::chrono::microseconds LastCallTime{ 0 };
        std::chrono::microseconds LongestCallTime{ 0 };
    };

    class Chronometer
    {
      public:
        Chronometer(const std::string& Name);
        ~Chronometer();

        static Stats QueryAverageStats(const std::string& Name);
        static std::string Print(const std::string& Name);

      private:
        Clock mStartTime{ std::chrono::system_clock::now() };
        std::string mName;

        static std::mutex MUTEX;
        static std::map<std::string, Stats> STATS_OF_INSTANCES;
    };
}

#define MEASURE_CALL_TIME(NAME) DiffusionCurveRenderer::Chronometer CHRONOMETER__##NAME = DiffusionCurveRenderer::Chronometer(NAME)

#define MEASURE_CALL_TIME_WITH_ARGS(NAME, FORMAT, ...) DiffusionCurveRenderer::Chronometer CHRONOMETER__##NAME = DiffusionCurveRenderer::Chronometer(std::format(FORMAT, __VA_ARGS__))
