#ifndef RME_GROUND_BRUSH_H
#define RME_GROUND_BRUSH_H

#include "core/brush/Brush.h" // Base class
#include "core/Position.h"

// Forward declarations
namespace RME { namespace core {
    class BrushSettings;
    namespace map { class Map; }
    namespace assets { class MaterialData; struct MaterialGroundSpecifics; } // Forward declare MaterialData
    namespace editor { class EditorControllerInterface; }
}}

namespace RME {
namespace core {

class ::TestGroundBrush; // Forward declaration

class GroundBrush : public RME::core::Brush {
    friend class ::TestGroundBrush;
public:
    GroundBrush();
    ~GroundBrush() override = default;

    void setMaterial(const RME::core::assets::MaterialData* materialData);
    const RME::core::assets::MaterialData* getMaterial() const;

    // Overridden methods from Brush
    void apply(RME::core::editor::EditorControllerInterface* controller,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& settings) override;

    QString getName() const override;
    QString getType() const override;

    int getLookID(const RME::core::BrushSettings& settings) const override;

    bool canApply(const RME::core::map::Map* map,
                  const RME::core::Position& pos,
                  const RME::core::BrushSettings& settings) const override;

    // Legacy compatibility methods for direct map manipulation
    void draw(RME::core::map::Map* map, RME::core::Tile* tile, const RME::core::BrushSettings* settings) override;
    void undraw(RME::core::map::Map* map, RME::core::Tile* tile, const RME::core::BrushSettings* settings = nullptr) override;

    static void initializeStaticData();

private:
    void doAutoBorders(RME::core::editor::EditorControllerInterface* controller, const RME::core::Position& targetPos);

    // Helper to get ground specifics from m_material
    const RME::core::assets::MaterialGroundSpecifics* getCurrentGroundSpecifics() const;

    const RME::core::assets::MaterialData* m_materialData = nullptr;

    // The border_types lookup table, similar to wxwidgets.
    // It maps an 8-bit neighbor configuration to up to 4 border types (encoded in a uint32_t).
    static uint32_t s_border_types[256];
    static bool s_staticDataInitialized;
};

} // namespace core
} // namespace RME

#endif // RME_GROUND_BRUSH_H
