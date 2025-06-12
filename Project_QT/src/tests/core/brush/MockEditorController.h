// Content of MockEditorController.h (showing additions and structure)
#ifndef MOCK_EDITORCONTROLLER_H
#define MOCK_EDITORCONTROLLER_H

// Includes for original interface and core types
#if __has_include("core/editor/EditorControllerInterface.h") // Adjusted path
    #include "core/editor/EditorControllerInterface.h"
#else
    // Minimal interface definition (as fallback, ensure it matches the real one)
    // This part should ideally not be needed if interface is always found
    namespace RME { namespace core { namespace editor { class EditorControllerInterface { /*...*/ }; }}}
#endif

#include "core/Position.h"
#include "core/Tile.h" // For RME::Tile, RME::TileMapFlag
#include "core/map/Map.h" // For RME::core::map::Map
#include "core/settings/AppSettings.h" // For RME::core::AppSettings
#include "core/assets/CreatureData.h" // For RME::core::assets::CreatureData
#include "core/assets/CreatureDatabase.h" // For RME::core::assets::CreatureDatabase
#include "core/spawns/SpawnData.h" // For RME::core::SpawnData
#include "core/actions/AppUndoCommand.h" // For RME::core::actions::AppUndoCommand


#include <QList>
#include <QString>
#include <memory> // For std::unique_ptr

// Forward declare if full includes are too heavy or cause cycles, though for mocks it's often fine
// namespace RME { namespace core {
//     class AppSettings;
//     namespace map { class Map; }
//     namespace assets { struct CreatureData; class CreatureDatabase; }
//     namespace actions { class AppUndoCommand; }
//     class SpawnData;
// }}


class MockEditorController : public RME::core::editor::EditorControllerInterface {
public:
    using RMEPosition = RME::core::Position;
    using RMEItem = RME::core::Item; // Assuming Item.h is included via Tile.h or similar
    using RMETileMapFlag = RME::TileMapFlag;
    using RMEMap = RME::core::map::Map;
    using RMETile = RME::core::Tile;
    using RMEAppSettings = RME::core::AppSettings;
    using RMECreatureData = RME::core::assets::CreatureData;
    using RMECreatureDatabase = RME::core::assets::CreatureDatabase;
    using RMESpawnData = RME::core::SpawnData;
    using RMEAppUndoCommand = RME::core::actions::AppUndoCommand;

    // Mocked member variables to be returned by getter methods
    RMEMap* m_mockMap = nullptr;
    RMEAppSettings* m_mockAppSettings = nullptr;
    RMECreatureDatabase* m_mockCreatureDatabase = nullptr;
    RMETile* m_mockTileForEditing = nullptr; // If getTileForEditing should return a specific mock tile


    struct CallRecord {
        QString method;
        RMEPosition pos;
        // Fields for RawBrush & EraserBrush
        uint16_t itemId = 0;
        bool leaveUnique = false;
        // Fields for HouseBrush
        uint32_t houseId = 0;
        uint32_t oldHouseId = 0;
        RMETileMapFlag mapFlag = RMETileMapFlag::NO_FLAGS;
        bool flagSet = false;
        // Fields for CreatureBrush
        const RMECreatureData* creatureType = nullptr;
        RMESpawnData spawnData; // Store by value or relevant parts
        QString commandType; // For generic recordAction
        // Store old/new tile states if needed for recordTileChange, or just log call

        // Default constructor
        CallRecord(QString m = "", RMEPosition p = RMEPosition()) : method(m), pos(p) {}
        // Constructor for item calls (itemID)
        CallRecord(QString m, RMEPosition p, uint16_t id) : method(m), pos(p), itemId(id) {}
        // Constructor for EraserBrush calls (leaveUnique)
        CallRecord(QString m, RMEPosition p, bool unique) : method(m), pos(p), leaveUnique(unique) {}
        // Constructor for HouseBrush setTileHouseId
        CallRecord(QString m, RMEPosition p, uint32_t hId) : method(m), pos(p), houseId(hId) {}
        // Constructor for HouseBrush setTileMapFlag
        CallRecord(QString m, RMEPosition p, RMETileMapFlag flagVal, bool setVal) : method(m), pos(p), mapFlag(flagVal), flagSet(setVal) {}
        // Constructor for HouseBrush assignHouseDoorIdToTileDoors
        CallRecord(QString m, RMEPosition p, uint32_t currentHId, uint32_t oldHId) : method(m), pos(p), houseId(currentHId), oldHouseId(oldHId) {}
        // Constructor for Creature calls
        CallRecord(QString m, RMEPosition p, const RMECreatureData* ct) : method(m), pos(p), creatureType(ct) {}
        // Constructor for SpawnData calls
        CallRecord(QString m, RMEPosition p, const RMESpawnData& sd) : method(m), pos(p), spawnData(sd) {}
         // Constructor for general action command
        CallRecord(QString m, QString cmdType) : method(m), commandType(cmdType) {}

    };
    QList<CallRecord> calls;
    uint32_t mock_current_tile_house_id = 0; // From existing mock

