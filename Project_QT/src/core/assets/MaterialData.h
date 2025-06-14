#ifndef RME_MATERIALDATA_H
#define RME_MATERIALDATA_H

#include <QString>
#include <QList>
#include <QSet>
#include <QMap>
#include <variant>
#include <cstdint>

namespace RME {
namespace core {
namespace assets {

// --- Type-specific data structures for MaterialData ---

struct MaterialItemEntry {
    uint16_t itemId = 0;
    int chance = 100; // Default chance

    bool operator==(const MaterialItemEntry& other) const {
        return itemId == other.itemId && chance == other.chance;
    }
};

enum class SpecificConditionType {
    UNKNOWN,
    MATCH_BORDER,   // Example: <match_border id="41" edge="s" />
    MATCH_GROUND    // Example: <match_ground id="101" /> (hypothetical)
    // Add other condition types as identified from XML or needed
};

struct SpecificCondition {
    SpecificConditionType type = SpecificConditionType::UNKNOWN;
    QString targetId;      // For MATCH_BORDER (item ID), MATCH_GROUND (material ID or item ID)
    QString edge;          // For MATCH_BORDER
    // QString propertyName;  // For future generic property checks
    // QString propertyValue;

    bool operator==(const SpecificCondition& other) const {
        return type == other.type && targetId == other.targetId && edge == other.edge;
    }
};

enum class SpecificActionType {
    UNKNOWN,
    REPLACE_BORDER, // Example: <replace_border id="41" edge="s" with="4661" />
    ADD_ITEM        // Example: <add_item id="X" /> (hypothetical)
    // Add other action types as identified
};

struct SpecificAction {
    SpecificActionType type = SpecificActionType::UNKNOWN;
    QString targetId;       // For REPLACE_BORDER (original item ID to be replaced on a specific edge)
    QString edge;           // For REPLACE_BORDER (which edge's item to replace)
    uint16_t withItemId = 0;  // For REPLACE_BORDER (the new item ID)
    // uint16_t affectedItemId; // Could be targetId for REMOVE_ITEM, or newItemId for ADD_ITEM

    bool operator==(const SpecificAction& other) const {
        return type == other.type && targetId == other.targetId &&
               edge == other.edge && withItemId == other.withItemId;
    }
};

struct SpecificRuleCase {
    bool keepBaseBorder = false; // From keep_border attribute
    QList<SpecificCondition> conditions;
    QList<SpecificAction> actions;

    bool operator==(const SpecificRuleCase& other) const {
        return keepBaseBorder == other.keepBaseBorder &&
               conditions == other.conditions &&
               actions == other.actions;
    }
};

struct BorderSetData {
    QString id; // The ID of this border set (e.g., "1", "38")
    QMap<QString, uint16_t> edgeItems; // Maps edge string (e.g., "n", "s", "cne") to item ID

    // Optional: Add default constructor or other helpers if needed
    BorderSetData() = default;
    explicit BorderSetData(QString setId) : id(std::move(setId)) {}

    bool operator==(const BorderSetData& other) const {
        return id == other.id && edgeItems == other.edgeItems;
    }
};

struct MaterialBorderRule {
    QString align; // e.g., "outer", "inner"
    QString toBrushName; // Name of the brush this border applies to, or "none"
    QString ruleTargetId; // Stores the raw 'id' attribute from XML (can be item ID or set ID)
    bool isSuper = false;
    uint16_t groundEquivalent = 0;
    QList<SpecificRuleCase> specificRuleCases; // NEW

    bool operator==(const MaterialBorderRule& other) const {
        return align == other.align &&
               toBrushName == other.toBrushName &&
               ruleTargetId == other.ruleTargetId &&
               isSuper == other.isSuper &&
               groundEquivalent == other.groundEquivalent &&
               specificRuleCases == other.specificRuleCases; // ADDED
    }
};

struct MaterialGroundSpecifics {
    QList<MaterialItemEntry> items;      // For <item id="..." chance="..."/>
    QList<MaterialBorderRule> borders;   // For <border .../>
    QSet<QString> friends;               // For <friend name="..."/>
    QList<uint16_t> optionals;           // For <optional id="..."/> (item IDs)

    bool operator==(const MaterialGroundSpecifics& other) const {
        return items == other.items && borders == other.borders &&
               friends == other.friends && optionals == other.optionals;
    }
};

struct MaterialDoorDefinition {
    uint16_t id = 0;
    QString doorType; // e.g., "normal", "locked", "archway", "window"
    bool isOpen = false;
    bool isLocked = false; // Relevant for "normal" or "locked" door types

    bool operator==(const MaterialDoorDefinition& other) const {
        return id == other.id && doorType == other.doorType &&
               isOpen == other.isOpen && isLocked == other.isLocked;
    }
};

struct MaterialWallPart {
    QString orientationType; // e.g., "horizontal", "vertical", "pole", "corner"
    QList<MaterialItemEntry> items; // Items for this wall part
    QList<MaterialDoorDefinition> doors; // Doors defined for this wall part

    bool operator==(const MaterialWallPart& other) const {
        return orientationType == other.orientationType && items == other.items && doors == other.doors;
    }
};

struct MaterialWallSpecifics {
    QList<MaterialWallPart> parts;

