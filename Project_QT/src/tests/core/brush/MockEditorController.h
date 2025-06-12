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

    namespace RME { namespace core { namespace editor {
    class EditorControllerInterface {
    public:
        virtual ~EditorControllerInterface() = default;
        // These methods are conceptual based on RawBrush's needs.
        // The actual EditorController will implement these via commands.
        virtual void setTileGround(const Position& pos, std::unique_ptr<Item> groundItem) = 0;
        virtual void addStackedItemToTile(const Position& pos, std::unique_ptr<Item> item) = 0;
        virtual void removeGroundItemFromTile(const Position& pos, uint16_t itemId) = 0;
        virtual void removeStackedItemFromTile(const Position& pos, uint16_t itemId) = 0;
        // Add other methods if RawBrush needs them e.g. for complex placement logic.
    };
    }}} // namespace RME::core::editor
#endif


#include <QList>
#include <QVariantMap> // Not strictly needed for CallRecord but often useful for mocks
#include <QString>     // For CallRecord::method

// Ensure Position and Item are RME::core types if the minimal interface is used
// If the actual interface is included, these are already namespaced correctly.
#ifndef __has_include
    namespace RME { namespace core { class Position; class Item; } }
#endif


class MockEditorController : public RME::core::editor::EditorControllerInterface {
public:
    struct CallRecord {
        QString method;
        RME::core::Position pos;
        uint16_t itemId = 0;
        // bool isGround = false; // Could be inferred from method name or explicit
        // Add more fields if needed, e.g., item attributes, subtype
    };
    QList<CallRecord> calls;

    // Ensure using directives or fully qualified names match the actual interface definition
    using RMEPosition = RME::core::Position;
    using RMEItem = RME::core::Item;

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
    void reset() {
        calls.clear();
    }
};
#endif // MOCK_EDITORCONTROLLER_H
