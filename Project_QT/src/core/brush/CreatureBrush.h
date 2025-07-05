#ifndef RME_CREATURE_BRUSH_H
#define RME_CREATURE_BRUSH_H

#include "core/brush/Brush.h" // Base class
#include "core/Position.h"    // For Position
#include "core/assets/CreatureData.h" // Ensure full definition for constructor/member type

// Forward declarations to avoid full includes in header where possible
namespace RME { namespace core {
    class BrushSettings; // Used in method signatures
    namespace map { class Map; }
    // assets::CreatureData is now included above
    namespace editor { class EditorControllerInterface; }
}}

namespace RME {
namespace core {

// Forward declare again inside RME::core if EditorControllerInterface is in RME::core::editor
namespace editor { class EditorControllerInterface; }

class CreatureBrush : public Brush {
public:
    // Constructor takes CreatureData for this brush
    CreatureBrush(RME::core::editor::EditorControllerInterface* controller = nullptr,
                  const RME::core::assets::CreatureData* creatureData = nullptr);
    ~CreatureBrush() override = default;

    // Renamed from setCreatureType to align with constructor setting, or keep if dynamic changes are needed
    void setCreatureData(const RME::core::assets::CreatureData* creatureData);
    const RME::core::assets::CreatureData* getCreatureData() const;

    // Overridden methods from Brush
    void apply(RME::core::editor::EditorControllerInterface* controller,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& settings) override;

    QString getName() const override;

    int getLookID(const RME::core::BrushSettings& settings) const override;

    bool canApply(const RME::core::map::Map* map,
                  const RME::core::Position& pos,
                  const RME::core::BrushSettings& settings) const override;

    // Legacy compatibility methods for direct map manipulation
    void draw(RME::core::map::Map* map, RME::core::Tile* tile, const RME::core::BrushSettings* settings) override;
    void undraw(RME::core::map::Map* map, RME::core::Tile* tile, const RME::core::BrushSettings* settings = nullptr) override;

private:
    const RME::core::assets::CreatureData* m_creatureData = nullptr; // Changed from m_creatureType
};

} // namespace core
} // namespace RME

#endif // RME_CREATURE_BRUSH_H
