#ifndef RME_HOUSE_H
#define RME_HOUSE_H

#include "core/Position.h"
#include <QString>
#include <QList>
#include <QObject> // For Q_OBJECT if signals are ever needed, and for Q_DISABLE_COPY

// Forward declarations
namespace RME {
namespace core {
    class Map;
    class Tile;
}
}

// Forward declaration for potential test class
class TestHouse;
class TestHouses; // Houses manager might need to be a friend for some operations

namespace RME {
namespace core {
namespace houses {

class House {
    // Q_OBJECT // Uncomment if signals/slots are needed in the future
    friend class ::TestHouse;
    friend class ::TestHouses; // Allow Houses manager to call e.g. setId if changing ID
    friend class RME::core::Map; // Allow Map to set m_map pointer if houses are created by Map during load

public:
    // Constructor: Requires an ID and a map context.
    // Name, rent, etc., are set via setters.
    explicit House(quint32 id, RME::core::Map* map);
    ~House() = default;

    // Prevent copying, allow moving if needed (though typically managed by unique_ptr in Houses)
    House(const House&) = delete;
    House& operator=(const House&) = delete;
    House(House&&) = default; // Or delete if not needed
    House& operator=(House&&) = default; // Or delete if not needed

    // --- Getters ---
    quint32 getId() const { return m_id; }
    QString getName() const { return m_name; }
    int getRent() const { return m_rent; }
    quint32 getTownId() const { return m_townId; }
    bool isGuildhall() const { return m_isGuildhall; }
    const RME::core::Position& getExitPos() const { return m_exitPos; } // Returns potentially invalid if not set
    RME::core::Map* getMap() const { return m_map; } // Returns the map context
    const QList<RME::core::Position>& getTilePositions() const { return m_tilePositions; }

    // --- Setters ---
    void setName(const QString& name) { m_name = name; }
    void setRent(int rent) { m_rent = rent; }
    void setTownId(quint32 townId) { m_townId = townId; }
    void setIsGuildhall(bool isGuildhall) { m_isGuildhall = isGuildhall; }
    // Internal setter for exit position, direct. Public setExit will have more logic.
    void setExitPosInternal(const RME::core::Position& pos) { m_exitPos = pos; }

    // --- Tile List Management ---
    // Adds a tile position to this house. Does not affect the Tile object itself yet.
    void addTilePosition(const RME::core::Position& pos);
    // Removes a tile position from this house. Does not affect the Tile object itself yet.
    void removeTilePosition(const RME::core::Position& pos);
    bool hasTilePosition(const RME::core::Position& pos) const;
    void clearTilePositions() { m_tilePositions.clear(); }
    int getTileCount() const { return m_tilePositions.size(); }

    // --- Tile Interaction Logic (Declarations - Implementations in Part 2 or later) ---
    // Links a specific tile object to this house (e.g., tile->setHouseId(m_id)).
    void linkTile(RME::core::Tile* tile);
    // Unlinks a specific tile object from this house (e.g., tile->setHouseId(0)).
    void unlinkTile(RME::core::Tile* tile);

    // Sets the house exit. Handles unsetting old exit tile flag and setting new one.
    void setExit(const RME::core::Position& newExitPos);

    // Called when the house is being deleted. Iterates m_tilePositions,
    // gets actual Tile objects from m_map, and calls unlinkTile() on them.
    void cleanAllTileLinks();

private:
    // Allow Houses class to change ID directly if needed (e.g. during changeHouseID)
    void setId(quint32 id) { m_id = id; }

    quint32 m_id;
    QString m_name;
    int m_rent = 0; // Initialize primitive types
    quint32 m_townId = 0; // Initialize primitive types
    bool m_isGuildhall = false; // Initialize primitive types
    RME::core::Position m_exitPos; // Invalid by default (Position default constructor)
    QList<RME::core::Position> m_tilePositions;
    RME::core::Map* m_map; // Non-owning pointer to the map context
};

} // namespace houses
} // namespace core
} // namespace RME

#endif // RME_HOUSE_H
