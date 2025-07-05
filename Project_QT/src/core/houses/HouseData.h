#ifndef RME_CORE_HOUSES_HOUSEDATA_H
#define RME_CORE_HOUSES_HOUSEDATA_H

#include <QString>
#include <QList>
#include "core/Position.h" // Path confirmed from previous ls output

// Forward declaration if HouseExit becomes a more complex struct/class later
// namespace RME { namespace core { namespace houses { struct HouseExit; } } }

namespace RME {
namespace core {
namespace houses {

class HouseData {
public:
    uint32_t id = 0;
    QString name;
    Position entryPoint; // Main entry/exit point, often the temple position for towns
                         // but for houses it's their specific door.
    QList<Position> exits; // Additional exits, if any. OTBM might just store one main exit.
                           // Original RME had House::exit and TileLocation::house_exits.
                           // For simplicity, HouseData can store its designated exit(s).
    uint32_t townId = 0;   // ID of the town this house belongs to
    uint32_t rent = 0;     // Rent in gold coins
    int sizeInSqms = 0; // Size in SQM (often calculated, but can be stored)
    // bool isGuildhall = false; // Guildhall status, if applicable

    HouseData() = default;
    HouseData(uint32_t houseId, const QString& houseName, const Position& entry) :
        id(houseId), name(houseName), entryPoint(entry) {}

    // Methods to manage exits, if needed
    void addExit(const Position& exitPos) {
        if (!exits.contains(exitPos)) {
            exits.append(exitPos);
        }
    }
    void removeExit(const Position& exitPos) {
        exits.removeAll(exitPos);
    }

    // Basic equality operator
    bool operator==(const HouseData& other) const {
        return id == other.id &&
               name == other.name &&
               entryPoint == other.entryPoint &&
               exits == other.exits && // QList supports ==
               townId == other.townId &&
               rent == other.rent &&
               sizeInSqms == other.sizeInSqms;
    }

    bool operator!=(const HouseData& other) const {
        return !(*this == other);
    }
};

} // namespace houses
} // namespace core
} // namespace RME

#endif // RME_CORE_HOUSES_HOUSEDATA_H
