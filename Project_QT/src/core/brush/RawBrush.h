#ifndef RME_RAWBRUSH_H
#define RME_RAWBRUSH_H

#include "core/brush/Brush.h" // Base RME::core::Brush
#include <cstdint>           // For uint16_t

// Forward declarations from RME::core
namespace RME {
namespace core {
    class Position; // Defined in core/Position.h
    struct BrushSettings; // Defined in core/brush/BrushSettings.h
    namespace map { class Map; } // Defined in core/map/Map.h
    namespace assets { struct ItemType; } // Defined in core/assets/ItemData.h
    namespace editor { class EditorControllerInterface; } // To be defined
} // namespace core
} // namespace RME


namespace RME {
namespace core {
namespace brush {

class RawBrush : public Brush { // Inherits RME::core::Brush
public:
    RawBrush();
    ~RawBrush() override = default;

    void setItemId(uint16_t id);
    uint16_t getItemId() const { return m_itemId; }

    // RME::core::Brush interface overrides
    QString getName() const override;
    // getLookID now takes BrushSettings to potentially get ItemTypeProvider
    int getLookID(const BrushSettings& settings) const override;
    bool canDrag() const override { return true; } // Specific to RawBrush
    bool isRaw() const override { return true; }   // Specific to RawBrush

    void apply(RME::core::editor::EditorControllerInterface* controller, const Position& pos, const BrushSettings& settings) override;
    bool canApply(const RME::core::map::Map* map, const Position& pos, const BrushSettings& settings) const override;

private:
    // Helper to get ItemType, needs access to IItemTypeProvider via BrushSettings or similar
    const RME::core::assets::ItemType* getItemTypeData(const BrushSettings& settings) const;

    uint16_t m_itemId = 0;
};

} // namespace brush
} // namespace core
} // namespace RME
#endif // RME_RAWBRUSH_H
