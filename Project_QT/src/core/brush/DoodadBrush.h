#ifndef RME_DOODAD_BRUSH_H
#define RME_DOODAD_BRUSH_H

#include "core/brush/Brush.h"
#include "core/Position.h" // For RME::core::Position

#include <QString>
#include <cstdint> // For uint16_t

// Forward declarations
namespace RME {
namespace core {
    class BrushSettings;
    namespace map { class Map; }
    namespace assets { class MaterialData; struct MaterialDoodadSpecifics; struct MaterialAlternate; }
    namespace editor { class EditorControllerInterface; }
} // namespace core
} // namespace RME

// Forward declaration for the test class (global namespace)
class TestDoodadBrush;

namespace RME {
namespace core {

class DoodadBrush : public Brush {
    friend class ::TestDoodadBrush; // Friend class for testing

public:
    DoodadBrush();
    ~DoodadBrush() override = default;

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

private:
    const RME::core::assets::MaterialDoodadSpecifics* getCurrentDoodadSpecifics() const;
    // Selects an alternate based on index (e.g., from brush variation setting)
    // Returns nullptr if no alternates or index is out of bounds.
    const RME::core::assets::MaterialAlternate* selectAlternate(
        const RME::core::assets::MaterialDoodadSpecifics* specifics,
        int variationIndex) const;

    const RME::core::assets::MaterialData* m_materialData = nullptr;
};

} // namespace core
} // namespace RME

#endif // RME_DOODAD_BRUSH_H
