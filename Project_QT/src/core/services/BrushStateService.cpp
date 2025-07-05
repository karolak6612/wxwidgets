#include "BrushStateService.h"
#include "core/brush/Brush.h"
#include "core/assets/CreatureData.h"
#include "core/map/BaseMap.h"
#include <QDebug>

namespace RME {
namespace core {

BrushStateService::BrushStateService(QObject* parent)
    : IBrushStateService(parent)
    , m_activeBrush(nullptr)
    , m_brushShape(BrushShape::Square)
    , m_brushSize(1)
    , m_brushVariation(0)
    , m_drawLockedDoors(false)
    , m_useCustomThickness(false)
    , m_customThicknessMod(1.0f)
    , m_currentRawItemId(0)
    , m_currentCreatureType(nullptr)
    , m_doodadBufferMap(nullptr)
{
    qDebug() << "BrushStateService: Initialized with default settings";
}

BrushStateService::~BrushStateService()
{
    // Note: We don't own the brush, creature data, or buffer map
    // so we don't delete them here
}

void BrushStateService::setActiveBrush(Brush* brush)
{
    if (m_activeBrush != brush) {
        m_activeBrush = brush;
        emit activeBrushChanged(brush);
        qDebug() << "BrushStateService: Active brush changed to" << (brush ? brush->getName() : "null");
    }
}

Brush* BrushStateService::getActiveBrush() const
{
    return m_activeBrush;
}

void BrushStateService::setBrushShape(BrushShape shape)
{
    if (m_brushShape != shape) {
        m_brushShape = shape;
        emit brushShapeChanged(shape);
        qDebug() << "BrushStateService: Brush shape changed to" << static_cast<int>(shape);
    }
}

BrushShape BrushStateService::getBrushShape() const
{
    return m_brushShape;
}

void BrushStateService::setBrushSize(int size)
{
    // Clamp size to reasonable bounds
    int clampedSize = qMax(1, qMin(size, 50));
    
    if (m_brushSize != clampedSize) {
        m_brushSize = clampedSize;
        emit brushSizeChanged(clampedSize);
        qDebug() << "BrushStateService: Brush size changed to" << clampedSize;
    }
}

int BrushStateService::getBrushSize() const
{
    return m_brushSize;
}

void BrushStateService::setBrushVariation(int variation)
{
    // Clamp variation to reasonable bounds
    int clampedVariation = qMax(0, qMin(variation, 100));
    
    if (m_brushVariation != clampedVariation) {
        m_brushVariation = clampedVariation;
        emit brushVariationChanged(clampedVariation);
        qDebug() << "BrushStateService: Brush variation changed to" << clampedVariation;
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
        qDebug() << "BrushStateService: Draw locked doors" << (enabled ? "enabled" : "disabled");
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
        qDebug() << "BrushStateService: Custom thickness" << (enabled ? "enabled" : "disabled");
    }
}

bool BrushStateService::getUseCustomThickness() const
{
    return m_useCustomThickness;
}

void BrushStateService::setCustomThicknessMod(float mod)
{
    // Clamp modifier to reasonable bounds
    float clampedMod = qMax(0.1f, qMin(mod, 10.0f));
    
    if (qAbs(m_customThicknessMod - clampedMod) > 0.001f) {
        m_customThicknessMod = clampedMod;
        emit customThicknessChanged(m_useCustomThickness, clampedMod);
        emit brushSettingsChanged();
        qDebug() << "BrushStateService: Custom thickness modifier changed to" << clampedMod;
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
        qDebug() << "BrushStateService: Current raw item ID changed to" << itemId;
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
        qDebug() << "BrushStateService: Current creature type changed to" 
                 << (creature ? creature->name : "null");
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
        qDebug() << "BrushStateService: Doodad buffer map changed to" << (map ? "valid map" : "null");
    }
}

BaseMap* BrushStateService::getDoodadBufferMap() const
{
    return m_doodadBufferMap;
}

} // namespace core
} // namespace RME