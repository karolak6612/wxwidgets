#ifndef RME_ERASER_BRUSH_H
#define RME_ERASER_BRUSH_H

#include "core/brush/Brush.h"
#include "core/Position.h" // For RME::core::Position
#include <QString>

// Forward declarations
namespace RME {
namespace core {
    class BrushSettings;
    namespace map { class Map; }
    namespace editor { class EditorControllerInterface; }
}
}

// Forward declaration for potential test class
class TestEraserBrush;

namespace RME {
namespace core {
namespace brush {

class EraserBrush : public RME::core::Brush {
    friend class ::TestEraserBrush; // Friend class for testing

public:
    EraserBrush();
    ~EraserBrush() override = default;

    // Overridden methods from Brush
    void apply(RME::core::editor::EditorControllerInterface* controller,
               const RME::core::Position& pos,
               const RME::core::BrushSettings& settings) override;

    QString getName() const override;
    int getLookID(const RME::core::BrushSettings& settings) const override;
    bool canApply(const RME::core::map::Map* map,
                  const RME::core::Position& pos,
                  const RME::core::BrushSettings& settings) const override;

    // EraserBrush does not use materials
    bool hasMaterial() const override { return false; }

private:
    // No specific members needed for EraserBrush if logic is self-contained in apply()
    // and driven by BrushSettings and global AppSettings.
};

} // namespace brush
} // namespace core
} // namespace RME

#endif // RME_ERASER_BRUSH_H
