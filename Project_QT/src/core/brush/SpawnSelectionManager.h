#ifndef RME_SPAWN_SELECTION_MANAGER_H
#define RME_SPAWN_SELECTION_MANAGER_H

#include <QObject>
#include <QSet>
#include "core/Position.h"
#include "core/spawns/SpawnData.h"

// Forward declarations
namespace RME {
namespace core {
    namespace spawns { class SpawnManager; }
    namespace editor { class EditorControllerInterface; }
}
}

namespace RME {
namespace core {
namespace brush {

/**
 * @brief Manages spawn selection and highlighting for the map editor
 * 
 * This class provides functionality for selecting spawns, highlighting them
 * in the UI, and managing spawn-related operations like editing properties
 * and removing spawns.
 */
class SpawnSelectionManager : public QObject {
    Q_OBJECT

public:
    explicit SpawnSelectionManager(RME::core::spawns::SpawnManager* spawnManager,
                                  RME::core::editor::EditorControllerInterface* editorController,
                                  QObject* parent = nullptr);
    ~SpawnSelectionManager() override = default;

    // Selection management
    void selectSpawn(const RME::core::Position& spawnPosition);
    void deselectSpawn(const RME::core::Position& spawnPosition);
    void clearSelection();
    void selectMultipleSpawns(const QList<RME::core::Position>& spawnPositions);
    
    // Selection queries
    bool isSpawnSelected(const RME::core::Position& spawnPosition) const;
    QList<RME::core::Position> getSelectedSpawns() const;
    int getSelectionCount() const;
    bool hasSelection() const;
    
    // Spawn operations on selected spawns
    void deleteSelectedSpawns();
    void copySelectedSpawns();
    void moveSelectedSpawns(const RME::core::Position& offset);
    
    // Spawn property editing
    void editSpawnProperties(const RME::core::Position& spawnPosition);
    void setSpawnRadius(const RME::core::Position& spawnPosition, int radius);
    void addCreatureToSpawn(const RME::core::Position& spawnPosition, const QString& creatureName);
    void removeCreatureFromSpawn(const RME::core::Position& spawnPosition, const QString& creatureName);

public slots:
    // Slots for UI integration
    void onMapClick(const RME::core::Position& position, Qt::KeyboardModifiers modifiers);
    void onSpawnAdded(const RME::core::Position& position);
    void onSpawnRemoved(const RME::core::Position& position);

signals:
    // Signals for UI feedback
    void spawnSelected(const RME::core::Position& position);
    void spawnDeselected(const RME::core::Position& position);
    void selectionChanged();
    void selectionCleared();
    void spawnHighlightRequested(const RME::core::Position& position, bool highlight);

private:
    RME::core::spawns::SpawnManager* m_spawnManager;
    RME::core::editor::EditorControllerInterface* m_editorController;
    
    // Selection state
    QSet<RME::core::Position> m_selectedSpawns;
    
    // Helper methods
    void updateHighlighting();
    bool isValidSpawnPosition(const RME::core::Position& position) const;
};

} // namespace brush
} // namespace core
} // namespace RME

#endif // RME_SPAWN_SELECTION_MANAGER_H