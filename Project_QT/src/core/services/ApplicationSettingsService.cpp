#include "ApplicationSettingsService.h"
#include "core/brush/BrushShape.h"
#include <QStandardPaths>
#include <QDebug>

namespace RME {
namespace core {

ApplicationSettingsService::ApplicationSettingsService(QObject* parent)
    : IApplicationSettingsService(parent)
    , m_settings(nullptr)
{
    m_settings = new QSettings("RME", "RemereMapEditor", this);
    initializeDefaults();
    loadSettings();
}

ApplicationSettingsService::~ApplicationSettingsService()
{
    saveSettings();
}

void ApplicationSettingsService::initializeDefaults()
{
    // UI-related settings defaults
    m_doorLocked = false;
    m_pasting = false;
    m_autoSaveEnabled = true;
    m_autoSaveInterval = 5; // 5 minutes
    
    // View settings defaults
    m_gridVisible = true;
    m_creaturesVisible = true;
    m_spawnsVisible = true;
    m_housesVisible = true;
    m_waypointsVisible = true;
    
    // Brush settings defaults
    m_defaultBrushSize = 1;
    m_defaultBrushShape = BrushShape::Square;
    
    // Application behavior defaults
    m_undoRedoEnabled = true;
    m_undoRedoLimit = 50;
    
    // File handling defaults
    m_defaultMapPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    m_defaultClientPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

void ApplicationSettingsService::loadSettings()
{
    if (!m_settings) {
        return;
    }
    
    // Load UI-related settings
    m_doorLocked = m_settings->value("ui/doorLocked", m_doorLocked).toBool();
    m_pasting = m_settings->value("ui/pasting", m_pasting).toBool();
    m_autoSaveEnabled = m_settings->value("ui/autoSaveEnabled", m_autoSaveEnabled).toBool();
    m_autoSaveInterval = m_settings->value("ui/autoSaveInterval", m_autoSaveInterval).toInt();
    
    // Load view settings
    m_gridVisible = m_settings->value("view/gridVisible", m_gridVisible).toBool();
    m_creaturesVisible = m_settings->value("view/creaturesVisible", m_creaturesVisible).toBool();
    m_spawnsVisible = m_settings->value("view/spawnsVisible", m_spawnsVisible).toBool();
    m_housesVisible = m_settings->value("view/housesVisible", m_housesVisible).toBool();
    m_waypointsVisible = m_settings->value("view/waypointsVisible", m_waypointsVisible).toBool();
    
    // Load brush settings
    m_defaultBrushSize = m_settings->value("brush/defaultSize", m_defaultBrushSize).toInt();
    int shapeValue = m_settings->value("brush/defaultShape", static_cast<int>(m_defaultBrushShape)).toInt();
    m_defaultBrushShape = static_cast<BrushShape>(shapeValue);
    
    // Load application behavior settings
    m_undoRedoEnabled = m_settings->value("app/undoRedoEnabled", m_undoRedoEnabled).toBool();
    m_undoRedoLimit = m_settings->value("app/undoRedoLimit", m_undoRedoLimit).toInt();
    
    // Load file handling settings
    m_defaultMapPath = m_settings->value("paths/defaultMapPath", m_defaultMapPath).toString();
    m_defaultClientPath = m_settings->value("paths/defaultClientPath", m_defaultClientPath).toString();
    
    qDebug() << "ApplicationSettingsService: Settings loaded";
}

void ApplicationSettingsService::saveSettings()
{
    if (!m_settings) {
        return;
    }
    
    // Save UI-related settings
    m_settings->setValue("ui/doorLocked", m_doorLocked);
    m_settings->setValue("ui/pasting", m_pasting);
    m_settings->setValue("ui/autoSaveEnabled", m_autoSaveEnabled);
    m_settings->setValue("ui/autoSaveInterval", m_autoSaveInterval);
    
    // Save view settings
    m_settings->setValue("view/gridVisible", m_gridVisible);
    m_settings->setValue("view/creaturesVisible", m_creaturesVisible);
    m_settings->setValue("view/spawnsVisible", m_spawnsVisible);
    m_settings->setValue("view/housesVisible", m_housesVisible);
    m_settings->setValue("view/waypointsVisible", m_waypointsVisible);
    
    // Save brush settings
    m_settings->setValue("brush/defaultSize", m_defaultBrushSize);
    m_settings->setValue("brush/defaultShape", static_cast<int>(m_defaultBrushShape));
    
    // Save application behavior settings
    m_settings->setValue("app/undoRedoEnabled", m_undoRedoEnabled);
    m_settings->setValue("app/undoRedoLimit", m_undoRedoLimit);
    
    // Save file handling settings
    m_settings->setValue("paths/defaultMapPath", m_defaultMapPath);
    m_settings->setValue("paths/defaultClientPath", m_defaultClientPath);
    
    m_settings->sync();
    qDebug() << "ApplicationSettingsService: Settings saved";
}

void ApplicationSettingsService::resetToDefaults()
{
    initializeDefaults();
    saveSettings();
    
    // Emit all change signals
    emit doorLockedChanged(m_doorLocked);
    emit pastingChanged(m_pasting);
    emit autoSaveSettingsChanged(m_autoSaveEnabled, m_autoSaveInterval);
    emit viewSettingsChanged();
    emit brushSettingsChanged();
    emit undoRedoSettingsChanged(m_undoRedoEnabled, m_undoRedoLimit);
    emit pathSettingsChanged();
    
    qDebug() << "ApplicationSettingsService: Settings reset to defaults";
}

// IApplicationSettingsService implementation
bool ApplicationSettingsService::isDoorLocked() const
{
    return m_doorLocked;
}

void ApplicationSettingsService::setDoorLocked(bool locked)
{
    if (m_doorLocked != locked) {
        m_doorLocked = locked;
        emit doorLockedChanged(locked);
    }
}

bool ApplicationSettingsService::isPasting() const
{
    return m_pasting;
}

void ApplicationSettingsService::setPasting(bool pasting)
{
    if (m_pasting != pasting) {
        m_pasting = pasting;
        emit pastingChanged(pasting);
    }
}

bool ApplicationSettingsService::isAutoSaveEnabled() const
{
    return m_autoSaveEnabled;
}

void ApplicationSettingsService::setAutoSaveEnabled(bool enabled)
{
    if (m_autoSaveEnabled != enabled) {
        m_autoSaveEnabled = enabled;
        emit autoSaveSettingsChanged(enabled, m_autoSaveInterval);
    }
}

int ApplicationSettingsService::getAutoSaveInterval() const
{
    return m_autoSaveInterval;
}

void ApplicationSettingsService::setAutoSaveInterval(int minutes)
{
    if (m_autoSaveInterval != minutes) {
        m_autoSaveInterval = minutes;
        emit autoSaveSettingsChanged(m_autoSaveEnabled, minutes);
    }
}

bool ApplicationSettingsService::isGridVisible() const
{
    return m_gridVisible;
}

void ApplicationSettingsService::setGridVisible(bool visible)
{
    if (m_gridVisible != visible) {
        m_gridVisible = visible;
        emit viewSettingsChanged();
    }
}

bool ApplicationSettingsService::areCreaturesVisible() const
{
    return m_creaturesVisible;
}

void ApplicationSettingsService::setCreaturesVisible(bool visible)
{
    if (m_creaturesVisible != visible) {
        m_creaturesVisible = visible;
        emit viewSettingsChanged();
    }
}

bool ApplicationSettingsService::areSpawnsVisible() const
{
    return m_spawnsVisible;
}

void ApplicationSettingsService::setSpawnsVisible(bool visible)
{
    if (m_spawnsVisible != visible) {
        m_spawnsVisible = visible;
        emit viewSettingsChanged();
    }
}

bool ApplicationSettingsService::areHousesVisible() const
{
    return m_housesVisible;
}

void ApplicationSettingsService::setHousesVisible(bool visible)
{
    if (m_housesVisible != visible) {
        m_housesVisible = visible;
        emit viewSettingsChanged();
    }
}

bool ApplicationSettingsService::areWaypointsVisible() const
{
    return m_waypointsVisible;
}

void ApplicationSettingsService::setWaypointsVisible(bool visible)
{
    if (m_waypointsVisible != visible) {
        m_waypointsVisible = visible;
        emit viewSettingsChanged();
    }
}

int ApplicationSettingsService::getDefaultBrushSize() const
{
    return m_defaultBrushSize;
}

void ApplicationSettingsService::setDefaultBrushSize(int size)
{
    if (m_defaultBrushSize != size) {
        m_defaultBrushSize = size;
        emit brushSettingsChanged();
    }
}

BrushShape ApplicationSettingsService::getDefaultBrushShape() const
{
    return m_defaultBrushShape;
}

void ApplicationSettingsService::setDefaultBrushShape(BrushShape shape)
{
    if (m_defaultBrushShape != shape) {
        m_defaultBrushShape = shape;
        emit brushSettingsChanged();
    }
}

bool ApplicationSettingsService::isUndoRedoEnabled() const
{
    return m_undoRedoEnabled;
}

void ApplicationSettingsService::setUndoRedoEnabled(bool enabled)
{
    if (m_undoRedoEnabled != enabled) {
        m_undoRedoEnabled = enabled;
        emit undoRedoSettingsChanged(enabled, m_undoRedoLimit);
    }
}

int ApplicationSettingsService::getUndoRedoLimit() const
{
    return m_undoRedoLimit;
}

void ApplicationSettingsService::setUndoRedoLimit(int limit)
{
    if (m_undoRedoLimit != limit) {
        m_undoRedoLimit = limit;
        emit undoRedoSettingsChanged(m_undoRedoEnabled, limit);
    }
}

QString ApplicationSettingsService::getDefaultMapPath() const
{
    return m_defaultMapPath;
}

void ApplicationSettingsService::setDefaultMapPath(const QString& path)
{
    if (m_defaultMapPath != path) {
        m_defaultMapPath = path;
        emit pathSettingsChanged();
    }
}

QString ApplicationSettingsService::getDefaultClientPath() const
{
    return m_defaultClientPath;
}

void ApplicationSettingsService::setDefaultClientPath(const QString& path)
{
    if (m_defaultClientPath != path) {
        m_defaultClientPath = path;
        emit pathSettingsChanged();
    }
}

} // namespace core
} // namespace RME