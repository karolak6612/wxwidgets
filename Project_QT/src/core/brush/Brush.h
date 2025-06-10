#ifndef RME_BRUSH_H
#define RME_BRUSH_H

#include <QString>
// Forward declarations (full types will be included by concrete brushes or when used)
namespace RME {
    struct Position;
    class Map; // For canApply context
    struct BrushSettings; // Parameter for methods
    class EditorControllerInterface; // Parameter for apply
}

namespace RME {

class Brush {
public:
    Brush() = default;
    virtual ~Brush() = default; // Important for base class with virtual functions

    // Disallow copy and assign, brushes are typically unique instances managed by BrushManager
    Brush(const Brush&) = delete;
    Brush& operator=(const Brush&) = delete;
    Brush(Brush&&) = delete;
    Brush& operator=(Brush&&) = delete;

    // Applies the brush effect (draw or erase) at the given position.
    // The controller provides context and methods to interact with the map/editor state.
    // settings provides current brush parameters like size, shape, variation, erase mode.
    virtual void apply(EditorControllerInterface* controller,
                       const Position& pos,
                       const BrushSettings& settings) = 0;

    // Returns the display name of the brush (e.g., "Ground Brush", "Wall Brush").
    virtual QString getName() const = 0;

    // Returns an item ID or sprite ID representing this brush for display in a palette.
    // Can depend on current BrushSettings (e.g., variation).
    // Default implementation returns 0, meaning no specific icon or derived brush must override.
    virtual int getLookID(const BrushSettings& settings) const;

    // Checks if the brush can be applied at the given position with current settings.
    // E.g., a house brush might only be applicable on empty tiles.
    // Default implementation returns true. Derived brushes can override for specific conditions.
    virtual bool canApply(const Map* map,
                          const Position& pos,
                          const BrushSettings& settings) const;

    // Optional: Some brushes might want to draw a preview on the map canvas.
    // virtual void drawPreview(MapCanvas* canvas, const Position& pos, const BrushSettings& settings) const;
};

} // namespace RME

#endif // RME_BRUSH_H
