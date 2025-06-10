#ifndef RME_OUTFIT_H
#define RME_OUTFIT_H

#include <qglobal.h> // For quint16, quint8

namespace RME {

struct Outfit {
    quint16 lookType = 0;
    quint16 lookItem = 0;     // Item ID for looktype if it's an item
    quint16 lookMount = 0;    // Mount looktype
    quint8  head = 0;
    quint8  body = 0;
    quint8  legs = 0;
    quint8  feet = 0;
    quint8  addons = 0;       // Bitmask: 1=addon1, 2=addon2, 3=both

    Outfit() = default;

    bool operator==(const Outfit& other) const {
        return lookType == other.lookType &&
               lookItem == other.lookItem &&
               lookMount == other.lookMount &&
               head == other.head &&
               body == other.body &&
               legs == other.legs &&
               feet == other.feet &&
               addons == other.addons;
    }
    bool operator!=(const Outfit& other) const {
        return !(*this == other);
    }
};

} // namespace RME

#endif // RME_OUTFIT_H
