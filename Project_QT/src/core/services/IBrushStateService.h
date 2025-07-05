#ifndef RME_IBRUSHSTATESERVICE_H
#define RME_IBRUSHSTATESERVICE_H

#include <QObject>
#include <cstdint>

namespace RME {
namespace core {

class Brush;
class CreatureData;
class BaseMap;

enum class BrushShape {
    Square,
    Circle,
    Custom
};

/**
 * @brief Interface for brush state management service
 * 
 * This interface defines the contract for managing brush state including
 * active brush, brush properties, and brush-specific data.
 */
class IBrushStateService : public QObject {
    Q_OBJECT

public:
    virtual ~IBrushStateService() = default;

    // Brush management
    virtual void setActiveBrush(Brush* brush) = 0;
    virtual Brush* getActiveBrush() const = 0;
    
    // Brush properties
    virtual void setBrushShape(BrushShape shape) = 0;
    virtual BrushShape getBrushShape() const = 0;
    
    virtual void setBrushSize(int size) = 0;
    virtual int getBrushSize() const = 0;
    
    virtual void setBrushVariation(int variation) = 0;
    virtual int getBrushVariation() const = 0;
    
    // Brush settings
    virtual void setDrawLockedDoors(bool enabled) = 0;
    virtual bool getDrawLockedDoors() const = 0;
    
    virtual void setUseCustomThickness(bool enabled) = 0;
    virtual bool getUseCustomThickness() const = 0;
    
    virtual void setCustomThicknessMod(float mod) = 0;
    virtual float getCustomThicknessMod() const = 0;
    
    // Specific brush data
    virtual void setCurrentRawItemId(uint32_t itemId) = 0;
    virtual uint32_t getCurrentRawItemId() const = 0;
    
    virtual void setCurrentCreatureType(const CreatureData* creature) = 0;
    virtual const CreatureData* getCurrentCreatureType() const = 0;
    
    // Doodad buffer
    virtual void setDoodadBufferMap(BaseMap* map) = 0;
    virtual BaseMap* getDoodadBufferMap() const = 0;

signals:
    void activeBrushChanged(Brush* brush);
    void brushShapeChanged(BrushShape shape);
    void brushSizeChanged(int size);
    void brushVariationChanged(int variation);
    void brushSettingsChanged();
    void drawLockedDoorsChanged(bool enabled);
    void customThicknessChanged(bool enabled, float mod);
    void currentRawItemIdChanged(uint32_t itemId);
    void currentCreatureTypeChanged(const CreatureData* creature);
    void doodadBufferMapChanged(BaseMap* map);
};

} // namespace core
} // namespace RME

#endif // RME_IBRUSHSTATESERVICE_H