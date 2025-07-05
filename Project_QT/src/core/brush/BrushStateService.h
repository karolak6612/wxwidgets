#ifndef RME_BRUSH_STATE_SERVICE_H
#define RME_BRUSH_STATE_SERVICE_H

#include <QObject>
#include <cstdint>
#include "BrushEnums.h"
#include "BrushShape.h"
#include "BrushSettings.h"
#include "services/IBrushStateService.h"

namespace RME {
namespace core {

class Brush;
class CreatureData;
class BaseMap;

namespace brush {
class BrushIntegrationManager;

/**
 * @brief Service that provides information about the current brush state
 * 
 * This service is responsible for tracking and providing information about
 * the current brush state, such as the current brush type, brush shape,
 * brush size, etc. It also emits signals when these values change.
 */
class BrushStateService : public IBrushStateService
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new Brush State Service
     * 
     * @param brushManager Brush integration manager
     * @param parent Parent QObject
     */
    explicit BrushStateService(BrushIntegrationManager* brushManager, QObject* parent = nullptr);
    
    // IBrushStateService interface implementation
    void setActiveBrush(Brush* brush) override;
    Brush* getActiveBrush() const override;
    
    void setBrushShape(BrushShape shape) override;
    BrushShape getBrushShape() const override;
    
    void setBrushSize(int size) override;
    int getBrushSize() const override;
    
    void setBrushVariation(int variation) override;
    int getBrushVariation() const override;
    
    void setDrawLockedDoors(bool enabled) override;
    bool getDrawLockedDoors() const override;
    
    void setUseCustomThickness(bool enabled) override;
    bool getUseCustomThickness() const override;
    
    void setCustomThicknessMod(float mod) override;
    float getCustomThicknessMod() const override;
    
    void setCurrentRawItemId(uint32_t itemId) override;
    uint32_t getCurrentRawItemId() const override;
    
    void setCurrentCreatureType(const CreatureData* creature) override;
    const CreatureData* getCurrentCreatureType() const override;
    
    void setDoodadBufferMap(BaseMap* map) override;
    BaseMap* getDoodadBufferMap() const override;
    
    /**
     * @brief Get the current brush
     * 
     * @return Brush* Pointer to the current brush
     */
    Brush* getCurrentBrush() const;
    
    /**
     * @brief Get the current brush type
     * 
     * @return BrushType Current brush type
     */
    BrushType getCurrentBrushType() const;
    
    /**
     * @brief Get the current brush shape
     * 
     * @return BrushShape Current brush shape
     */
    BrushShape getCurrentBrushShape() const;
    
    /**
     * @brief Get the current brush size
     * 
     * @return int Current brush size
     */
    int getCurrentBrushSize() const;
    
    /**
     * @brief Get the current brush settings
     * 
     * @return BrushSettings Current brush settings
     */
    BrushSettings getCurrentBrushSettings() const;
    
    /**
     * @brief Check if the current brush is enabled
     * 
     * @return true If the current brush is enabled
     * @return false If the current brush is disabled
     */
    bool isBrushEnabled() const;

public slots:
    /**
     * @brief Set the current brush type
     * 
     * @param type New brush type
     */
    void setCurrentBrushType(BrushType type);
    
    /**
     * @brief Set the current brush shape
     * 
     * @param shape New brush shape
     */
    void setCurrentBrushShape(BrushShape shape);
    
    /**
     * @brief Set the current brush size
     * 
     * @param size New brush size
     */
    void setCurrentBrushSize(int size);
    
    /**
     * @brief Set the current brush settings
     * 
     * @param settings New brush settings
     */
    void setCurrentBrushSettings(const BrushSettings& settings);
    
    /**
     * @brief Enable or disable the current brush
     * 
     * @param enabled Whether the brush should be enabled
     */
    void setBrushEnabled(bool enabled);

signals:
    /**
     * @brief Signal emitted when the current brush changes
     * 
     * @param brush Pointer to the new brush
     */
    void currentBrushChanged(Brush* brush);
    
    /**
     * @brief Signal emitted when the current brush type changes
     * 
     * @param type New brush type
     */
    void currentBrushTypeChanged(BrushType type);
    
    /**
     * @brief Signal emitted when the current brush shape changes
     * 
     * @param shape New brush shape
     */
    void currentBrushShapeChanged(BrushShape shape);
    
    /**
     * @brief Signal emitted when the current brush size changes
     * 
     * @param size New brush size
     */
    void currentBrushSizeChanged(int size);
    
    /**
     * @brief Signal emitted when the current brush settings change
     * 
     * @param settings New brush settings
     */
    void currentBrushSettingsChanged(const BrushSettings& settings);
    
    /**
     * @brief Signal emitted when the brush enabled state changes
     * 
     * @param enabled Whether the brush is enabled
     */
    void brushEnabledChanged(bool enabled);

private:
    BrushIntegrationManager* m_brushManager;
    BrushType m_currentBrushType = BrushType::None;
    BrushShape m_currentBrushShape = BrushShape::Square;
    int m_currentBrushSize = 1;
    BrushSettings m_currentBrushSettings;
    bool m_brushEnabled = true;
    
    // Additional state for IBrushStateService
    Brush* m_activeBrush = nullptr;
    int m_brushVariation = 0;
    bool m_drawLockedDoors = false;
    bool m_useCustomThickness = false;
    float m_customThicknessMod = 1.0f;
    uint32_t m_currentRawItemId = 0;
    const CreatureData* m_currentCreatureType = nullptr;
    BaseMap* m_doodadBufferMap = nullptr;
};

} // namespace brush
} // namespace core
} // namespace RME

#endif // RME_BRUSH_STATE_SERVICE_H