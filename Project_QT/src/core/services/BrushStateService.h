#ifndef RME_BRUSHSTATESERVICE_H
#define RME_BRUSHSTATESERVICE_H

#include "IBrushStateService.h"

namespace RME {
namespace core {

class Brush;
class CreatureData;
class BaseMap;

/**
 * @brief Concrete implementation of IBrushStateService
 * 
 * This class manages the state of brushes including active brush,
 * brush properties, and brush-specific settings.
 */
class BrushStateService : public IBrushStateService {
    Q_OBJECT

public:
    explicit BrushStateService(QObject* parent = nullptr);
    ~BrushStateService() override;

    // Brush management
    void setActiveBrush(Brush* brush) override;
    Brush* getActiveBrush() const override;
    
    // Brush properties
    void setBrushShape(BrushShape shape) override;
    BrushShape getBrushShape() const override;
    
    void setBrushSize(int size) override;
    int getBrushSize() const override;
    
    void setBrushVariation(int variation) override;
    int getBrushVariation() const override;
    
    // Brush settings
    void setDrawLockedDoors(bool enabled) override;
    bool getDrawLockedDoors() const override;
    
    void setUseCustomThickness(bool enabled) override;
    bool getUseCustomThickness() const override;
    
    void setCustomThicknessMod(float mod) override;
    float getCustomThicknessMod() const override;
    
    // Specific brush data
    void setCurrentRawItemId(uint32_t itemId) override;
    uint32_t getCurrentRawItemId() const override;
    
    void setCurrentCreatureType(const CreatureData* creature) override;
    const CreatureData* getCurrentCreatureType() const override;
    
    // Doodad buffer
    void setDoodadBufferMap(BaseMap* map) override;
    BaseMap* getDoodadBufferMap() const override;

private:
    Brush* m_activeBrush;
    BrushShape m_brushShape;
    int m_brushSize;
    int m_brushVariation;
    bool m_drawLockedDoors;
    bool m_useCustomThickness;
    float m_customThicknessMod;
    uint32_t m_currentRawItemId;
    const CreatureData* m_currentCreatureType;
    BaseMap* m_doodadBufferMap;
};

} // namespace core
} // namespace RME

#endif // RME_BRUSHSTATESERVICE_H