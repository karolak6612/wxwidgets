#ifndef MOCK_EDITORCONTROLLER_H
#define MOCK_EDITORCONTROLLER_H

// Attempt to include the real interface if it exists, otherwise use minimal definition
#if __has_include("editor_logic/EditorControllerInterface.h")
    #include "editor_logic/EditorControllerInterface.h"
    // Define RME::core::TileMapFlag if it's not pulled in by the interface
    #ifndef RME_TILE_H // Assuming Tile.h defines TileMapFlag
        #include "core/Tile.h" // For RME::TileMapFlag
    #endif
#elif __has_include("core/editor_logic/EditorControllerInterface.h")
    #include "core/editor_logic/EditorControllerInterface.h"
    #ifndef RME_TILE_H
        #include "core/Tile.h"
    #endif
#else
    // Minimal definition for EditorControllerInterface if not present from LOGIC-01
    #include "core/Position.h"
    #include "core/Item.h"
    #include "core/Tile.h"     // For RME::TileMapFlag
    #include <memory>
    #include <cstdint>

    namespace RME { namespace core {
        class Position;
        class Item;
        // TileMapFlag is in RME namespace, not RME::core
    }}
    // Forward declare RME::TileMapFlag correctly
    namespace RME { enum class TileMapFlag : uint32_t; }


    namespace RME { namespace core { namespace editor {
        class EditorControllerInterface {
        public:
            virtual ~EditorControllerInterface() = default;
            // Methods for RawBrush
            virtual void setTileGround(const Position& pos, std::unique_ptr<Item> groundItem) = 0;
            virtual void addStackedItemToTile(const Position& pos, std::unique_ptr<Item> item) = 0;
            virtual void removeGroundItemFromTile(const Position& pos, uint16_t itemId) = 0;
            virtual void removeStackedItemFromTile(const Position& pos, uint16_t itemId) = 0;
            // Methods for EraserBrush
            virtual void clearTileNormally(const Position& pos, bool leaveUniqueItems) = 0;
            virtual void clearTileAggressively(const Position& pos, bool leaveUniqueItems) = 0;
            // New methods for HouseBrush
            virtual uint32_t getTileHouseId(const Position& pos) = 0;
            virtual void setTileHouseId(const Position& pos, uint32_t houseId) = 0;
            virtual void setTileMapFlag(const Position& pos, RME::TileMapFlag flag, bool set) = 0;
            virtual void clearDoorIdsOnTile(const Position& pos) = 0; // Placeholder
            virtual void removeMovablesFromTile(const Position& pos) = 0; // Placeholder
            virtual void assignHouseDoorIdToTileDoors(const Position& pos, uint32_t currentHouseId, uint32_t oldHouseIdOnTile) = 0; // Placeholder
            virtual void addTilePositionToHouse(uint32_t houseId, const Position& pos) = 0;
            virtual void removeTilePositionFromHouse(uint32_t houseId, const Position& pos) = 0;
        };
    }}} // namespace RME::core::editor
#endif


#include <QList>
#include <QString>
// Ensure RME::core::Position and RME::TileMapFlag are fully defined or correctly forward declared for CallRecord
// If minimal interface is used, Tile.h (for TileMapFlag) and Position.h would be included by it.
#include "core/Position.h"
#include "core/Tile.h" // For RME::TileMapFlag


class MockEditorController : public RME::core::editor::EditorControllerInterface {
public:
    using RMEPosition = RME::core::Position;
    using RMEItem = RME::core::Item;
    using RMETileMapFlag = RME::TileMapFlag;


    struct CallRecord {
        QString method;
        RMEPosition pos;
        // Fields for RawBrush & EraserBrush
        uint16_t itemId = 0;
        // bool isGround = false; // Infer from method name
        bool leaveUnique = false;
        // Fields for HouseBrush
        uint32_t houseId = 0;
        uint32_t oldHouseId = 0;
        RMETileMapFlag mapFlag = RMETileMapFlag::NO_FLAGS;
        bool flagSet = false;

        // Default constructor
        CallRecord(QString m = "", RMEPosition p = RMEPosition()) : method(m), pos(p) {}
        // Constructor for RawBrush item calls (itemID)
        CallRecord(QString m, RMEPosition p, uint16_t id) :
            method(m), pos(p), itemId(id) {}
        // Constructor for EraserBrush calls (leaveUnique)
        CallRecord(QString m, RMEPosition p, bool unique) :
            method(m), pos(p), leaveUnique(unique) {}
        // Constructor for HouseBrush setTileHouseId
        CallRecord(QString m, RMEPosition p, uint32_t hId) :
            method(m), pos(p), houseId(hId) {}
        // Constructor for HouseBrush setTileMapFlag
        CallRecord(QString m, RMEPosition p, RMETileMapFlag flagVal, bool setVal) :
            method(m), pos(p), mapFlag(flagVal), flagSet(setVal) {}
        // Constructor for HouseBrush assignHouseDoorIdToTileDoors
        CallRecord(QString m, RMEPosition p, uint32_t currentHId, uint32_t oldHId) :
            method(m), pos(p), houseId(currentHId), oldHouseId(oldHId) {}
    };
    QList<CallRecord> calls;
    uint32_t mock_current_tile_house_id = 0;

    // Implementations for RawBrush & EraserBrush methods
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

    // New implementations for HouseBrush conceptual methods
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

    void reset() {
        calls.clear();
        mock_current_tile_house_id = 0;
    }
};
#endif // MOCK_EDITORCONTROLLER_H
