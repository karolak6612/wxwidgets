#ifndef RME_WALL_BRUSH_H
#define RME_WALL_BRUSH_H

#include "core/brush/Brush.h"
#include "core/Position.h"
#include "core/brush/BrushEnums.h" // For RME::BorderType

#include <QString>
#include <cstdint> // For uint16_t, uint32_t

// Forward declarations
namespace RME {
namespace core {
    class BrushSettings;
    namespace map { class Map; }
    namespace assets { class MaterialData; struct MaterialWallSpecifics; }
    namespace editor { class EditorControllerInterface; }
} // namespace core
} // namespace RME

// Forward declaration for the test class (global namespace)
class TestWallBrush;

namespace RME {
namespace core {

class WallBrush : public RME::core::Brush {
    friend class ::TestWallBrush; // Friend class for testing

public:
    WallBrush();
    ~WallBrush() override = default;

    void setMaterial(const RME::core::assets::MaterialData* materialData);
    const RME::core::assets::MaterialData* getMaterial() const;

    // Overridden methods from Brush
    void apply(RME::core::editor::EditorControllerInterface* controller,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& settings) override;

    QString getName() const override;
    int getLookID(const RME::core::BrushSettings& settings) const override;
    bool canApply(const RME::core::map::Map* map,
                  const RME::core::Position& pos,
                  const RME::core::BrushSettings& settings) const override;

    static void initializeStaticData();

private:
    void updateWallAppearance(RME::core::editor::EditorControllerInterface* controller, const RME::core::Position& pos);

    // Helper to get an item ID for a given wall segment type, considering current brush settings (e.g., place door/window)
    uint16_t getItemIdForSegment(RME::BorderType segmentType,
                                 const RME::core::BrushSettings& settings,
                                 const RME::core::assets::MaterialWallSpecifics* specifics) const;

    // Converts a wall segment BorderType (e.g., WALL_HORIZONTAL) to an orientation string (e.g., "horizontal")
    // used in MaterialWallSpecifics.parts.orientationType.
    QString wallSegmentTypeToOrientationString(RME::BorderType segmentType) const;

    const RME::core::assets::MaterialWallSpecifics* getCurrentWallSpecifics() const;

    const RME::core::assets::MaterialData* m_materialData = nullptr;
    // QString m_redirectBrushName; // Store redirect name, resolve to MaterialData* as needed.
                                 // Or handle redirection directly via MaterialManager providing linked material.
                                 // For now, let's assume MaterialData might contain this if needed, or it's a post-load step.

    // Wall types lookup tables (4-neighbor based, so 2^4 = 16 entries)
    // These store RME::BorderType enum values representing wall segments.
    static uint32_t s_full_wall_types[16];
    static uint32_t s_half_wall_types[16]; // For fences or half-height walls if applicable
    static bool s_staticDataInitialized;
};

} // namespace core
} // namespace RME

#endif // RME_WALL_BRUSH_H
