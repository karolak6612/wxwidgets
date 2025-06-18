#ifndef RME_HOUSE_EXIT_BRUSH_H
#define RME_HOUSE_EXIT_BRUSH_H

#include "core/brush/Brush.h"
#include "core/Position.h" // For RME::core::Position
#include <QString>
#include <QtGlobal>    // For quint32

// Forward declarations
namespace RME {
namespace core {
    class BrushSettings;
    namespace map { class Map; }
    namespace editor { class EditorControllerInterface; }
}
}

// Forward declaration for potential test class
class TestHouseExitBrush;

namespace RME {
namespace core {
namespace brush {

class HouseExitBrush : public RME::core::Brush {
    friend class ::TestHouseExitBrush; // Friend class for testing

public:
    HouseExitBrush();
    ~HouseExitBrush() override = default;

    void setCurrentHouseId(quint32 houseId);
    quint32 getCurrentHouseId() const;

    // Overridden methods from Brush
    void apply(RME::core::editor::EditorControllerInterface* controller,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& settings) override;

    QString getName() const override;
    int getLookID(const RME::core::BrushSettings& settings) const override;
    bool canApply(const RME::core::map::Map* map,
                  const RME::core::Position& pos,
                  const RME::core::BrushSettings& settings) const override;

    bool hasMaterial() const override { return false; } // HouseExitBrush doesn't use materials
    bool canDrag() const override { return false; } // House exits are set individually
    bool canSmear() const override { return false; } // House exits are set individually
    bool oneSizeFitsAll() const override { return true; } // Size doesn't matter for exits

private:
    quint32 m_currentHouseId = 0; // ID of the house to set exit for
};

} // namespace brush
} // namespace core
} // namespace RME

#endif // RME_HOUSE_EXIT_BRUSH_H