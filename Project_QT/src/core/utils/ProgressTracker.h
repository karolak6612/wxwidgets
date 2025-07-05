#ifndef RME_PROGRESS_TRACKER_H
#define RME_PROGRESS_TRACKER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <functional>

namespace RME {
namespace core {
namespace utils {

/**
 * @brief Tracks progress for long-running operations with cancellation support
 * 
 * This class provides a standardized way to track progress for map-wide operations,
 * import/export tasks, and other long-running processes. It supports progress
 * callbacks, cancellation, and automatic UI updates.
 */
class ProgressTracker : public QObject {
    Q_OBJECT

public:
    explicit ProgressTracker(QObject* parent = nullptr);
    ~ProgressTracker() override = default;

    // Progress management
    void start(const QString& operationName, quint32 totalSteps = 100);
    void setProgress(quint32 currentStep, const QString& message = QString());
    void setProgress(int percentage, const QString& message = QString());
    void finish(const QString& message = QString());
    void cancel();
    
    // State queries
    bool isRunning() const { return m_isRunning; }
    bool isCancelled() const { return m_isCancelled; }
    int getProgress() const { return m_currentProgress; }
    QString getCurrentMessage() const { return m_currentMessage; }
    QString getOperationName() const { return m_operationName; }
    
    // Callbacks
    void setProgressCallback(std::function<void(int, const QString&)> callback);
    void setCancellationCallback(std::function<bool()> callback);
    
    // Convenience methods
    void incrementProgress(const QString& message = QString());
    void setSubOperation(const QString& subOperationName, quint32 subSteps = 100);

public slots:
    void requestCancel();

signals:
    void progressChanged(int percentage, const QString& message);
    void operationStarted(const QString& operationName);
    void operationFinished(const QString& message);
    void operationCancelled();

private slots:
    void updateUI();

private:
    QString m_operationName;
    QString m_currentMessage;
    quint32 m_totalSteps = 100;
    quint32 m_currentStep = 0;
    int m_currentProgress = 0;
    bool m_isRunning = false;
    bool m_isCancelled = false;
    
    // Sub-operation support
    QString m_subOperationName;
    quint32 m_subTotalSteps = 100;
    quint32 m_subCurrentStep = 0;
    int m_subOperationWeight = 100; // Percentage of main operation
    
    // Callbacks
    std::function<void(int, const QString&)> m_progressCallback;
    std::function<bool()> m_cancellationCallback;
    
    // UI update timer
    QTimer* m_updateTimer;
    
    // Helper methods
    void updateProgress();
    QString formatMessage(const QString& message) const;
};

} // namespace utils
} // namespace core
} // namespace RME

#endif // RME_PROGRESS_TRACKER_H