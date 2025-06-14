#ifndef RME_SPAWN_BRUSH_H
#define RME_SPAWN_BRUSH_H

#include "core/brush/Brush.h"
#include "core/Position.h" // For RME::core::Position

#include <QString>
#include <cstdint> // For uint16_t

// Forward declarations
namespace RME {
namespace core {
    class BrushSettings;
    namespace map { class Map; }
    // No MaterialData needed for SpawnBrush typically
    namespace editor { class EditorControllerInterface; }
} // namespace core
} // namespace RME

// Forward declaration for the test class (global namespace)
class TestSpawnBrush;

namespace RME {
namespace core {
namespace brush { // Place SpawnBrush in the brush namespace

class SpawnBrush : public RME::core::Brush {
    friend class ::TestSpawnBrush; // Friend class for testing

public:
    SpawnBrush();
    ~SpawnBrush() override = default;

    // SpawnBrush typically doesn't use materials, so no setMaterial/getMaterial

    // Overridden methods from Brush
    void apply(RME::core::editor::EditorControllerInterface* controller,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& settings) override;

    QString getName() const override;
    int getLookID(const RME::core::BrushSettings& settings) const override; // Spawns often have no specific item look
    bool canApply(const RME::core::map::Map* map,
                  const RME::core::Position& pos,
                  const RME::core::BrushSettings& settings) const override;

private:
    // No specific members needed for basic spawn brush if radius comes from settings
    // and it doesn't have variations like material brushes.
};

} // namespace brush
} // namespace core
} // namespace RME

#endif // RME_SPAWN_BRUSH_H
