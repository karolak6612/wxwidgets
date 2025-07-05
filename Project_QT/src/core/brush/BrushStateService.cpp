#include "BrushStateService.h"
#include "Brush.h"
#include "BrushIntegrationManager.h"
#include "core/assets/CreatureData.h"
#include "core/map/BaseMap.h"

namespace RME {
namespace core {
namespace brush {

BrushStateService::BrushStateService(BrushIntegrationManager* brushManager, QObject* parent)
    : IBrushStateService(parent)
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

// IBrushStateService implementation
void BrushStateService::setActiveBrush(Brush* brush)
{
    if (m_activeBrush != brush) {
        m_activeBrush = brush;
        emit activeBrushChanged(brush);
    }
}

Brush* BrushStateService::getActiveBrush() const
{
    return m_activeBrush;
}

void BrushStateService::setBrushShape(BrushShape shape)
{
    setCurrentBrushShape(shape);
    emit brushShapeChanged(shape);
}

BrushShape BrushStateService::getBrushShape() const
{
    return getCurrentBrushShape();
}

void BrushStateService::setBrushSize(int size)
{
    setCurrentBrushSize(size);
    emit brushSizeChanged(size);
}

int BrushStateService::getBrushSize() const
{
    return getCurrentBrushSize();
}

void BrushStateService::setBrushVariation(int variation)
{
    if (m_brushVariation != variation) {
        m_brushVariation = variation;
        emit brushVariationChanged(variation);
    }
}

int BrushStateService::getBrushVariation() const
{
    return m_brushVariation;
}

void BrushStateService::setDrawLockedDoors(bool enabled)
{
    if (m_drawLockedDoors != enabled) {
        m_drawLockedDoors = enabled;
        emit drawLockedDoorsChanged(enabled);
        emit brushSettingsChanged();
    }
}

bool BrushStateService::getDrawLockedDoors() const
{
    return m_drawLockedDoors;
}

void BrushStateService::setUseCustomThickness(bool enabled)
{
    if (m_useCustomThickness != enabled) {
        m_useCustomThickness = enabled;
        emit customThicknessChanged(enabled, m_customThicknessMod);
        emit brushSettingsChanged();
    }
}

bool BrushStateService::getUseCustomThickness() const
{
    return m_useCustomThickness;
}

void BrushStateService::setCustomThicknessMod(float mod)
{
    if (m_customThicknessMod != mod) {
        m_customThicknessMod = mod;
        emit customThicknessChanged(m_useCustomThickness, mod);
        emit brushSettingsChanged();
    }
}

float BrushStateService::getCustomThicknessMod() const
{
    return m_customThicknessMod;
}

void BrushStateService::setCurrentRawItemId(uint32_t itemId)
{
    if (m_currentRawItemId != itemId) {
        m_currentRawItemId = itemId;
        emit currentRawItemIdChanged(itemId);
    }
}

uint32_t BrushStateService::getCurrentRawItemId() const
{
    return m_currentRawItemId;
}

void BrushStateService::setCurrentCreatureType(const CreatureData* creature)
{
    if (m_currentCreatureType != creature) {
        m_currentCreatureType = creature;
        emit currentCreatureTypeChanged(creature);
    }
}

const CreatureData* BrushStateService::getCurrentCreatureType() const
{
    return m_currentCreatureType;
}

void BrushStateService::setDoodadBufferMap(BaseMap* map)
{
    if (m_doodadBufferMap != map) {
        m_doodadBufferMap = map;
        emit doodadBufferMapChanged(map);
    }
}

BaseMap* BrushStateService::getDoodadBufferMap() const
{
    return m_doodadBufferMap;
}

} // namespace brush
} // namespace core
} // namespace RME