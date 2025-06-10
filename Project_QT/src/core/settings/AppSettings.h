#ifndef RME_APP_SETTINGS_H
#define RME_APP_SETTINGS_H

#include <QSettings>
#include <QString>
#include <QVariant>
#include <QReadWriteLock>
#include <memory> // For std::unique_ptr

// Forward declare QCoreApplication for org/app name (though not used directly in this header)
// class QCoreApplication; // QSettings constructor might need it, but AppSettings takes strings

namespace RME {
namespace Config {

enum Key {
    // --- From View group ---
    TRANSPARENT_FLOORS,      // int (bool)
    SHOW_GRID,               // int (bool)
    // --- From Version group ---
    DATA_DIRECTORY,          // string
    // --- From Editor group ---
    SCROLL_SPEED,            // float
    UNDO_SIZE,               // int
    // --- From Graphics group ---
    TEXTURE_MANAGEMENT,      // int (bool)
    // --- From UI group ---
    PALETTE_COL_COUNT,       // int
    // --- From Network group ---
    LIVE_HOST,               // string
    LIVE_PORT,               // int

    LAST_KEY_PLACEHOLDER
};

} // namespace Config

class AppSettings {
public:
    AppSettings(QSettings::Format format = QSettings::IniFormat,
                QSettings::Scope scope = QSettings::UserScope,
                const QString& organization = "RMEditor_TestOrg",
                const QString& application = "RME-Qt_TestApp");

    ~AppSettings();

    QVariant getValue(Config::Key key, const QVariant& defaultValue = QVariant()) const;
    void setValue(Config::Key key, const QVariant& value);

    // Typed Getters
    bool isTransparentFloorsEnabled() const;
    bool isShowGridEnabled() const;
    QString getDataDirectory() const;
    float getScrollSpeed() const;
    int getUndoSize() const;
    bool isTextureManagementEnabled() const;
    int getPaletteColCount() const;
    QString getLiveHost() const;
    int getLivePort() const;

    // Typed Setters
    void setTransparentFloorsEnabled(bool enabled);
    void setShowGridEnabled(bool enabled);
    void setDataDirectory(const QString& dir);
    void setScrollSpeed(float speed);
    void setUndoSize(int size);
    void setTextureManagementEnabled(bool enabled);
    void setPaletteColCount(int count);
    void setLiveHost(const QString& host);
    void setLivePort(int port);

    static QString getKeyString(Config::Key key); // Made public for potential use in UI or tests

private:
    std::unique_ptr<QSettings> settings;
    // mutable QReadWriteLock lock; // Uncomment if singleton and thread-safety needed
};

} // namespace RME

#endif // RME_APP_SETTINGS_H
