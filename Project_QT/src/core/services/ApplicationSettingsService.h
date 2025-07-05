#ifndef RME_APPLICATIONSETTINGSSERVICE_H
#define RME_APPLICATIONSETTINGSSERVICE_H

#include "IApplicationSettingsService.h"
#include <QObject>
#include <QSettings>
#include <QString>

namespace RME {
namespace core {

enum class BrushShape;

/**
 * @brief Service for managing application settings
 * 
 * This service centralizes all application settings management and provides
 * a unified interface for accessing and modifying settings with proper
 * change notifications.
 */
class ApplicationSettingsService : public IApplicationSettingsService
{
    Q_OBJECT

public:
    explicit ApplicationSettingsService(QObject* parent = nullptr);
    ~ApplicationSettingsService();

    // IApplicationSettingsService implementation
    bool isDoorLocked() const override;
    void setDoorLocked(bool locked) override;
    
    bool isPasting() const override;
    void setPasting(bool pasting) override;
    
    bool isAutoSaveEnabled() const override;
    void setAutoSaveEnabled(bool enabled) override;
    
    int getAutoSaveInterval() const override;
    void setAutoSaveInterval(int minutes) override;
    
    bool isGridVisible() const override;
    void setGridVisible(bool visible) override;
    
    bool areCreaturesVisible() const override;
    void setCreaturesVisible(bool visible) override;
    
    bool areSpawnsVisible() const override;
    void setSpawnsVisible(bool visible) override;
    
    bool areHousesVisible() const override;
    void setHousesVisible(bool visible) override;
    
    bool areWaypointsVisible() const override;
    void setWaypointsVisible(bool visible) override;
    
    int getDefaultBrushSize() const override;
    void setDefaultBrushSize(int size) override;
    
    BrushShape getDefaultBrushShape() const override;
    void setDefaultBrushShape(BrushShape shape) override;
    
    bool isUndoRedoEnabled() const override;
    void setUndoRedoEnabled(bool enabled) override;
    
    int getUndoRedoLimit() const override;
    void setUndoRedoLimit(int limit) override;
    
    QString getDefaultMapPath() const override;
    void setDefaultMapPath(const QString& path) override;
    
    QString getDefaultClientPath() const override;
    void setDefaultClientPath(const QString& path) override;

    // Additional utility methods
    void loadSettings();
    void saveSettings();
    void resetToDefaults();

private:
    void initializeDefaults();
    void connectToSettings();

private:
    QSettings* m_settings;
    
    // Cached values for performance
    bool m_doorLocked;
    bool m_pasting;
    bool m_autoSaveEnabled;
    int m_autoSaveInterval;
    bool m_gridVisible;
    bool m_creaturesVisible;
    bool m_spawnsVisible;
    bool m_housesVisible;
    bool m_waypointsVisible;
    int m_defaultBrushSize;
    BrushShape m_defaultBrushShape;
    bool m_undoRedoEnabled;
    int m_undoRedoLimit;
    QString m_defaultMapPath;
    QString m_defaultClientPath;
};

} // namespace core
} // namespace RME

#endif // RME_APPLICATIONSETTINGSSERVICE_H