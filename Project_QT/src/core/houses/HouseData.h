#ifndef HOUSEDATA_H
#define HOUSEDATA_H

#include "Project_QT/src/core/Position.h"
#include <QString>
#include <QList>
#include <QSet>
#include <QHash> // For QSet<Position> if Position needs qHash

// Forward declaration
namespace RME {
class Map;
}

// NOTE: For QSet<RME::Position> to compile and work correctly,
// RME::Position needs a global or RME namespaced qHash function
// and an operator==. These should ideally be defined in or alongside
// Position.h. For example:
// inline uint qHash(const RME::Position& key, uint seed = 0) {
//     return qHash(key.x, seed) ^ qHash(key.y, seed >> 1) ^ qHash(key.z, seed >> 2);
// }
// inline bool operator==(const RME::Position& p1, const RME::Position& p2) {
//    return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
// }

namespace RME {

class HouseData {
public:
    // --- Constructors ---
    HouseData();
    explicit HouseData(uint32_t houseId, const QString& houseName = QString());

    // --- Properties ---
    uint32_t getId() const { return m_id; }
    void setId(uint32_t id) { m_id = id; }

    const QString& getName() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    uint32_t getTownId() const { return m_townId; }
    void setTownId(uint32_t townId) { m_townId = townId; }

    const Position& getEntryPoint() const { return m_entryPoint; }
    void setEntryPoint(const Position& entryPoint) { m_entryPoint = entryPoint; }

    uint32_t getRent() const { return m_rent; }
    void setRent(uint32_t rent) { m_rent = rent; }

    int getSizeInSqms() const { return m_sizeInSqms; }
    void setSizeInSqms(int size) { m_sizeInSqms = size; }

    bool isGuildhall() const { return m_isGuildhall; }
    void setIsGuildhall(bool isGuildhall) { m_isGuildhall = isGuildhall; }

    // --- Exits Management ---
    const QList<Position>& getExits() const { return m_exits; }
    void addExit(const Position& pos);
    bool removeExit(const Position& pos);
    void clearExits() { m_exits.clear(); }

    // --- Tile Positions Management ---
    const QSet<Position>& getTilePositions() const { return m_tiles; }
    void addTilePosition(const Position& pos);
    bool removeTilePosition(const Position& pos);
    bool containsTile(const Position& pos) const;
    void clearTilePositions() { m_tiles.clear(); }

    // --- Utility Methods ---
    // int calculateSizeSqms(const Map& map) const;
    QString getDescription() const;


private:
    uint32_t m_id = 0;
    QString m_name;
    uint32_t m_townId = 0;
    Position m_entryPoint;
    uint32_t m_rent = 0;
    int m_sizeInSqms = 0;
    bool m_isGuildhall = false;

    QList<Position> m_exits;
    QSet<Position> m_tiles;
};

} // namespace RME

#endif // HOUSEDATA_H
