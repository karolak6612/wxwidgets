#include "AppSettings.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMap> // Required for QMap

namespace RME {

// Static map for Key enum to string representation and group.
struct SettingDetail {
    QString qKey; // QSettings key string (e.g., "View/TRANSPARENT_FLOORS")
    QVariant defaultValue;
};

// Using a static function to initialize the map to avoid static initialization order issues
// if this map were a global static variable.
static const QMap<Config::Key, SettingDetail>& getKeyDetailsMap() {
    static const QMap<Config::Key, SettingDetail> keyDetailsMapInstance = {
        { Config::TRANSPARENT_FLOORS,   {"View/TRANSPARENT_FLOORS", QVariant(false)} },
        { Config::SHOW_GRID,            {"View/SHOW_GRID", QVariant(false)} },
        { Config::DATA_DIRECTORY,       {"Version/DATA_DIRECTORY", QVariant("")} },
        { Config::SCROLL_SPEED,         {"Editor/SCROLL_SPEED", QVariant(3.5f)} },
        { Config::UNDO_SIZE,            {"Editor/UNDO_SIZE", QVariant(40)} },
        { Config::TEXTURE_MANAGEMENT,   {"Graphics/TEXTURE_MANAGEMENT", QVariant(true)} },
        { Config::PALETTE_COL_COUNT,    {"UI/PALETTE_COL_COUNT", QVariant(8)} },
        { Config::LIVE_HOST,            {"Network/LIVE_HOST", QVariant("localhost")} },
        { Config::LIVE_PORT,            {"Network/LIVE_PORT", QVariant(12356)} }
    };
    return keyDetailsMapInstance;
}


QString AppSettings::getKeyString(Config::Key key) {
    const auto& map = getKeyDetailsMap();
    if (map.contains(key)) {
        return map[key].qKey;
    }
    qWarning() << "AppSettings::getKeyString - Unknown Config::Key:" << static_cast<int>(key);
    return QString("Unknown/KEY_%1").arg(static_cast<int>(key));
}

QVariant AppSettings::getValue(Config::Key key, const QVariant& defaultValue) const {
    if (!settings) { // Should not happen if constructor is robust
        qWarning() << "AppSettings::getValue - QSettings object is null.";
        return defaultValue.isValid() ? defaultValue : QVariant();
    }

    const auto& map = getKeyDetailsMap();
    if (map.contains(key)) {
        // Use provided defaultValue if valid, otherwise use map's default
        return settings->value(map[key].qKey, defaultValue.isValid() ? defaultValue : map[key].defaultValue);
    } else {
         qWarning() << "AppSettings::getValue - Unknown key:" << static_cast<int>(key);
         return defaultValue.isValid() ? defaultValue : QVariant();
    }
}

void AppSettings::setValue(Config::Key key, const QVariant& value) {
    if (!settings) {
        qWarning() << "AppSettings::setValue - QSettings object is null.";
        return;
    }
    const auto& map = getKeyDetailsMap();
    if (map.contains(key)) {
         settings->setValue(map[key].qKey, value);
    } else {
        qWarning() << "AppSettings::setValue - Attempted to set unknown key:" << static_cast<int>(key);
    }
}

AppSettings::AppSettings(QSettings::Format format, QSettings::Scope scope,
                         const QString& organization, const QString& application) {
    if (!organization.isEmpty() && !application.isEmpty()) {
        settings = std::make_unique<QSettings>(format, scope, organization, application);
    } else {
        // This case is primarily for when the application uses QCoreApplication organization/name.
        // For tests, it's better to provide explicit org/app names to avoid relying on global state.
        if (QCoreApplication::organizationName().isEmpty() || QCoreApplication::applicationName().isEmpty()) {
            qWarning("AppSettings: QCoreApplication organizationName or applicationName is not set. "
                     "Using provided test defaults or potentially unstable default QSettings path. "
                     "Provide explicit organization and application names to constructor for stable testing.");
            // Fallback to ensure settings object is created, using test defaults if they were also empty.
            QString orgToUse = organization.isEmpty() ? "RMEditor_DefaultOrg" : organization;
            QString appToUse = application.isEmpty() ? "RME-Qt_DefaultApp" : application;
            settings = std::make_unique<QSettings>(format, scope, orgToUse, appToUse);
        } else {
             settings = std::make_unique<QSettings>(); // Uses QCoreApplication settings
        }
    }
    if(settings) {
        qInfo() << "AppSettings: Initialized. Using settings file:" << settings->fileName();
    } else {
        qCritical("AppSettings: Failed to initialize QSettings object!");
    }
}

AppSettings::~AppSettings() {
    if (settings) {
        settings->sync();
    }
}

// --- Typed Getters (Subset) ---
bool AppSettings::isTransparentFloorsEnabled() const { return getValue(Config::TRANSPARENT_FLOORS).toBool(); }
bool AppSettings::isShowGridEnabled() const { return getValue(Config::SHOW_GRID).toBool(); }
QString AppSettings::getDataDirectory() const { return getValue(Config::DATA_DIRECTORY).toString(); }
float AppSettings::getScrollSpeed() const { return getValue(Config::SCROLL_SPEED).toFloat(); }
int AppSettings::getUndoSize() const { return getValue(Config::UNDO_SIZE).toInt(); }
bool AppSettings::isTextureManagementEnabled() const { return getValue(Config::TEXTURE_MANAGEMENT).toBool(); }
int AppSettings::getPaletteColCount() const { return getValue(Config::PALETTE_COL_COUNT).toInt(); }
QString AppSettings::getLiveHost() const { return getValue(Config::LIVE_HOST).toString(); }
int AppSettings::getLivePort() const { return getValue(Config::LIVE_PORT).toInt(); }

// --- Typed Setters (Subset) ---
void AppSettings::setTransparentFloorsEnabled(bool enabled) { setValue(Config::TRANSPARENT_FLOORS, enabled); }
void AppSettings::setShowGridEnabled(bool enabled) { setValue(Config::SHOW_GRID, enabled); }
void AppSettings::setDataDirectory(const QString& dir) { setValue(Config::DATA_DIRECTORY, dir); }
void AppSettings::setScrollSpeed(float speed) { setValue(Config::SCROLL_SPEED, speed); }
void AppSettings::setUndoSize(int size) { setValue(Config::UNDO_SIZE, size); }
void AppSettings::setTextureManagementEnabled(bool enabled) { setValue(Config::TEXTURE_MANAGEMENT, enabled); }
void AppSettings::setPaletteColCount(int count) { setValue(Config::PALETTE_COL_COUNT, count); }
void AppSettings::setLiveHost(const QString& host) { setValue(Config::LIVE_HOST, host); }
void AppSettings::setLivePort(int port) { setValue(Config::LIVE_PORT, port); }

} // namespace RME
