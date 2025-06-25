#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTimer>
#include <QMainWindow>
#include <cmath>
#include <memory>

#include "ProfilingUtils.h"
#include "../core/services/ServiceContainer.h"
#include "../ui/MainWindow.h"

#ifdef _WIN32
#include <windows.h>
#include <crtdbg.h>
#endif

using namespace RME::profiling;

void printHeader()
{
    qDebug() << "========================================";
    qDebug() << "  RME-Qt6 Visual Studio Profiling Target";
    qDebug() << "  REFACTOR-02 Implementation";
    qDebug() << "  Microsoft Visual Studio + CMake";
    qDebug() << "========================================";
    qDebug() << "Build Date:" << __DATE__ << __TIME__;
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Compiler:" << 
#ifdef _MSC_VER
        "MSVC" << _MSC_VER;
#elif defined(__GNUC__)
        "GCC" << __GNUC__ << "." << __GNUC_MINOR__;
#elif defined(__clang__)
        "Clang" << __clang_major__ << "." << __clang_minor__;
#else
        "Unknown";
#endif
    qDebug() << "========================================\n";
}

void printSystemInfo()
{
    qDebug() << "=== System Information ===";
    
    auto memInfo = MemoryTracker::getCurrentMemoryInfo();
    MemoryTracker::printMemoryInfo(memInfo, "Initial Memory State");
    
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    qDebug() << "Processor Count:" << sysInfo.dwNumberOfProcessors;
    qDebug() << "Page Size:" << sysInfo.dwPageSize << "bytes";
    
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        qDebug() << "Total Physical Memory:" << (memStatus.ullTotalPhys / 1024 / 1024) << "MB";
        qDebug() << "Available Physical Memory:" << (memStatus.ullAvailPhys / 1024 / 1024) << "MB";
    }
#endif
    
    qDebug() << "";
}

void enableMemoryLeakDetection()
{
#ifdef _WIN32
    // Enable CRT debug heap for memory leak detection
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    qDebug() << "[PROFILING] Memory leak detection enabled";
#endif
}

void runApplicationStartupScenario()
{
    qDebug() << "[SCENARIO] Application Startup - Creating services...";
    VSProfilingUtils::markProfilingStart("Application Startup");
    RME_PROFILE_SCOPE("Application Startup");
    
    VSProfilingUtils::takeMemorySnapshot("Before Startup");
    
    // Simulate application startup
    VSProfilingUtils::markProfilingEvent("Creating Service Container");
    auto serviceContainer = std::make_unique<RME::core::ServiceContainer>();
    
    VSProfilingUtils::markProfilingEvent("Service Container Created");
    VSProfilingUtils::takeMemorySnapshot("After Service Container");
    
    // Hold for profiling
    QTimer::singleShot(2000, []() {
        VSProfilingUtils::markProfilingEnd("Application Startup");
        qDebug() << "[SCENARIO] Startup scenario complete";
    });
}

void runUICreationScenario()
{
    qDebug() << "[SCENARIO] UI Creation - Creating main window...";
    VSProfilingUtils::markProfilingStart("UI Creation");
    RME_PROFILE_SCOPE("UI Creation");
    
    VSProfilingUtils::takeMemorySnapshot("Before UI Creation");
    
    // Create main window for profiling
    VSProfilingUtils::markProfilingEvent("Creating Main Window");
    auto mainWindow = std::make_unique<RME::ui::MainWindow>();
    
    VSProfilingUtils::markProfilingEvent("Showing Main Window");
    mainWindow->show();
    
    VSProfilingUtils::takeMemorySnapshot("After UI Creation");
    
    // Hold for profiling
    QTimer::singleShot(3000, []() {
        VSProfilingUtils::markProfilingEnd("UI Creation");
        qDebug() << "[SCENARIO] UI creation scenario complete";
    });
}

void runMemoryStressScenario()
{
    qDebug() << "[SCENARIO] Memory Stress - Allocating and deallocating...";
    VSProfilingUtils::markProfilingStart("Memory Stress");
    VSProfilingUtils::enableHeapProfiling();
    RME_PROFILE_SCOPE("Memory Stress");
    
    VSProfilingUtils::takeMemorySnapshot("Before Memory Stress");
    
    // Memory stress test for profiling
    std::vector<std::unique_ptr<char[]>> allocations;
    
    VSProfilingUtils::markProfilingEvent("Starting Memory Allocations");
    for (int i = 0; i < 1000; ++i) {
        size_t size = 1024 * (i % 100 + 1); // Variable sizes
        allocations.push_back(std::make_unique<char[]>(size));
        
        // Take snapshots at intervals
        if (i % 250 == 0) {
            VSProfilingUtils::takeMemorySnapshot(QString("Allocation %1").arg(i));
        }
        
        // Occasionally process events
        if (i % 100 == 0) {
            QApplication::processEvents();
        }
    }
    
    VSProfilingUtils::markProfilingEvent("Starting Memory Deallocation");
    VSProfilingUtils::takeMemorySnapshot("Before Deallocation");
    
    // Clear allocations
    allocations.clear();
    
    VSProfilingUtils::takeMemorySnapshot("After Deallocation");
    VSProfilingUtils::disableHeapProfiling();
    VSProfilingUtils::markProfilingEnd("Memory Stress");
}

