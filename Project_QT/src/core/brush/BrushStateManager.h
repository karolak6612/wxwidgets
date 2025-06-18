#ifndef RME_BRUSH_STATE_MANAGER_H
#define RME_BRUSH_STATE_MANAGER_H

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QHash>
#include <memory>

// Forward declarations
namespace RME {
namespace core {
    namespace brush { class Brush; }
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace core {
namespace brush {

/**
 * @brief Manages brush state and activation for the map editor
 * 
 * This class provides a centralized way to manage which brush is currently
 * active and handles the state transitions between different brushes and tools.
 * It works in conjunction with BrushIntegrationManager to provide complete
 * brush management functionality.
 */
class BrushStateManager : public QObject {
    Q_OBJECT

public:
    explicit BrushStateManager(QObject* parent = nullptr);
    ~BrushStateManager() override = default;

    // Brush registration and management
    void registerBrush(const QString& brushId, RME::core::brush::Brush* brush);
    void unregisterBrush(const QString& brushId);
    
    // Active brush management
    void setActiveBrush(const QString& brushId);
    void setActiveBrush(RME::core::brush::Brush* brush);
    RME::core::brush::Brush* getActiveBrush() const;
    QString getActiveBrushId() const;
    
    // Brush queries
    RME::core::brush::Brush* getBrush(const QString& brushId) const;
    QStringList getRegisteredBrushIds() const;
    bool hasBrush(const QString& brushId) const;
    
    // State management
    void saveCurrentState();
    void restorePreviousState();
    void clearState();

public slots:
    // Slots for external state changes
    void onBrushActivated(RME::core::brush::Brush* brush);
    void onToolModeChanged(int toolMode);

signals:
    // Signals for state changes
    void activeBrushChanged(RME::core::brush::Brush* brush, const QString& brushId);
    void brushRegistered(const QString& brushId, RME::core::brush::Brush* brush);
    void brushUnregistered(const QString& brushId);
    void stateChanged();

private:
    // Brush registry
    QHash<QString, RME::core::brush::Brush*> m_brushRegistry;
    
    // Current state
    RME::core::brush::Brush* m_activeBrush = nullptr;
    QString m_activeBrushId;
    
    // Previous state for restoration
    RME::core::brush::Brush* m_previousBrush = nullptr;
    QString m_previousBrushId;
    
    // Helper methods
    QString generateBrushId(RME::core::brush::Brush* brush) const;
    void updateActiveBrush(RME::core::brush::Brush* brush, const QString& brushId);
};

} // namespace brush
} // namespace core
} // namespace RME

#endif // RME_BRUSH_STATE_MANAGER_H