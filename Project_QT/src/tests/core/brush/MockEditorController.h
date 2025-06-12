#ifndef MOCK_EDITORCONTROLLER_H
#define MOCK_EDITORCONTROLLER_H

// Attempt to include the real interface if it exists, otherwise use minimal definition
// This path assumes the interface would be in editor_logic as per previous tasks.
#if __has_include("editor_logic/EditorControllerInterface.h")
    #include "editor_logic/EditorControllerInterface.h"
#elif __has_include("core/editor_logic/EditorControllerInterface.h")
    #include "core/editor_logic/EditorControllerInterface.h"
#else
    // Minimal definition for EditorControllerInterface if not present from LOGIC-01
    #include "core/Position.h" // For RME::core::Position
    #include "core/Item.h"     // For RME::core::Item (used in unique_ptr)
    #include <memory>          // For std::unique_ptr
    #include <cstdint>         // For uint16_t

    // Ensure Position and Item are RME::core types if the minimal interface is used
    // This block is for the case where the actual interface is NOT found by __has_include
    namespace RME { namespace core {
        class Position;
        class Item;
        namespace editor {
            class EditorControllerInterface {
            public:
                virtual ~EditorControllerInterface() = default;
                // Methods for RawBrush
                virtual void setTileGround(const Position& pos, std::unique_ptr<Item> groundItem) = 0;
                virtual void addStackedItemToTile(const Position& pos, std::unique_ptr<Item> item) = 0;
                virtual void removeGroundItemFromTile(const Position& pos, uint16_t itemId) = 0;
                virtual void removeStackedItemFromTile(const Position& pos, uint16_t itemId) = 0;
                // New methods for EraserBrush
                virtual void clearTileNormally(const Position& pos, bool leaveUniqueItems) = 0;
                virtual void clearTileAggressively(const Position& pos, bool leaveUniqueItems) = 0;
            };
        } // namespace editor
    }} // namespace RME::core
#endif


#include <QList>
#include <QVariantMap>
#include <QString>

// Forward declare RME::core::Position and RME::core::Item again in case minimal interface was used
// and they were only forward declared inside RME::core::editor namespace block.
// This ensures they are known in the global scope for MockEditorController parameters if needed.
// However, it's better to fully qualify them in MockEditorController method signatures.
namespace RME { namespace core { class Position; class Item; }}


class MockEditorController : public RME::core::editor::EditorControllerInterface {
public:
    // Using fully qualified names for RME types to avoid ambiguity.
    using RMEPosition = RME::core::Position;
    using RMEItem = RME::core::Item;

    struct CallRecord {
        QString method;
        RMEPosition pos;
        uint16_t itemId = 0;
        // bool isGround = false; // This field was ambiguous, better to infer from method name or add specific fields
        bool leaveUnique = false;

        // Constructor for RawBrush-like calls that involve itemID
        CallRecord(QString m, RMEPosition p, uint16_t id) :
            method(m), pos(p), itemId(id), leaveUnique(false) {}
        // Constructor for EraserBrush-like calls that involve leaveUnique flag
        CallRecord(QString m, RMEPosition p, bool unique) :
            method(m), pos(p), itemId(0), leaveUnique(unique) {}
        // Default constructor for safety or other types of calls
        CallRecord(QString m, RMEPosition p) :
            method(m), pos(p), itemId(0), leaveUnique(false) {}
    };
    QList<CallRecord> calls;


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

    // New implementations for EraserBrush
    void clearTileNormally(const RMEPosition& pos, bool leaveUniqueItems) override {
        calls.append({ "clearTileNormally", pos, leaveUniqueItems });
    }
    void clearTileAggressively(const RMEPosition& pos, bool leaveUniqueItems) override {
        calls.append({ "clearTileAggressively", pos, leaveUniqueItems });
    }

    void reset() {
        calls.clear();
    }
};
#endif // MOCK_EDITORCONTROLLER_H
