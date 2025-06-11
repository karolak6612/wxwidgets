#ifndef RME_MATERIAL_DATA_H
#define RME_MATERIAL_DATA_H

#include <QString>
#include <QList>
#include <QMap>
#include <cstdint> // For uint16_t, etc.

namespace RME {
namespace core {
namespace assets {

/**
 * @brief Represents an item that can be part of a material brush, with a chance.
 * Typically used for ground brushes or as primary items in other brush types.
 */
struct MaterialItem {
    quint16 itemId = 0; ///< The server ID of the item.
    int chance = 100;   ///< The chance for this item to be chosen (interpretation depends on brush logic).

    /**
     * @brief Default constructor.
     */
    MaterialItem() = default;

    /**
     * @brief Constructs a MaterialItem with an item ID and chance.
     * @param id The server item ID.
     * @param ch The chance value.
     */
    MaterialItem(quint16 id, int ch = 100) : itemId(id), chance(ch) {}
};

/**
 * @brief Defines auto-bordering properties for a material.
 * Specifies how this material should border against other materials or itself.
 */
struct MaterialBorder {
    QString align;              ///< Alignment of the border (e.g., "outer", "inner").
    QString borderSetId;        ///< Identifier for a set of border items/rules (references another definition).
    QString toMaterialName;     ///< Optional: Name of the material this border applies against.
    quint16 groundEquivalentId = 0; ///< Optional: An item ID that acts as a ground equivalent for border calculations.
    bool isSuper = false;       ///< If true, this border takes precedence.

    /**
     * @brief Default constructor.
     */
    MaterialBorder() = default;
};

/**
 * @brief Represents an item used within a specific part of a wall material (e.g., a piece of a horizontal wall).
 */
struct MaterialWallPartItem {
    quint16 itemId = 0; ///< The server ID of the item.
    int chance = 100;   ///< Chance for this item to be used in this wall part.

    /**
     * @brief Default constructor.
     */
    MaterialWallPartItem() = default;
    /**
     * @brief Constructs a MaterialWallPartItem.
     * @param id The server item ID.
     * @param ch The chance value.
     */
    MaterialWallPartItem(quint16 id, int ch = 100) : itemId(id), chance(ch) {}
};

/**
 * @brief Represents a door that can be part of a wall material.
 */
struct MaterialDoor {
    quint16 itemId = 0;     ///< The server ID of the door item.
    QString type;         ///< Type of the door (e.g., "normal", "locked", "quest").
    bool isOpen = false;    ///< Default state of the door (open or closed).
    bool isLocked = false;  ///< If the door is locked by default (if applicable to type).

    /**
     * @brief Default constructor.
     */
    MaterialDoor() = default;

    /**
     * @brief Constructs a MaterialDoor.
     * @param id Server item ID of the door.
     * @param t Type string of the door.
     * @param open Default open state.
     * @param locked Default locked state.
     */
    MaterialDoor(quint16 id, const QString& t = QString(), bool open = false, bool locked = false)
        : itemId(id), type(t), isOpen(open), isLocked(locked) {}
};

/**
 * @brief Defines a specific visual representation for a part of a wall material.
 * For example, a horizontal section, a vertical section, a corner, a T-junction, or a pole.
 * Each part can consist of multiple items (chosen by chance) and can contain door definitions.
 */
struct MaterialWallPart {
    QString type;                       ///< Type of the wall part (e.g., "horizontal", "vertical", "pole").
    QList<MaterialWallPartItem> items;  ///< List of items that can form this wall part.
    QList<MaterialDoor> doors;          ///< List of possible doors for this wall part.

    /**
     * @brief Default constructor.
     */
    MaterialWallPart() = default;

    /**
     * @brief Constructs a MaterialWallPart with a specific type.
     * @param t The type string (e.g., "horizontal").
     */
    explicit MaterialWallPart(const QString& t) : type(t) {}
};

/**
 * @brief Represents a single tile within a composite material (e.g., a multi-tile doodad).
 * Specifies the item ID and its relative position within the composite structure.
 */
struct MaterialCompositeTile {
    int relativeX = 0;  ///< Relative X offset from the composite's anchor point.
    int relativeY = 0;  ///< Relative Y offset from the composite's anchor point.
    quint16 itemId = 0; ///< The server ID of the item for this tile of the composite.

