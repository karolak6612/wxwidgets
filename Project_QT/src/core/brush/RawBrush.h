#ifndef RME_RAW_BRUSH_H
#define RME_RAW_BRUSH_H

#include "core/brush/Brush.h"
#include "core/Position.h" // For RME::core::Position
#include <QString>
#include <cstdint> // For uint16_t

// Forward declarations
namespace RME {
namespace core {
    class BrushSettings;
    namespace map { class Map; }
    namespace editor { class EditorControllerInterface; }
    class ItemData; // Forward declare ItemData if getLookID needs it via AssetManager
}
}

// Forward declaration for potential test class
class TestRawBrush;

namespace RME {
namespace core {
namespace brush {

class RawBrush : public RME::core::Brush {
    friend class ::TestRawBrush; // Friend class for testing

public:
    RawBrush(uint16_t itemId = 0);
    ~RawBrush() override = default;

    void setItemId(uint16_t itemId);
    uint16_t getItemId() const;

    // Overridden methods from Brush
    void apply(RME::core::editor::EditorControllerInterface* controller,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& settings) override;

    QString getName() const override;
    // getLookID might need ItemData from AssetManager if it's to return clientID
    // For now, let's assume it might just return m_itemId or a generic icon ID
    int getLookID(const RME::core::BrushSettings& settings) const override;
    bool canApply(const RME::core::map::Map* map,
                  const RME::core::Position& pos,
                  const RME::core::BrushSettings& settings) const override;

    // Specific to RawBrush, indicates it doesn't use material system like some other brushes
    bool hasMaterial() const override { return false; }

private:
    uint16_t m_itemId;
};

} // namespace brush
} // namespace core
} // namespace RME

#endif // RME_RAW_BRUSH_H