void runCPUIntensiveScenario()
{
    qDebug() << "[SCENARIO] CPU Intensive - Mathematical operations...";
    VSProfilingUtils::markProfilingStart("CPU Intensive");
    VSProfilingUtils::enableCPUProfiling();
    RME_PROFILE_SCOPE("CPU Intensive");
    
    // CPU-intensive operations for profiling
    VSProfilingUtils::markProfilingEvent("Starting Mathematical Computations");
    volatile double result = 0.0;
    for (int i = 0; i < 1000000; ++i) {
        result += std::sin(i) * std::cos(i) * std::sqrt(i + 1);
        
        // Mark progress for profiling
        if (i % 100000 == 0) {
            VSProfilingUtils::markProfilingEvent(QString("Computation Progress: %1%").arg(i / 10000));
        }
        
        // Occasionally process events
        if (i % 10000 == 0) {
            QApplication::processEvents();
        }
    }
    
    VSProfilingUtils::markProfilingEvent("Mathematical Computations Complete");
    VSProfilingUtils::disableCPUProfiling();
    VSProfilingUtils::markProfilingEnd("CPU Intensive");
    qDebug() << "[SCENARIO] CPU intensive result:" << result;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("RME Visual Studio Profiling Target");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("RME Team");
    
    // Enable memory leak detection
    enableMemoryLeakDetection();
    
    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("RME-Qt6 Visual Studio Profiling Target for REFACTOR-02");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Command line options for profiling scenarios
    QCommandLineOption profileScenariosOption("profile-scenarios", "Run all profiling scenarios");
    QCommandLineOption startupOption("startup", "Run application startup scenario");
    QCommandLineOption uiOption("ui", "Run UI creation scenario");
    QCommandLineOption memoryOption("memory", "Run memory stress scenario");
    QCommandLineOption cpuOption("cpu", "Run CPU intensive scenario");
    QCommandLineOption interactiveOption("interactive", "Run in interactive mode (keeps app running)");
    QCommandLineOption durationOption("duration", "Duration to run scenarios (seconds)", "duration", "30");
    
    parser.addOption(profileScenariosOption);
    parser.addOption(startupOption);
    parser.addOption(uiOption);
    parser.addOption(memoryOption);
    parser.addOption(cpuOption);
    parser.addOption(interactiveOption);
    parser.addOption(durationOption);
    
    parser.process(app);
    
    // Print header and system info
    printHeader();
    printSystemInfo();
    
    // Determine what scenarios to run
    bool runStartup = parser.isSet(startupOption) || parser.isSet(profileScenariosOption);
    bool runUI = parser.isSet(uiOption) || parser.isSet(profileScenariosOption);
    bool runMemory = parser.isSet(memoryOption) || parser.isSet(profileScenariosOption);
    bool runCPU = parser.isSet(cpuOption) || parser.isSet(profileScenariosOption);
    bool interactive = parser.isSet(interactiveOption);
    
    // If no specific scenarios specified, run all by default
    if (!runStartup && !runUI && !runMemory && !runCPU && !interactive) {
        runStartup = runUI = runMemory = runCPU = true;
    }
    
    // Get duration
    int duration = parser.value(durationOption).toInt();
    if (duration <= 0) {
        duration = 30;
    }
    
    qDebug() << "Profiling Configuration:";
    qDebug() << "  Startup Scenario:" << (runStartup ? "Enabled" : "Disabled");
    qDebug() << "  UI Creation Scenario:" << (runUI ? "Enabled" : "Disabled");
    qDebug() << "  Memory Stress Scenario:" << (runMemory ? "Enabled" : "Disabled");
    qDebug() << "  CPU Intensive Scenario:" << (runCPU ? "Enabled" : "Disabled");
    qDebug() << "  Interactive Mode:" << (interactive ? "Enabled" : "Disabled");
    qDebug() << "  Duration:" << duration << "seconds";
    qDebug() << "";
    
    qDebug() << "=== Visual Studio Profiling Instructions ===";
    qDebug() << "1. Open Visual Studio";
    qDebug() << "2. Go to Debug > Performance Profiler";
    qDebug() << "3. Select this executable as the target";
    qDebug() << "4. Choose profiling tools:";
    qDebug() << "   - CPU Usage (for CPU bottlenecks)";
    qDebug() << "   - Memory Usage (for memory leaks/usage)";
    qDebug() << "   - .NET/C++ Heap (for detailed memory analysis)";
    qDebug() << "5. Click 'Start' to begin profiling";
    qDebug() << "==========================================\n";
    
    // Run profiling scenarios
    try {
        RME_PROFILE_SCOPE("Total Profiling Session");
        
        if (runStartup) {
            runApplicationStartupScenario();
        }
        
        if (runUI) {
            runUICreationScenario();
        }
        
        if (runMemory) {
            runMemoryStressScenario();
        }
        
        if (runCPU) {
            runCPUIntensiveScenario();
        }
        
        if (interactive) {
            qDebug() << "\n[INTERACTIVE] Application running in interactive mode...";
            qDebug() << "[INTERACTIVE] Use Visual Studio profiler to analyze performance";
            qDebug() << "[INTERACTIVE] Close this window or press Ctrl+C to exit";
            
            // Keep application running for interactive profiling
            return app.exec();
        } else {
            // Run for specified duration
            qDebug() << QString("\n[PROFILING] Running scenarios for %1 seconds...").arg(duration);
            
            QTimer::singleShot(duration * 1000, &app, &QApplication::quit);
            return app.exec();
        }
        
    } catch (const std::exception& e) {
        qCritical() << "Profiling scenario execution failed:" << e.what();
        return 1;
    }
}

#ifdef RME_PROFILING_BUILD
// Additional profiling hooks for Visual Studio
extern "C" {
    // These functions can be called by Visual Studio profiler
    __declspec(dllexport) void RME_StartProfiling() {
        qDebug() << "RME Profiling Started";
    }
    
    __declspec(dllexport) void RME_StopProfiling() {
        qDebug() << "RME Profiling Stopped";
    }
    
    __declspec(dllexport) void RME_MarkProfilingPoint(const char* name) {
        qDebug() << "Profiling Point:" << name;
    }
}
#endif