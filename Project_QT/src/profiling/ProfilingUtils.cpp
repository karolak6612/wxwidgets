#include "ProfilingUtils.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <algorithm>
#include <numeric>

namespace RME {
namespace profiling {

// PerformanceTimer Implementation
PerformanceTimer::PerformanceTimer(const QString& name)
    : m_name(name)
    , m_totalElapsed(0)
    , m_isRunning(false)
{
}

PerformanceTimer::~PerformanceTimer()
{
    if (m_isRunning) {
        stop();
        printResults();
    }
}

void PerformanceTimer::start()
{
    if (!m_isRunning) {
        m_timer.start();
        m_isRunning = true;
    }
}

void PerformanceTimer::stop()
{
    if (m_isRunning) {
        m_totalElapsed += m_timer.nsecsElapsed();
        m_isRunning = false;
    }
}

void PerformanceTimer::reset()
{
    m_totalElapsed = 0;
    m_isRunning = false;
}

qint64 PerformanceTimer::elapsedNanoseconds() const
{
    qint64 elapsed = m_totalElapsed;
    if (m_isRunning) {
        elapsed += m_timer.nsecsElapsed();
    }
    return elapsed;
}

double PerformanceTimer::elapsedMilliseconds() const
{
    return elapsedNanoseconds() / 1000000.0;
}

double PerformanceTimer::elapsedSeconds() const
{
    return elapsedNanoseconds() / 1000000000.0;
}

void PerformanceTimer::printResults() const
{
    qDebug() << QString("[PROFILE] %1: %2 ms").arg(m_name).arg(elapsedMilliseconds(), 0, 'f', 3);
}

// MemoryTracker Implementation
MemoryTracker::MemoryInfo MemoryTracker::getCurrentMemoryInfo()
{
    MemoryInfo info;
    
#ifdef _WIN32
    getWindowsMemoryInfo(info);
#else
    // Linux/macOS implementation would go here
    // For now, return empty info
#endif
    
    return info;
}

void MemoryTracker::printMemoryInfo(const MemoryInfo& info, const QString& label)
{
    qDebug() << QString("[MEMORY] %1:").arg(label);
    qDebug() << QString("  Working Set: %1 KB").arg(info.workingSetSize / 1024);
    qDebug() << QString("  Peak Working Set: %1 KB").arg(info.peakWorkingSetSize / 1024);
    qDebug() << QString("  Private Usage: %1 KB").arg(info.privateUsage / 1024);
    qDebug() << QString("  Virtual Size: %1 KB").arg(info.virtualSize / 1024);
}

MemoryTracker::MemoryInfo MemoryTracker::getMemoryDifference(const MemoryInfo& before, const MemoryInfo& after)
{
    MemoryInfo delta;
    delta.workingSetSize = after.workingSetSize - before.workingSetSize;
    delta.peakWorkingSetSize = after.peakWorkingSetSize - before.peakWorkingSetSize;
    delta.privateUsage = after.privateUsage - before.privateUsage;
    delta.virtualSize = after.virtualSize - before.virtualSize;
    return delta;
}

#ifdef _WIN32
bool MemoryTracker::getWindowsMemoryInfo(MemoryInfo& info)
{
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        info.workingSetSize = pmc.WorkingSetSize;
        info.peakWorkingSetSize = pmc.PeakWorkingSetSize;
        info.privateUsage = pmc.PrivateUsage;
        
        // Get virtual memory size
        MEMORY_BASIC_INFORMATION mbi;
        SIZE_T virtualSize = 0;
        LPVOID addr = 0;
        
        while (VirtualQuery(addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            if (mbi.State == MEM_COMMIT) {
                virtualSize += mbi.RegionSize;
            }
            addr = (LPBYTE)mbi.BaseAddress + mbi.RegionSize;
        }
        
        info.virtualSize = virtualSize;
        return true;
    }
    return false;
}
#endif

// VSProfilingUtils Implementation
bool VSProfilingUtils::s_profilingEnabled = false;

void VSProfilingUtils::markProfilingStart(const QString& name)
{
#ifdef _WIN32
    // Output profiling marker for Visual Studio
    qDebug() << QString("[VS_PROFILE_START] %1").arg(name);
    OutputDebugStringA(QString("[VS_PROFILE_START] %1\n").arg(name).toLocal8Bit().data());
#endif
}

void VSProfilingUtils::markProfilingEnd(const QString& name)
{
#ifdef _WIN32
    // Output profiling marker for Visual Studio
    qDebug() << QString("[VS_PROFILE_END] %1").arg(name);
    OutputDebugStringA(QString("[VS_PROFILE_END] %1\n").arg(name).toLocal8Bit().data());
#endif
}

void VSProfilingUtils::markProfilingEvent(const QString& event)
{
#ifdef _WIN32
    // Output profiling event for Visual Studio
    qDebug() << QString("[VS_PROFILE_EVENT] %1").arg(event);
    OutputDebugStringA(QString("[VS_PROFILE_EVENT] %1\n").arg(event).toLocal8Bit().data());
#endif
}

void VSProfilingUtils::takeMemorySnapshot(const QString& label)
{
#ifdef _WIN32
    auto memInfo = MemoryTracker::getCurrentMemoryInfo();
    qDebug() << QString("[VS_MEMORY_SNAPSHOT] %1").arg(label);
    MemoryTracker::printMemoryInfo(memInfo, QString("Snapshot: %1").arg(label));
    
    // Force garbage collection to get accurate memory readings
    _CrtCheckMemory();
#endif
}

void VSProfilingUtils::enableHeapProfiling()
{
#ifdef _WIN32
    qDebug() << "[VS_PROFILING] Enabling heap profiling";
    s_profilingEnabled = true;
    
    // Enable detailed heap checking
    int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    flags |= _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF;
    _CrtSetDbgFlag(flags);
#endif
}

void VSProfilingUtils::disableHeapProfiling()
{
#ifdef _WIN32
    qDebug() << "[VS_PROFILING] Disabling heap profiling";
    s_profilingEnabled = false;
    
    // Reset heap checking to default
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}

void VSProfilingUtils::enableCPUProfiling()
{
    qDebug() << "[VS_PROFILING] CPU profiling enabled - use Visual Studio Performance Profiler";
    s_profilingEnabled = true;
}

void VSProfilingUtils::disableCPUProfiling()
{
    qDebug() << "[VS_PROFILING] CPU profiling disabled";
    s_profilingEnabled = false;
}

// ProfilingScope Implementation
ProfilingScope::ProfilingScope(const QString& name)
    : m_timer(name)
{
    m_timer.start();
}

ProfilingScope::~ProfilingScope()
{
    m_timer.stop();
    m_timer.printResults();
}

} // namespace profiling
} // namespace RME