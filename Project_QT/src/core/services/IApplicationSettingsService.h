#ifndef RME_IAPPLICATIONSETTINGSSERVICE_H
#define RME_IAPPLICATIONSETTINGSSERVICE_H

#include <QObject>
#include <QString>

namespace RME {
namespace core {

enum class BrushShape;

/**
 * @brief Interface for application settings management service
 * 
 * This interface defines the contract for managing application settings
 * including UI-related settings, view settings, and brush settings.
 */
class IApplicationSettingsService : public QObject {
    Q_OBJECT

public:
    virtual ~IApplicationSettingsService() = default;

    // UI-related settings
    virtual bool isDoorLocked() const = 0;
    virtual void setDoorLocked(bool locked) = 0;
    
    virtual bool isPasting() const = 0;
    virtual void setPasting(bool pasting) = 0;
    
    virtual bool isAutoSaveEnabled() const = 0;
    virtual void setAutoSaveEnabled(bool enabled) = 0;
    
    virtual int getAutoSaveInterval() const = 0;
    virtual void setAutoSaveInterval(int minutes) = 0;
    
    // View settings
    virtual bool isGridVisible() const = 0;
    virtual void setGridVisible(bool visible) = 0;
    
    virtual bool areCreaturesVisible() const = 0;
    virtual void setCreaturesVisible(bool visible) = 0;
    
    virtual bool areSpawnsVisible() const = 0;
    virtual void setSpawnsVisible(bool visible) = 0;
    
    virtual bool areHousesVisible() const = 0;
    virtual void setHousesVisible(bool visible) = 0;
    
    virtual bool areWaypointsVisible() const = 0;
    virtual void setWaypointsVisible(bool visible) = 0;
    
    // Brush settings
    virtual int getDefaultBrushSize() const = 0;
    virtual void setDefaultBrushSize(int size) = 0;
    
    virtual BrushShape getDefaultBrushShape() const = 0;
    virtual void setDefaultBrushShape(BrushShape shape) = 0;
    
    // Application behavior settings
    virtual bool isUndoRedoEnabled() const = 0;
    virtual void setUndoRedoEnabled(bool enabled) = 0;
    
    virtual int getUndoRedoLimit() const = 0;
    virtual void setUndoRedoLimit(int limit) = 0;
    
    // File handling settings
    virtual QString getDefaultMapPath() const = 0;
    virtual void setDefaultMapPath(const QString& path) = 0;
    
    virtual QString getDefaultClientPath() const = 0;
    virtual void setDefaultClientPath(const QString& path) = 0;

signals:
    void doorLockedChanged(bool locked);
    void pastingChanged(bool pasting);
    void autoSaveSettingsChanged(bool enabled, int interval);
    void viewSettingsChanged();
    void brushSettingsChanged();
    void undoRedoSettingsChanged(bool enabled, int limit);
    void pathSettingsChanged();
};

} // namespace core
} // namespace RME

#endif // RME_IAPPLICATIONSETTINGSSERVICE_H