#ifndef RME_EDITOR_CONTROLLER_H
#define RME_EDITOR_CONTROLLER_H

#include "core/editor/EditorControllerInterface.h" // Base interface
#include "core/Position.h"
#include <QList>
#include <QString>
#include <memory> // For std::unique_ptr
#include <QtGlobal> // For Qt::KeyboardModifiers

// Service interfaces
#include "core/services/IBrushStateService.h"
#include "core/services/IEditorStateService.h"
#include "core/services/IClientDataService.h"
#include "core/services/IApplicationSettingsService.h"

// Forward declarations
class QUndoStack;
class QUndoCommand;

namespace RME {
namespace core {
    class Map;
    class BrushSettings;
    // class Item; // Not directly used in method signatures here
    namespace brush { class BrushManager; }
    namespace selection { class SelectionManager; }
    namespace settings { class AppSettings; }
    namespace assets { class AssetManager; }
    namespace houses { class Houses; } // Added Forward declaration
    namespace clipboard { class ClipboardManager; } // Added for LOGIC-03
    namespace waypoints { class WaypointManager; } // Added for LOGIC-04
    class Tile; // For getTileForEditing
    class SpawnData; // For recordAddSpawn etc.
    struct CreatureData; // For recordAddCreature etc. (often a struct in assets namespace)
    namespace actions { class AppUndoCommand; } // For recordAction
}
}

namespace RME {
namespace editor_logic {

class EditorController : public QObject, public RME::core::editor::EditorControllerInterface {
    Q_OBJECT
public:
    EditorController(
        RME::core::Map* map,
        RME::core::IBrushStateService* brushStateService,
        RME::core::IEditorStateService* editorStateService,
        RME::core::IClientDataService* clientDataService,
        RME::core::IApplicationSettingsService* settingsService,
        QObject* parent = nullptr
    );
    ~EditorController() override = default;

    // --- Core editing operations defined in this class ---
    void applyBrushStroke(const QList<RME::core::Position>& positions, const RME::core::BrushSettings& settings);
    void deleteSelection();
    void clearCurrentSelection();
    void performBoundingBoxSelection(const RME::core::Position& p1, const RME::core::Position& p2, Qt::KeyboardModifiers modifiers, const RME::core::BrushSettings& currentBrushSettings);
    void handleDeleteSelection();
    
    // Map interaction handling (LOGIC-06)
    void handleMapClick(const RME::core::Position& pos, Qt::MouseButton button, Qt::KeyboardModifiers modifiers, const RME::core::BrushSettings& settings);
    void handleMapDrag(const QList<RME::core::Position>& positions, Qt::MouseButton button, Qt::KeyboardModifiers modifiers, const RME::core::BrushSettings& settings);
    void handleMapRelease(const RME::core::Position& pos, Qt::MouseButton button, Qt::KeyboardModifiers modifiers, const RME::core::BrushSettings& settings);

    // House-specific operations
    void setHouseExit(quint32 houseId, const RME::core::Position& exitPos);
    quint32 createHouse(const QString& name, const RME::core::Position& entryPoint);
    quint32 createHouse(const QString& name, const RME::core::Position& entryPoint, 
                       quint32 townId, quint32 rent, bool isGuildhall);
    void removeHouse(quint32 houseId);
    void assignTileToHouse(const RME::core::Position& pos, quint32 houseId);
    void removeHouseAssignment(const RME::core::Position& pos);
    void setHouseProperty(quint32 houseId, const QString& property, const QVariant& value);
    void setHouseProperties(quint32 houseId, const QHash<QString, QVariant>& properties);
    
    // Clipboard operations (LOGIC-03)
    void copySelection();
    void cutSelection();
    void pasteAtPosition(const RME::core::Position& pos);
    void moveSelection(const RME::core::Position& offset);
    
    // Waypoint operations (LOGIC-04)
    void placeOrMoveWaypoint(const QString& name, const RME::core::Position& targetPos);
    void removeWaypoint(const QString& name);
    void renameWaypoint(const QString& oldName, const QString& newName);
    void navigateToWaypoint(const QString& name);
    
    // House operations (LOGIC-05)
    quint32 createHouse(const QString& name, const RME::core::Position& entryPoint);
    void assignTileToHouse(const RME::core::Position& pos, quint32 houseId);
    void removeHouseAssignment(const RME::core::Position& pos);
    void setHouseProperty(quint32 houseId, const QString& property, const QVariant& value);
    void removeHouse(quint32 houseId);
    
    // Brush and tool management (LOGIC-06)
    enum class ToolMode {
        Brush,          // Normal brush drawing
        HouseExit,      // House exit placement tool
        Waypoint        // Waypoint placement tool
    };
    
    void setToolMode(ToolMode mode);
    ToolMode getToolMode() const;
    void setCurrentHouseForTools(quint32 houseId);
    void setCurrentWaypointForTools(const QString& waypointName);
    
    // Map-wide operations (LOGIC-09)
    void borderizeMap(bool showProgressDialog = true);
    void randomizeMap(bool showProgressDialog = true);
    void clearInvalidHouseTiles(bool showProgressDialog = true);
    void clearModifiedTileState(bool showProgressDialog = true);
    quint32 validateGrounds(bool validateStack = true, bool generateEmpty = true, bool removeDuplicates = true);
    
