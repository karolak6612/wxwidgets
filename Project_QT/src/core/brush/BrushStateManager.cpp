#include "core/brush/BrushStateManager.h"
#include "core/brush/Brush.h"

#include <QDebug>

namespace RME {
namespace core {
namespace brush {

BrushStateManager::BrushStateManager(QObject* parent)
    : QObject(parent)
{
    qDebug() << "BrushStateManager: Initialized";
}

void BrushStateManager::registerBrush(const QString& brushId, RME::core::brush::Brush* brush) {
    if (!brush) {
        qWarning() << "BrushStateManager::registerBrush: Brush is null for ID" << brushId;
        return;
    }
    
    if (brushId.isEmpty()) {
        qWarning() << "BrushStateManager::registerBrush: Brush ID is empty";
        return;
    }
    
    if (m_brushRegistry.contains(brushId)) {
        qWarning() << "BrushStateManager::registerBrush: Brush ID" << brushId << "already registered";
        return;
    }
    
    m_brushRegistry[brushId] = brush;
    emit brushRegistered(brushId, brush);
    
    qDebug() << "BrushStateManager::registerBrush: Registered brush" << brushId << "(" << brush->getName() << ")";
}

void BrushStateManager::unregisterBrush(const QString& brushId) {
    if (!m_brushRegistry.contains(brushId)) {
        qWarning() << "BrushStateManager::unregisterBrush: Brush ID" << brushId << "not found";
        return;
    }
    
    RME::core::brush::Brush* brush = m_brushRegistry.value(brushId);
    
    // If this was the active brush, clear it
    if (m_activeBrush == brush) {
        m_activeBrush = nullptr;
        m_activeBrushId.clear();
        emit activeBrushChanged(nullptr, QString());
    }
    
    // If this was the previous brush, clear it
    if (m_previousBrush == brush) {
        m_previousBrush = nullptr;
        m_previousBrushId.clear();
    }
    
    m_brushRegistry.remove(brushId);
    emit brushUnregistered(brushId);
    
    qDebug() << "BrushStateManager::unregisterBrush: Unregistered brush" << brushId;
}

void BrushStateManager::setActiveBrush(const QString& brushId) {
    if (!m_brushRegistry.contains(brushId)) {
        qWarning() << "BrushStateManager::setActiveBrush: Brush ID" << brushId << "not found";
        return;
    }
    
    RME::core::brush::Brush* brush = m_brushRegistry.value(brushId);
    updateActiveBrush(brush, brushId);
}

void BrushStateManager::setActiveBrush(RME::core::brush::Brush* brush) {
    if (!brush) {
        updateActiveBrush(nullptr, QString());
        return;
    }
    
    // Find the brush ID for this brush
    QString brushId;
    for (auto it = m_brushRegistry.constBegin(); it != m_brushRegistry.constEnd(); ++it) {
        if (it.value() == brush) {
            brushId = it.key();
            break;
        }
    }
    
    if (brushId.isEmpty()) {
        // Generate a temporary ID for unregistered brushes
        brushId = generateBrushId(brush);
        qDebug() << "BrushStateManager::setActiveBrush: Using generated ID" << brushId << "for unregistered brush";
    }
    
    updateActiveBrush(brush, brushId);
}

RME::core::brush::Brush* BrushStateManager::getActiveBrush() const {
    return m_activeBrush;
}

QString BrushStateManager::getActiveBrushId() const {
    return m_activeBrushId;
}

RME::core::brush::Brush* BrushStateManager::getBrush(const QString& brushId) const {
    return m_brushRegistry.value(brushId, nullptr);
}

QStringList BrushStateManager::getRegisteredBrushIds() const {
    return m_brushRegistry.keys();
}

bool BrushStateManager::hasBrush(const QString& brushId) const {
    return m_brushRegistry.contains(brushId);
}

void BrushStateManager::saveCurrentState() {
    m_previousBrush = m_activeBrush;
    m_previousBrushId = m_activeBrushId;
    
    qDebug() << "BrushStateManager::saveCurrentState: Saved state for brush" << m_activeBrushId;
}

void BrushStateManager::restorePreviousState() {
    if (m_previousBrush) {
        updateActiveBrush(m_previousBrush, m_previousBrushId);
        qDebug() << "BrushStateManager::restorePreviousState: Restored to brush" << m_previousBrushId;
    } else {
        qDebug() << "BrushStateManager::restorePreviousState: No previous state to restore";
    }
}

void BrushStateManager::clearState() {
    m_activeBrush = nullptr;
    m_activeBrushId.clear();
    m_previousBrush = nullptr;
    m_previousBrushId.clear();
    
    emit activeBrushChanged(nullptr, QString());
    emit stateChanged();
    
    qDebug() << "BrushStateManager::clearState: Cleared all state";
}

// Slots
void BrushStateManager::onBrushActivated(RME::core::brush::Brush* brush) {
    setActiveBrush(brush);
}

void BrushStateManager::onToolModeChanged(int toolMode) {
    // Tool mode changes might affect brush state
    emit stateChanged();
    
    qDebug() << "BrushStateManager::onToolModeChanged: Tool mode changed to" << toolMode;
}

// Private helper methods
QString BrushStateManager::generateBrushId(RME::core::brush::Brush* brush) const {
    if (!brush) {
        return QString();
    }
    
    // Generate ID based on brush name and pointer address
    return QString("%1_%2").arg(brush->getName()).arg(reinterpret_cast<quintptr>(brush), 0, 16);
}

void BrushStateManager::updateActiveBrush(RME::core::brush::Brush* brush, const QString& brushId) {
    if (m_activeBrush == brush && m_activeBrushId == brushId) {
        return; // No change
    }
    
    // Save current state before changing
    if (m_activeBrush != brush) {
        saveCurrentState();
    }
    
    m_activeBrush = brush;
    m_activeBrushId = brushId;
    
    emit activeBrushChanged(brush, brushId);
    emit stateChanged();
    
    if (brush) {
        qDebug() << "BrushStateManager::updateActiveBrush: Activated brush" << brushId << "(" << brush->getName() << ")";
    } else {
        qDebug() << "BrushStateManager::updateActiveBrush: Deactivated brush";
    }
}

} // namespace brush
} // namespace core
} // namespace RME

// #include "BrushStateManager.moc" // Removed - Q_OBJECT is in header