    /**
     * @brief Default constructor.
     */
    MaterialCompositeTile() = default;

    /**
     * @brief Constructs a MaterialCompositeTile.
     * @param rx Relative X offset.
     * @param ry Relative Y offset.
     * @param id Server item ID.
     */
    MaterialCompositeTile(int rx, int ry, quint16 id) : relativeX(rx), relativeY(ry), itemId(id) {}
};

/**
 * @brief Defines a composite material, typically a multi-tile doodad.
 * Consists of a list of tiles that form the complete structure, with a chance for this
 * specific arrangement to be chosen if multiple composites are defined for a doodad material.
 */
struct MaterialComposite {
    int chance = 100;                   ///< Chance for this composite arrangement to be chosen.
    QList<MaterialCompositeTile> tiles; ///< List of tiles forming this composite structure.

    /**
     * @brief Default constructor.
     */
    MaterialComposite() = default;

    /**
     * @brief Constructs a MaterialComposite with a specific chance.
     * @param ch The chance value.
     */
    MaterialComposite(int ch) : chance(ch) {}
};

/**
 * @brief Main class to store all data related to a single material brush definition.
 * This class aggregates all properties and specific data structures for different
 * types of material brushes like ground, wall, doodad, etc., as parsed from
 * RME's material XML files.
 */
class MaterialData {
public:
    QString id;                 ///< Unique identifier for the material (from <brush name="...">).
    QString brushType;          ///< Type of the brush (e.g., "ground", "wall", "doodad", "table").
    quint16 serverLookId = 0;   ///< Server look ID, typically for items placed by the brush.
    quint16 lookId = 0;         ///< Client look ID, if different from server_lookid or as a fallback.
    int zOrder = 0;             ///< Z-ordering hint for items placed by this material.
    bool soloOptional = false;  ///< If true, this material might not be placed if it's alone (specific to some brush logic).

    // Common brush properties potentially applicable to multiple types
    bool onBlocking = false;    ///< If true, brush can be placed on blocking tiles (e.g. walls).
    bool redoBorders = false;   ///< If true, existing borders should be re-evaluated after placing this material.
    bool oneSize = false;       ///< If true, the brush places a single, non-variable item/structure.
    QString thickness;          ///< Thickness string (e.g., "100/100"), interpretation depends on brush type.

    QList<MaterialItem> primaryItems;         ///< Primary items for the brush (e.g., ground items, doodad items).
    QList<MaterialBorder> borders;            ///< Definitions for auto-bordering.
    QList<QString> friendMaterials;           ///< List of material names considered "friends" for bordering.
    QList<QString> optionalBorderSetIds;      ///< List of optional border set IDs to apply.

    // Type-specific data structures
    /** @brief For brushType="wall", stores definitions for different wall parts (horizontal, vertical, etc.), keyed by part type string. */
    QMap<QString, MaterialWallPart> wallParts;
    /** @brief For brushType="doodad" with <composite> child elements, stores definitions of multi-tile composite structures. */
    QList<MaterialComposite> composites;

    /**
     * @brief Default constructor.
     */
    MaterialData() = default;

    /**
     * @brief Constructs MaterialData with an ID and optionally a brush type.
     * @param matId The unique identifier for the material.
     * @param bType The type of the brush (e.g., "ground", "wall").
     */
    explicit MaterialData(const QString& matId, const QString& bType = QString()) : id(matId), brushType(bType) {}

    /**
     * @brief Convenience method for wall brushes to get or create a MaterialWallPart.
     * If a wall part of the given type does not exist, it is created.
     * @param wallPartType The type of the wall part (e.g., "horizontal", "vertical_end_south").
     * @return A reference to the MaterialWallPart for the given type.
     */
    MaterialWallPart& getOrCreateWallPart(const QString& wallPartType) {
        // QMap::operator[] default-constructs if key doesn't exist, then returns a reference.
        // If MaterialWallPart needs specific construction with type, this is slightly less direct.
        // However, our MaterialWallPart default constructor is fine, we can set type after.
        MaterialWallPart& part = wallParts[wallPartType];
        if (part.type.isEmpty()) { // If it was just default-constructed by operator[]
            part.type = wallPartType;
        }
        return part;
    }
};

} // namespace assets
} // namespace core
} // namespace RME

#endif // RME_MATERIAL_DATA_H
