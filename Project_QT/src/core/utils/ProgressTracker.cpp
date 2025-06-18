#include "core/utils/ProgressTracker.h"
#include <QDebug>

namespace RME {
namespace core {
namespace utils {

ProgressTracker::ProgressTracker(QObject* parent)
    : QObject(parent)
    , m_updateTimer(new QTimer(this))
{
    m_updateTimer->setSingleShot(false);
    m_updateTimer->setInterval(100); // Update UI every 100ms
    connect(m_updateTimer, &QTimer::timeout, this, &ProgressTracker::updateUI);
}

void ProgressTracker::start(const QString& operationName, quint32 totalSteps) {
    m_operationName = operationName;
    m_totalSteps = totalSteps;
    m_currentStep = 0;
    m_currentProgress = 0;
    m_currentMessage.clear();
    m_isRunning = true;
    m_isCancelled = false;
    
    // Clear sub-operation state
    m_subOperationName.clear();
    m_subTotalSteps = 100;
    m_subCurrentStep = 0;
    m_subOperationWeight = 100;
    
    m_updateTimer->start();
    
    emit operationStarted(m_operationName);
    emit progressChanged(0, formatMessage(QString()));
    
    qDebug() << "ProgressTracker::start:" << m_operationName << "with" << m_totalSteps << "steps";
}

void ProgressTracker::setProgress(quint32 currentStep, const QString& message) {
    if (!m_isRunning || m_isCancelled) {
        return;
    }
    
    m_currentStep = qMin(currentStep, m_totalSteps);
    m_currentMessage = message;
    
    updateProgress();
}

void ProgressTracker::setProgress(int percentage, const QString& message) {
    if (!m_isRunning || m_isCancelled) {
        return;
    }
    
    m_currentProgress = qBound(0, percentage, 100);
    m_currentMessage = message;
    
    // Update step based on percentage
    m_currentStep = (m_currentProgress * m_totalSteps) / 100;
    
    // Don't call updateProgress() here to avoid double calculation
    if (m_progressCallback) {
        m_progressCallback(m_currentProgress, formatMessage(message));
    }
}

void ProgressTracker::finish(const QString& message) {
    if (!m_isRunning) {
        return;
    }
    
    m_currentStep = m_totalSteps;
    m_currentProgress = 100;
    m_currentMessage = message.isEmpty() ? QObject::tr("Operation completed") : message;
    m_isRunning = false;
    
    m_updateTimer->stop();
    
    emit progressChanged(100, formatMessage(m_currentMessage));
    emit operationFinished(m_currentMessage);
    
    qDebug() << "ProgressTracker::finish:" << m_operationName << "-" << m_currentMessage;
}

void ProgressTracker::cancel() {
    if (!m_isRunning) {
        return;
    }
    
    m_isCancelled = true;
    m_isRunning = false;
    m_currentMessage = QObject::tr("Operation cancelled");
    
    m_updateTimer->stop();
    
    emit progressChanged(m_currentProgress, formatMessage(m_currentMessage));
    emit operationCancelled();
    
    qDebug() << "ProgressTracker::cancel:" << m_operationName;
}

void ProgressTracker::setProgressCallback(std::function<void(int, const QString&)> callback) {
    m_progressCallback = callback;
}

void ProgressTracker::setCancellationCallback(std::function<bool()> callback) {
    m_cancellationCallback = callback;
}

void ProgressTracker::incrementProgress(const QString& message) {
    setProgress(m_currentStep + 1, message);
}

void ProgressTracker::setSubOperation(const QString& subOperationName, quint32 subSteps) {
    m_subOperationName = subOperationName;
    m_subTotalSteps = subSteps;
    m_subCurrentStep = 0;
    
    qDebug() << "ProgressTracker::setSubOperation:" << subOperationName << "with" << subSteps << "steps";
}

void ProgressTracker::requestCancel() {
    if (m_isRunning) {
        cancel();
    }
}

void ProgressTracker::updateUI() {
    if (!m_isRunning) {
        return;
    }
    
    // Check for cancellation
    if (m_cancellationCallback && m_cancellationCallback()) {
        cancel();
        return;
    }
    
    emit progressChanged(m_currentProgress, formatMessage(m_currentMessage));
}

void ProgressTracker::updateProgress() {
    if (m_totalSteps == 0) {
        m_currentProgress = 100;
    } else {
        m_currentProgress = (m_currentStep * 100) / m_totalSteps;
    }
    
    m_currentProgress = qBound(0, m_currentProgress, 100);
    
    if (m_progressCallback) {
        m_progressCallback(m_currentProgress, formatMessage(m_currentMessage));
    }
}

QString ProgressTracker::formatMessage(const QString& message) const {
    QString formattedMessage;
    
    if (!m_operationName.isEmpty()) {
        formattedMessage = m_operationName;
        
        if (!m_subOperationName.isEmpty()) {
            formattedMessage += QString(" - %1").arg(m_subOperationName);
        }
        
        if (!message.isEmpty()) {
            formattedMessage += QString(": %1").arg(message);
        }
    } else {
        formattedMessage = message;
    }
    
    return formattedMessage;
}

} // namespace utils
} // namespace core
} // namespace RME