    // --- Existing mocked methods (Raw, Eraser, House) ---
    // ... (keep existing ones) ...
    void setTileGround(const RMEPosition& pos, std::unique_ptr<RMEItem> groundItem) override {
        calls.append({ "setTileGround", pos, groundItem ? groundItem->getID() : static_cast<uint16_t>(0) });
    }
    void addStackedItemToTile(const RMEPosition& pos, std::unique_ptr<RMEItem> item) override {
        calls.append({ "addStackedItemToTile", pos, item ? item->getID() : static_cast<uint16_t>(0) });
    }
    void removeGroundItemFromTile(const RMEPosition& pos, uint16_t itemId) override {
        calls.append({ "removeGroundItemFromTile", pos, itemId });
    }
    void removeStackedItemFromTile(const RMEPosition& pos, uint16_t itemId) override {
        calls.append({ "removeStackedItemFromTile", pos, itemId });
    }
    void clearTileNormally(const RMEPosition& pos, bool leaveUniqueItems) override {
        calls.append({ "clearTileNormally", pos, leaveUniqueItems });
    }
    void clearTileAggressively(const RMEPosition& pos, bool leaveUniqueItems) override {
        calls.append({ "clearTileAggressively", pos, leaveUniqueItems });
    }
    uint32_t getTileHouseId(const RMEPosition& pos) override {
        calls.append({ "getTileHouseId", pos });
        return mock_current_tile_house_id;
    }
    void setTileHouseId(const RMEPosition& pos, uint32_t houseId) override {
        calls.append({ "setTileHouseId", pos, houseId });
    }
    void setTileMapFlag(const RMEPosition& pos, RMETileMapFlag flag, bool set) override {
        calls.append({ "setTileMapFlag", pos, flag, set });
    }
    void clearDoorIdsOnTile(const RMEPosition& pos) override {
        calls.append({ "clearDoorIdsOnTile", pos });
    }
    void removeMovablesFromTile(const RMEPosition& pos) override {
        calls.append({ "removeMovablesFromTile", pos });
    }
    void assignHouseDoorIdToTileDoors(const RMEPosition& pos, uint32_t currentHouseId, uint32_t oldHouseIdOnTile) override {
        calls.append({ "assignHouseDoorIdToTileDoors", pos, currentHouseId, oldHouseIdOnTile });
    }
    void addTilePositionToHouse(uint32_t houseId, const RMEPosition& pos) override {
        calls.append({ "addTilePositionToHouse", pos, houseId });
    }
    void removeTilePositionFromHouse(uint32_t houseId, const RMEPosition& pos) override {
        calls.append({ "removeTilePositionFromHouse", pos, houseId });
    }

    // --- New methods from EditorControllerInterface (for CreatureBrush etc.) ---
    RMEMap* getMap() override { calls.append({"getMap"}); return m_mockMap; }
    const RMEMap* getMap() const override { calls.append({"getMap_const"}); return m_mockMap; }

    RMETile* getTileForEditing(const RMEPosition& pos) override {
        calls.append({"getTileForEditing", pos});
        if (m_mockTileForEditing) return m_mockTileForEditing;
        if (m_mockMap) return m_mockMap->getTileForEditing(pos); // Or getTile if it doesn't auto-create
        return nullptr;
    }

    RMEAppSettings* getAppSettings() override { calls.append({"getAppSettings"}); return m_mockAppSettings; }
    RMECreatureDatabase* getCreatureDatabase() override { calls.append({"getCreatureDatabase"}); return m_mockCreatureDatabase; }

    void recordAction(std::unique_ptr<RMEAppUndoCommand> command) override {
        calls.append({"recordAction", command ? command->text() : "UnknownCommand"});
    }

    void recordTileChange(const RMEPosition& pos, std::unique_ptr<RMETile> oldTileState, std::unique_ptr<RMETile> newTileState) override {
        CallRecord cr("recordTileChange", pos);
        // Optionally store more details about old/new tile state if needed for assertions
        calls.append(cr);
    }

    void recordAddCreature(const RMEPosition& tilePos, const RMECreatureData* creatureType) override {
        calls.append({"recordAddCreature", tilePos, creatureType});
    }
    void recordRemoveCreature(const RMEPosition& tilePos, const RMECreatureData* creatureType) override {
        calls.append({"recordRemoveCreature", tilePos, creatureType});
    }
    void recordAddSpawn(const RMESpawnData& spawnData) override {
        // For SpawnData, store its center position or a copy.
        calls.append({"recordAddSpawn", spawnData.getCenter(), spawnData});
    }
    void recordRemoveSpawn(const RMEPosition& spawnCenterPos) override {
        calls.append({"recordRemoveSpawn", spawnCenterPos});
    }
    void recordUpdateSpawn(const RMEPosition& spawnCenterPos, const RMESpawnData& oldSpawnData, const RMESpawnData& newSpawnData) override {
        // Could store both old and new spawn data if needed for complex verification
        CallRecord cr("recordUpdateSpawn", spawnCenterPos, newSpawnData);
        // cr.oldSpawnData = oldSpawnData; // If CallRecord is extended
        calls.append(cr);
    }

    void notifyTileChanged(const RMEPosition& pos) override {
        calls.append({"notifyTileChanged", pos});
    }

    void reset() {
        calls.clear();
        mock_current_tile_house_id = 0;
        m_mockMap = nullptr;
        m_mockAppSettings = nullptr;
        m_mockCreatureDatabase = nullptr;
        m_mockTileForEditing = nullptr;
    }
};
#endif // MOCK_EDITORCONTROLLER_H
