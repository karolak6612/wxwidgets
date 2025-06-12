#ifndef RME_ERASERBRUSH_H
#define RME_ERASERBRUSH_H

#include "core/brush/Brush.h" // Base RME::core::Brush

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

// Placeholder for a global constant, or define appropriately
const int EDITOR_SPRITE_ERASER_LOOKID = 99001; // Example value

class EraserBrush : public Brush { // Inherits RME::core::Brush
public:
    EraserBrush();
    ~EraserBrush() override = default;

    // RME::core::Brush interface overrides
    QString getName() const override;
    int getLookID(const BrushSettings& settings) const override;

    void apply(editor::EditorControllerInterface* controller, const Position& pos, const BrushSettings& settings) override;
    bool canApply(const map::Map* map, const Position& pos, const BrushSettings& settings) const override;

    // Brush specific flags/properties
    bool isEraser() const override { return true; }
    bool canDrag() const override { return true; }
    bool needsBorders() const override { return true; } // Original EraserBrush needed borders
};

} // namespace brush
} // namespace core
} // namespace RME
#endif // RME_ERASERBRUSH_H
