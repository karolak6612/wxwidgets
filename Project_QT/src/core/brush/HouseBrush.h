#ifndef RME_HOUSE_BRUSH_H
#define RME_HOUSE_BRUSH_H

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
    // No direct dependency on House.h here, uses ID.
}
}

// Forward declaration for potential test class
class TestHouseBrush;

namespace RME {
namespace core {
namespace brush {

class HouseBrush : public RME::core::Brush {
    friend class ::TestHouseBrush; // Friend class for testing

public:
    HouseBrush();
    ~HouseBrush() override = default;

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

    bool hasMaterial() const override { return false; } // HouseBrush doesn't use materials

private:
    quint32 m_currentHouseId = 0; // ID of the house to apply/erase
};

} // namespace brush
} // namespace core
} // namespace RME

#endif // RME_HOUSE_BRUSH_H
