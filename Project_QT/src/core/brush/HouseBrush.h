#ifndef RME_HOUSEBRUSH_H
#define RME_HOUSEBRUSH_H

#include "core/brush/Brush.h" // Base RME::core::Brush
#include <cstdint>           // For uint32_t

// Forward declarations from RME::core
namespace RME {
namespace core {
    class Position;
    struct BrushSettings;
    namespace map { class Map; }
    namespace editor { class EditorControllerInterface; }
} // namespace core
} // namespace RME

namespace RME {
namespace core {
namespace brush {

// Placeholder for a global constant
const int EDITOR_SPRITE_HOUSE_BRUSH_LOOKID = 99002; // Example value

class HouseBrush : public Brush { // Inherits RME::core::Brush
public:
    HouseBrush();
    ~HouseBrush() override = default;

    void setCurrentHouseId(uint32_t houseId);
    uint32_t getCurrentHouseId() const { return m_currentHouseId; }

    // RME::core::Brush interface overrides
    QString getName() const override;
    int getLookID(const BrushSettings& settings) const override;

    void apply(editor::EditorControllerInterface* controller, const Position& pos, const BrushSettings& settings) override;
    bool canApply(const map::Map* map, const Position& pos, const BrushSettings& settings) const override;

    // Brush specific flags/properties
    bool isHouse() const override { return true; }
    bool canDrag() const override { return true; }
    // needsBorders() can be default (false) unless house application directly causes border changes handled by brush.
    // The original HouseBrush didn't seem to override needsBorders explicitly.
    // bool needsBorders() const override { return false; } // Default from base is fine

private:
    uint32_t m_currentHouseId = 0;
};

} // namespace brush
} // namespace core
} // namespace RME
#endif // RME_HOUSEBRUSH_H
