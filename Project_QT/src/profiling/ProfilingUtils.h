#ifndef RME_PROFILING_UTILS_H
#define RME_PROFILING_UTILS_H

#include <QElapsedTimer>
#include <QString>
#include <QDebug>
#include <QApplication>
#include <chrono>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

namespace RME {
namespace profiling {

/**
 * @brief High-precision timer for performance measurements
 */
class PerformanceTimer {
public:
    PerformanceTimer(const QString& name = "Timer");
    ~PerformanceTimer();
    
    void start();
    void stop();
    void reset();
    
    qint64 elapsedNanoseconds() const;
    double elapsedMilliseconds() const;
    double elapsedSeconds() const;
    
    void printResults() const;

private:
    QString m_name;
    QElapsedTimer m_timer;
    qint64 m_totalElapsed;
    bool m_isRunning;
};

/**
 * @brief Memory usage tracker for profiling
 */
class MemoryTracker {
public:
    struct MemoryInfo {
        size_t workingSetSize = 0;      // Current memory usage
        size_t peakWorkingSetSize = 0;  // Peak memory usage
        size_t privateUsage = 0;        // Private memory usage
        size_t virtualSize = 0;         // Virtual memory size
    };
    
    static MemoryInfo getCurrentMemoryInfo();
    static void printMemoryInfo(const MemoryInfo& info, const QString& label = "Memory");
    static MemoryInfo getMemoryDifference(const MemoryInfo& before, const MemoryInfo& after);

private:
#ifdef _WIN32
    static bool getWindowsMemoryInfo(MemoryInfo& info);
#endif
};

/**
 * @brief Visual Studio profiling utilities
 */
class VSProfilingUtils {
public:
    // Visual Studio profiling markers
    static void markProfilingStart(const QString& name);
    static void markProfilingEnd(const QString& name);
    static void markProfilingEvent(const QString& event);
    
    // Memory snapshot utilities
    static void takeMemorySnapshot(const QString& label);
    static void enableHeapProfiling();
    static void disableHeapProfiling();
    
    // CPU profiling utilities
    static void enableCPUProfiling();
    static void disableCPUProfiling();
    
private:
    static bool s_profilingEnabled;
};

/**
 * @brief RAII-style profiling scope for automatic timing
 */
class ProfilingScope {
public:
    explicit ProfilingScope(const QString& name);
    ~ProfilingScope();

private:
    PerformanceTimer m_timer;
};

/**
 * @brief Macros for easy profiling integration
 */
#ifdef RME_PROFILING_BUILD
    #define RME_PROFILE_SCOPE(name) RME::profiling::ProfilingScope _prof_scope(name)
    #define RME_PROFILE_FUNCTION() RME_PROFILE_SCOPE(__FUNCTION__)
    #define RME_PROFILE_BLOCK(name) for(RME::profiling::ProfilingScope _prof_scope(name); _prof_scope; _prof_scope = {})
#else
    #define RME_PROFILE_SCOPE(name)
    #define RME_PROFILE_FUNCTION()
    #define RME_PROFILE_BLOCK(name)
#endif

} // namespace profiling
} // namespace RME

#endif // RME_PROFILING_UTILS_H