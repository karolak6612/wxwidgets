#ifndef RME_CREATURE_BRUSH_H
#define RME_CREATURE_BRUSH_H

#include "core/brush/Brush.h" // Base class
#include "core/Position.h"    // For Position
// Forward declarations to avoid full includes in header where possible
namespace RME { namespace core {
    class BrushSettings; // Used in method signatures
    namespace map { class Map; }
    namespace assets { struct CreatureData; }
    namespace editor { class EditorControllerInterface; }
}}

namespace RME {
namespace core {

class CreatureBrush : public RME::core::Brush {
public:
    CreatureBrush();
    ~CreatureBrush() override = default;

    void setCreatureType(const RME::core::assets::CreatureData* type);
    const RME::core::assets::CreatureData* getCreatureType() const;

    // Overridden methods from Brush
    void apply(RME::core::editor::EditorControllerInterface* controller,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& settings) override;

    QString getName() const override;

    int getLookID(const RME::core::BrushSettings& settings) const override;

    bool canApply(const RME::core::map::Map* map,
                  const RME::core::Position& pos,
                  const RME::core::BrushSettings& settings) const override;

private:
    const RME::core::assets::CreatureData* m_creatureType = nullptr;
};

} // namespace core
} // namespace RME

#endif // RME_CREATURE_BRUSH_H
