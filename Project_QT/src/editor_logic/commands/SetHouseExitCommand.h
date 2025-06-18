#ifndef RME_SETHOUSEEXITCOMMAND_H
#define RME_SETHOUSEEXITCOMMAND_H

#include <QUndoCommand>
#include "core/Position.h" // RME::core::Position

// Forward declarations
namespace RME {
    namespace core {
        class Map; // RME::core::Map
        namespace houses {
            class Houses; // RME::core::houses::Houses
            class HouseData; // RME::core::houses::HouseData
        }
    }
}

class SetHouseExitCommand : public QUndoCommand {
public:
    SetHouseExitCommand(quint32 houseId,
                        const RME::core::Position& newExitPos,
                        RME::core::houses::Houses* housesManager,
                        RME::core::Map* map, // Map pointer for notifications
                        QUndoCommand* parent = nullptr);

    ~SetHouseExitCommand() override = default;

    void undo() override;
    void redo() override;

    // Optional: for merging consecutive exit changes for the same house
    // int id() const override;
    // bool mergeWith(const QUndoCommand *other) override;

private:
    quint32 m_houseId;
    RME::core::houses::Houses* m_housesManager;
    RME::core::Map* m_map;
    RME::core::Position m_newExitPos;
    RME::core::Position m_oldExitPos; // Stored from house->getExitPos() at construction
};

#endif // RME_SETHOUSEEXITCOMMAND_H
