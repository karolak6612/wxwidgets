#include "BrushStateService.h"
#include "Brush.h"
#include "BrushIntegrationManager.h"

namespace RME {
namespace core {
namespace brush {

BrushStateService::BrushStateService(BrushIntegrationManager* brushManager, QObject* parent)
    : QObject(parent)
    , m_brushManager(brushManager)
    , m_currentBrushType(BrushType::None)
    , m_currentBrushShape(BrushShape::Square)
    , m_currentBrushSize(1)
    , m_brushEnabled(true)
{
    Q_ASSERT(m_brushManager);
    
    // Connect to brush manager signals
    connect(m_brushManager, &BrushIntegrationManager::brushChanged,
            this, &BrushStateService::currentBrushChanged);
}

Brush* BrushStateService::getCurrentBrush() const
{
    return m_brushManager->getCurrentBrush();
}

BrushType BrushStateService::getCurrentBrushType() const
{
    return m_currentBrushType;
}

BrushShape BrushStateService::getCurrentBrushShape() const
{
    return m_currentBrushShape;
}

int BrushStateService::getCurrentBrushSize() const
{
    return m_currentBrushSize;
}

BrushSettings BrushStateService::getCurrentBrushSettings() const
{
    return m_currentBrushSettings;
}

bool BrushStateService::isBrushEnabled() const
{
    return m_brushEnabled;
}

void BrushStateService::setCurrentBrushType(BrushType type)
{
    if (m_currentBrushType != type) {
        m_currentBrushType = type;
        
        // Update the brush in the manager
        m_brushManager->setBrushType(type);
        
        emit currentBrushTypeChanged(type);
    }
}

void BrushStateService::setCurrentBrushShape(BrushShape shape)
{
    if (m_currentBrushShape != shape) {
        m_currentBrushShape = shape;
        
        // Update the brush shape in the manager
        m_brushManager->setBrushShape(shape);
        
        emit currentBrushShapeChanged(shape);
    }
}

void BrushStateService::setCurrentBrushSize(int size)
{
    if (m_currentBrushSize != size) {
        m_currentBrushSize = size;
        
        // Update the brush size in the manager
        m_brushManager->setBrushSize(size);
        
        emit currentBrushSizeChanged(size);
    }
}

void BrushStateService::setCurrentBrushSettings(const BrushSettings& settings)
{
    m_currentBrushSettings = settings;
    
    // Update the brush settings in the manager
    m_brushManager->setBrushSettings(settings);
    
    emit currentBrushSettingsChanged(settings);
}

void BrushStateService::setBrushEnabled(bool enabled)
{
    if (m_brushEnabled != enabled) {
        m_brushEnabled = enabled;
        
        // Update the brush enabled state in the manager
        m_brushManager->setBrushEnabled(enabled);
        
        emit brushEnabledChanged(enabled);
    }
}

} // namespace brush
} // namespace core
} // namespace RME