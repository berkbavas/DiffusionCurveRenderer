#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <string>

namespace DiffusionCurveRenderer
{
    using Clock = std::chrono::time_point<std::chrono::high_resolution_clock>;

    struct Stats
    {
        uint64_t NumberOfCalls{ 0 };
        std::chrono::microseconds TotalCallTime{ 0 };
        std::chrono::microseconds LastCallTime{ 0 };
        std::chrono::microseconds LongestCallTime{ 0 };
    };

    class Chronometer
    {
      public:
        Chronometer(const std::string& Name);
        Chronometer(const Chronometer&) = delete;
        Chronometer& operator=(const Chronometer&) = delete;
        ~Chronometer();

        static std::string Print(const std::string& Name);
        static std::string PrintAll();

      private:
        Clock mStartTime{ std::chrono::high_resolution_clock::now() };
        std::string mName;

        static std::map<std::string, Stats> sStats;
    };
}
