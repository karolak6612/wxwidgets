#ifndef RME_SETHOUSEEXITCOMMAND_H
#define RME_SETHOUSEEXITCOMMAND_H

#include <QUndoCommand>
#include "core/Position.h" // RME::core::Position

// Forward declarations
namespace RME {
    class Map; // RME::Map
    namespace core {
        namespace houses {
            class House; // RME::core::houses::House
        }
    }
}

class SetHouseExitCommand : public QUndoCommand {
public:
    SetHouseExitCommand(RME::core::houses::House* house,
                        const RME::core::Position& newExitPos,
                        RME::Map* map, // Map pointer for notifications
                        QUndoCommand* parent = nullptr);

    ~SetHouseExitCommand() override = default;

    void undo() override;
    void redo() override;

    // Optional: for merging consecutive exit changes for the same house
    // int id() const override;
    // bool mergeWith(const QUndoCommand *other) override;

private:
    RME::core::houses::House* m_house;
    RME::Map* m_map;
    RME::core::Position m_newExitPos;
    RME::core::Position m_oldExitPos; // Stored from house->getExitPos() at construction
};

#endif // RME_SETHOUSEEXITCOMMAND_H