    bool operator==(const MaterialWallSpecifics& other) const {
        return parts == other.parts;
    }
};

struct MaterialCompositeTile {
    int x = 0, y = 0, z = 0; // Relative offsets
    QList<uint16_t> itemIds; // Items on this composite tile part

    bool operator==(const MaterialCompositeTile& other) const {
        return x == other.x && y == other.y && z == other.z && itemIds == other.itemIds;
    }
};

struct MaterialAlternate {
    int chance = 100;
    QList<uint16_t> singleItemIds; // For simple <alternate><item id="..."/></alternate>
    QList<MaterialCompositeTile> compositeTiles; // For <alternate><composite><tile>...</composite></alternate>
    // An alternate contains EITHER singleItemIds OR compositeTiles, not both typically.

    bool operator==(const MaterialAlternate& other) const {
        return chance == other.chance && singleItemIds == other.singleItemIds &&
               compositeTiles == other.compositeTiles;
    }
};

struct MaterialDoodadSpecifics {
    bool draggable = false;
    bool onBlocking = false; // If the doodad itself is considered blocking
    QString thickness;       // e.g., "100/100"
    bool oneSize = false;
    bool redoBorders = false;
    bool onDuplicate = false; // New attribute found in some doodads
    QList<MaterialAlternate> alternates;

    bool operator==(const MaterialDoodadSpecifics& other) const {
        return draggable == other.draggable && onBlocking == other.onBlocking &&
               thickness == other.thickness && oneSize == other.oneSize &&
               redoBorders == other.redoBorders && onDuplicate == other.onDuplicate &&
               alternates == other.alternates;
    }
};

struct MaterialOrientedPart { // For carpets, tables
    QString align; // e.g., "s", "cne", "center"
    QList<MaterialItemEntry> items;

    bool operator==(const MaterialOrientedPart& other) const {
        return align == other.align && items == other.items;
    }
};

struct MaterialCarpetSpecifics {
    QList<MaterialOrientedPart> parts;
    bool onBlocking = true; // Carpets often have this attribute

    bool operator==(const MaterialCarpetSpecifics& other) const {
        return parts == other.parts && onBlocking == other.onBlocking;
    }
};

struct MaterialTableSpecifics { // Similar to Carpet
    QList<MaterialOrientedPart> parts;
    bool onBlocking = true;

    bool operator==(const MaterialTableSpecifics& other) const {
        return parts == other.parts && onBlocking == other.onBlocking;
    }
};

// Main MaterialData class
class MaterialData {
public:
    QString id;                 // From <brush name="...">
    QString typeAttribute;      // From <brush type="...">
    uint16_t serverLookId = 0;
    uint16_t lookId = 0;
    int zOrder = 0;

    // Doodad-specific attributes (might also apply to others, but common for doodads)
    // These are here for convenience if they are top-level <brush> attributes
    bool isDraggable = false;
    bool isOnBlocking = false;
    QString brushThickness;
    bool isOneSize = false;
    bool isRedoBorders = false;
    bool isOnDuplicate = false; // From some doodad brushes

    // Using std::variant to hold one of the type-specific data structures
    std::variant<
        std::monostate, // Represents no specific data / uninitialized / error
        MaterialGroundSpecifics,
        MaterialWallSpecifics,
        MaterialDoodadSpecifics,
        MaterialCarpetSpecifics,
        MaterialTableSpecifics
    > specificData;

    MaterialData(QString brushId = QString(), QString brushType = QString())
        : id(std::move(brushId)), typeAttribute(std::move(brushType)) {}

    // Convenience type checkers
    bool isGround() const { return typeAttribute.compare("ground", Qt::CaseInsensitive) == 0; }
    bool isWall() const { return typeAttribute.compare("wall", Qt::CaseInsensitive) == 0; }
    bool isDoodad() const { return typeAttribute.compare("doodad", Qt::CaseInsensitive) == 0; }
    bool isCarpet() const { return typeAttribute.compare("carpet", Qt::CaseInsensitive) == 0; }
    bool isTable() const { return typeAttribute.compare("table", Qt::CaseInsensitive) == 0; }

    // Getters for specific data (example for Ground)
    const MaterialGroundSpecifics* getGroundSpecifics() const {
        return std::get_if<MaterialGroundSpecifics>(&specificData);
    }
    MaterialGroundSpecifics* getGroundSpecifics() {
        return std::get_if<MaterialGroundSpecifics>(&specificData);
    }
    // Similar getters can be added for Wall, Doodad, Carpet, Table

    bool operator==(const MaterialData& other) const {
        return id == other.id &&
               typeAttribute == other.typeAttribute &&
               serverLookId == other.serverLookId &&
               lookId == other.lookId &&
               zOrder == other.zOrder &&
               isDraggable == other.isDraggable &&
               isOnBlocking == other.isOnBlocking &&
               brushThickness == other.brushThickness &&
               isOneSize == other.isOneSize &&
               isRedoBorders == other.isRedoBorders &&
               isOnDuplicate == other.isOnDuplicate &&
               specificData == other.specificData;
    }
     bool operator!=(const MaterialData& other) const {
        return !(*this == other);
    }
};

} // namespace assets
} // namespace core
} // namespace RME

#endif // RME_MATERIALDATA_H