    // Selection-based operations (LOGIC-09)
    void borderizeSelection();
    void randomizeSelection();
    void moveSelection(const RME::core::Position& offset);
    void destroySelection();
    
    // File operations (FINAL-01)
    bool newMap(int width = 1024, int height = 1024, const QString& name = "Untitled");
    bool loadMap(const QString& filename);
    bool saveMap(const QString& filename = QString());
    bool saveMapAs(const QString& filename);
    bool closeMap();
    
    // Map properties
    QString getCurrentMapFilename() const;
    bool isMapModified() const;
    void setMapModified(bool modified = true);
    
    // Import/Export operations (LOGIC-09)
    bool importMap(const QString& filename, const RME::core::Position& offset = RME::core::Position(0, 0, 0));
    bool exportMiniMap(const QString& filename, int floor = -1, bool showDialog = true);
    bool exportSelectionAsMiniMap(const QString& filename);
    
    // Ground validation operations (LOGIC-09)
    quint32 validateGroundStacks();
    quint32 generateEmptySurroundedGrounds();
    quint32 removeDuplicateGrounds();

    // --- Implementation of EditorControllerInterface ---
    // Basic accessors
    RME::core::houses::Houses* getHousesManager() override; // Added
    const RME::core::houses::Houses* getHousesManager() const override; // Added
    RME::core::waypoints::WaypointManager* getWaypointManager() override; // Added for LOGIC-04
    const RME::core::waypoints::WaypointManager* getWaypointManager() const override; // Added for LOGIC-04
    RME::core::Map* getMap() override;
    const RME::core::Map* getMap() const override;
    QUndoStack* getUndoStack() override;
    RME::core::assets::AssetManager* getAssetManager() override;
    const RME::core::assets::AssetManager* getAssetManager() const override;
    RME::core::settings::AppSettings& getAppSettings() override; // Returns reference
    const RME::core::settings::AppSettings& getAppSettings() const override; // Returns const reference

    // Command handling
    void pushCommand(std::unique_ptr<QUndoCommand> command) override;

    // Tile/Notification related methods (likely from interface, used by brushes/commands)
    void notifyTileChanged(const RME::core::Position& pos) override;
    RME::core::Tile* getTileForEditing(const RME::core::Position& pos) override;

    // Record methods from EditorControllerInterface
    // These should be implemented, possibly by creating specific commands or a generic one.
    // For now, they are just declarations matching a plausible interface.
    void recordAction(std::unique_ptr<RME::core::actions::AppUndoCommand> command) override;
    void recordTileChange(const RME::core::Position& pos,
                          std::unique_ptr<RME::core::Tile> oldTileState,
                          std::unique_ptr<RME::core::Tile> newTileState) override;
    void recordAddCreature(const RME::core::Position& tilePos, const RME::core::CreatureData* creatureType) override;
    void recordRemoveCreature(const RME::core::Position& tilePos, const RME::core::CreatureData* creatureType) override;
    void recordAddSpawn(const RME::core::spawns::Spawn& spawn) override;
    void recordRemoveSpawn(const RME::core::Position& pos) override;
    void recordUpdateSpawn(const RME::core::Position& pos,
                           const RME::core::spawns::Spawn& oldState,
                           const RME::core::spawns::Spawn& newState) override;
    void recordSetGroundItem(const RME::core::Position& pos, uint16_t newGroundItemId, uint16_t oldGroundItemId) override;
    void recordSetBorderItems(const RME::core::Position& pos,
                              const QList<uint16_t>& newBorderItemIds,
                              const QList<uint16_t>& oldBorderItemIds) override;
    void recordAddItem(const RME::core::Position& pos, uint16_t itemId) override;
    void recordRemoveItem(const RME::core::Position& pos, uint16_t itemId) override;

signals:
    // Map state signals
    void mapChanged();
    void selectionChanged();
    
    // File operation signals (FINAL-01)
    void mapLoaded(const QString& filename);
    void mapSaved(const QString& filename);
    void mapModifiedChanged(bool modified);
    void mapClosed();

private:
    RME::core::Map* m_map;
    
    // Services
    RME::core::IBrushStateService* m_brushStateService;
    RME::core::IEditorStateService* m_editorStateService;
    RME::core::IClientDataService* m_clientDataService;
    RME::core::IApplicationSettingsService* m_settingsService;
    
    // Direct dependencies (will be obtained from services)
    QUndoStack* m_undoStack;
    RME::core::selection::SelectionManager* m_selectionManager;
    RME::core::brush::BrushManager* m_brushManager;
    RME::core::settings::AppSettings* m_appSettings;
    RME::core::assets::AssetManager* m_assetManager;
    RME::core::houses::Houses* m_housesManager;
    RME::core::clipboard::ClipboardManager* m_clipboardManager;
    RME::core::waypoints::WaypointManager* m_waypointManager;
    
    // Tool mode state (LOGIC-06)
    ToolMode m_currentToolMode = ToolMode::Brush;
    quint32 m_currentHouseForTools = 0;
    QString m_currentWaypointForTools;
    
    // File state (FINAL-01)
    QString m_currentMapFilename;
    bool m_mapModified = false;
    
    // Helper methods
    void initializeDependencies();
    RME::core::clipboard::ClipboardContent convertTilesToClipboardContent(const QList<RME::core::Tile*>& tiles) const;
};

} // namespace editor_logic
} // namespace RME

#endif // RME_EDITOR_CONTROLLER_H
